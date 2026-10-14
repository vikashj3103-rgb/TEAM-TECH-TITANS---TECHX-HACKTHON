"""
SmartCart AI person tracking (webcam + YOLO + Bluetooth serial).

Safety notes:
- Test with drive wheels lifted and motor power limited first.
- Use an independent physical emergency-stop/power cutoff; software stop
  commands are not a substitute for hardware safety.
- This follows the largest detected person in the frame. It does not identify
  a particular shopper and may switch targets if multiple people are visible.
- Person bounding-box height is only a rough distance proxy and varies with
  camera angle, pose, and framing.
"""

import time

import cv2
import serial
from ultralytics import YOLO

BLUETOOTH_PORT = "COM20"
BAUD_RATE = 115200
CAMERA_INDEX = 0

TARGET_HEIGHT_RATIO = 0.90
DEADBAND = 0.05
FORWARD_SPEED = 120
BACKWARD_SPEED = 70  # Reserved; reverse is disabled below for safety.


def main() -> None:
    bt = None
    camera = None
    last_command = ""
    last_send = 0.0

    def send_command(command: str, speed: int) -> None:
        nonlocal last_command, last_send
        if bt is None or not bt.is_open:
            return

        now = time.time()
        if command != last_command or now - last_send > 0.2:
            message = f"{command},{speed}\n"
            bt.write(message.encode("ascii"))
            print("Bluetooth sent:", message.strip())
            last_command = command
            last_send = now

    try:
        bt = serial.Serial(BLUETOOTH_PORT, BAUD_RATE, timeout=0.1)
        time.sleep(2)

        model = YOLO("yolo11n.pt")
        camera = cv2.VideoCapture(CAMERA_INDEX)

        if not camera.isOpened():
            send_command("S", 0)
            raise RuntimeError("Webcam not found; cart stop command sent.")

        camera.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
        camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

        while True:
            ok, frame = camera.read()
            if not ok:
                send_command("S", 0)
                print("Camera frame unavailable; stopping.")
                break

            height, _width = frame.shape[:2]
            results = model.predict(
                frame,
                classes=[0],
                conf=0.25,
                imgsz=640,
                verbose=False,
            )

            people = []
            boxes = results[0].boxes
            if boxes is not None:
                for box in boxes:
                    x1, y1, x2, y2 = box.xyxy[0].cpu().tolist()
                    confidence = float(box.conf[0])
                    box_height = y2 - y1
                    box_width = x2 - x1
                    if box_height > 0 and box_width > 0:
                        people.append(
                            (x1, y1, x2, y2, confidence, box_height)
                        )

            command = "S"
            speed = 0
            status = "NO PERSON - STOP"
            color = (0, 0, 255)

            if people:
                # Original behavior: follow the largest detected person.
                x1, y1, x2, y2, confidence, box_height = max(
                    people, key=lambda person: person[5]
                )
                ratio = box_height / height

                if ratio < TARGET_HEIGHT_RATIO - DEADBAND:
                    command = "F"
                    speed = FORWARD_SPEED
                    status = "PERSON FAR - FORWARD"
                    color = (0, 255, 0)
                elif ratio > TARGET_HEIGHT_RATIO + DEADBAND:
                    # Reverse is intentionally disabled because no rear
                    # camera/sensor safety verification is provided here.
                    command = "S"
                    speed = 0
                    status = "PERSON CLOSE - STOP (reverse disabled)"
                    color = (0, 0, 255)
                else:
                    status = "TARGET DISTANCE - STOP"
                    color = (0, 255, 255)

                cv2.rectangle(
                    frame, (int(x1), int(y1)), (int(x2), int(y2)), (0, 255, 0), 2
                )
                cv2.putText(
                    frame,
                    f"Confidence: {confidence:.2f}",
                    (10, 90),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.6,
                    (255, 255, 0),
                    2,
                )
                cv2.putText(
                    frame,
                    f"Person height ratio: {ratio:.2f}",
                    (10, 60),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    0.6,
                    (255, 255, 0),
                    2,
                )

            send_command(command, speed)
            cv2.putText(
                frame,
                status,
                (10, 30),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.65,
                color,
                2,
            )
            cv2.imshow("SmartCart AI Person Tracking", frame)

            if cv2.waitKey(1) & 0xFF == ord("q"):
                break

    except KeyboardInterrupt:
        print("Interrupted; stopping cart.")
    finally:
        # Best-effort stop for normal exit, camera failure, and exceptions.
        if bt is not None and bt.is_open:
            try:
                bt.write(b"S,0\n")
                time.sleep(0.1)
            except (serial.SerialException, OSError):
                pass
        if camera is not None:
            camera.release()
        cv2.destroyAllWindows()
        if bt is not None and bt.is_open:
            bt.close()


if __name__ == "__main__":
    main()
