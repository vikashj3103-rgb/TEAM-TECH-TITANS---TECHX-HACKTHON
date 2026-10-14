# SmartCart — Intelligent Smart Shopping Cart

**Team:** Team Tech Titans  
**Event:** IEEE CS SYP TechX'26 Hackathon

## Overview
SmartCart is a prototype concept for reducing checkout friction using RFID product identification, QR-based cart pairing, live billing interfaces, and a motorized cart controller. The current public repository includes Arduino firmware sketches and project notes. The complete web/backend application is **not currently included in this repository**, so its integration cannot be reproduced from this repository alone.

## Current repository contents
- `firmware/autonomous-movement/autonomous-movement.ino` — Bluetooth-commanded ESP32 motor controller with three ultrasonic sensors and a command timeout.
- `firmware/rfid-billing/rfid-billing.ino` — ESP32 firmware for two MFRC522 readers publishing add/remove events over MQTT.
- `CODE FOR AUTONOMOUS MOVEMENT.txt` — original motor-controller source retained for reference.
- `SMART BILLING SYSTEM.txt` — original RFID source retained for reference.
- `SMART CART CODINGS.txt` — original combined source retained for reference; it contains two sketches and should **not** be compiled as one Arduino sketch.
- `README.md` — project documentation and setup notes.

## Architecture (intended)
1. **RFID reader(s):** identify product tags and publish events.
2. **MQTT broker:** carries cart RFID messages to the application.
3. **Backend and web UI:** intended to maintain cart state, calculate the bill, pair a customer by QR code, and show updates on phone/tablet. These application files are not included here yet.
4. **Motor controller:** receives Bluetooth commands and uses ultrasonic sensors to stop for detected obstacles.

## RFID firmware setup
1. Install Arduino IDE and the ESP32 board package.
2. Install these libraries using Library Manager:
   - **MFRC522** (GithubCommunity)
   - **PubSubClient** (Nick O'Leary)
3. Open `firmware/rfid-billing/rfid-billing.ino` as its own sketch.
4. In the sketch, set `WIFI_SSID`, `WIFI_PASS`, and `MQTT_HOST` for your local test network. Use a **2.4 GHz Wi-Fi network**. The MQTT host should be the LAN IP of the machine running the broker, not the text `lap ip`.
5. Start an MQTT broker on the same reachable network and ensure TCP port 1883 is allowed for your test environment.
6. Select the correct ESP32 board and COM port, upload, then open Serial Monitor at **115200 baud**.
7. Confirm both readers report a valid MFRC522 version and that MQTT connects before testing tags.

The firmware publishes:
- `cart/CART01/rfid_scan` for the ADD reader.
- `cart/CART01/rfid_remove` for the REMOVE reader.

The ADD reader expects product text in the form `SC|name|price|quantity|expiry` on a compatible NTAG sticker. Verify that this payload format matches the backend before integration.

**Network security:** Never commit real Wi-Fi passwords, broker credentials, API keys, or private tokens. The placeholders in this public sketch must be configured locally and must not be replaced with actual secrets in a public commit.

## Motor firmware setup
1. Open `firmware/autonomous-movement/autonomous-movement.ino` as a separate Arduino sketch.
2. Select the ESP32 board and the correct port, then upload.
3. Pair a phone or test controller with Bluetooth device `FollowCar`.
4. Send newline-terminated commands: `F,90`, `B,90`, `L,90`, `R,90`, or `S,0`.
5. Test with the wheels lifted and motor power limited before any floor test.

This is **Bluetooth-commanded movement, not person-following AI**. The code stops on a front-sensor obstacle when reversing because it has no rear sensor. It does not demonstrate a physical emergency-stop switch. Add and validate appropriate independent safety hardware before carrying a person-facing cart in a public area.

## Safety and limitations
- Ultrasonic sensors can miss soft, angled, narrow, or moving objects. Never rely on this prototype as the only safety mechanism.
- Bench-test with wheels raised, use low speed, keep hands clear of motors, and have a physical power cutoff accessible during tests.
- Do not claim autonomous person-following, production-grade checkout, or end-to-end phone/tablet integration until these are implemented and demonstrated.
- RFID events are not buffered when MQTT is disconnected; a scan may be lost.
- Two MFRC522 readers share SPI. Validate chip-select wiring and reader behavior on the actual hardware.
- The complete backend/frontend and QR pairing flow must be added to this repository to make the full software stack reproducible.

## Suggested validation checklist
- [ ] Firmware compiles independently in Arduino IDE.
- [ ] Both RFID readers are detected reliably.
- [ ] ADD and REMOVE events reach the expected MQTT topics.
- [ ] Backend consumes the event format and updates the cart correctly.
- [ ] Duplicate scans, invalid tags, and MQTT disconnects are tested.
- [ ] Motor timeout stops the motors when commands stop.
- [ ] Obstacle response is tested at low speed in a controlled area.
- [ ] A separate physical emergency stop / power cutoff is installed and tested.
- [ ] QR pairing and customer phone/tablet views are demonstrated end to end.

## Team
**Team Tech Titans — IEEE CS SYP TechX'26 Hackathon**
