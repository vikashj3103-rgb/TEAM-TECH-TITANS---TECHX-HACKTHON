# SmartCart — Intelligent Smart Shopping Cart

**Team:** Team Tech Titans  
**Event:** IEEE CS SYP TechX'26 Hackathon

## 1. Overview

SmartCart is a smart shopping cart project designed to improve the retail shopping experience by reducing manual checkout effort and enabling digital billing. The system combines RFID-based product identification, QR-based customer-cart pairing, live bill updates, and a planned autonomous cart-following system.

## 2. Problem Statement

Traditional shopping often requires customers to wait in checkout queues while products are scanned and bills are generated. SmartCart aims to streamline this process through automated product identification and real-time billing.

## 3. Proposed Solution

SmartCart connects a shopping cart with a customer-facing interface. The customer can pair with the cart using a QR code, view scanned products, and monitor the bill. The proposed hardware system uses RFID and sensors to support product tracking and cart operation.

## 4. Key Features

- QR-based customer and cart pairing.
- RFID-based product identification.
- Live bill and cart-status updates.
- Web-based customer interface and monitoring dashboard.
- MQTT-based communication for connected components.
- Proposed motorized cart-following functionality.
- Safety features planned for autonomous movement, including obstacle detection and emergency stop.

**Note:** Features must be demonstrated and verified before being described as fully implemented.

## 5. System Architecture

The proposed system consists of:

1. **Customer interface:** Displays cart information, scanned products, and the bill.
2. **Backend server:** Coordinates application requests and cart-related data.
3. **Communication layer:** Uses MQTT and Socket.IO where configured.
4. **RFID and sensor hardware:** Supports product identification and sensor readings.
5. **Motor controller:** Intended to manage cart movement.
6. **Monitoring dashboard:** Provides a view of cart and system status.

## 6. Technology Stack

- **Frontend:** HTML, CSS, JavaScript.
- **Backend:** Node.js and Express.
- **Real-time communication:** MQTT and Socket.IO.
- **Data storage:** JSON-file storage in the current prototype.
- **Hardware platform:** ESP32-based controllers, RFID reader, and sensors, as applicable to the implemented build.

## 7. Project Structure

The repository contains the source code and supporting files for the implemented SmartCart components. Refer to the folders and configuration files in this repository for the exact project structure.

## 8. Setup and Execution

1. Install Node.js and npm.
2. Open the backend project directory in a terminal.
3. Install the dependencies listed in `package.json` using `npm install`.
4. Configure the required environment variables and communication settings using a local configuration file.
5. Start the backend using the command specified in the project's `package.json` or setup instructions.
6. Start any required MQTT broker and hardware components.
7. Open the relevant frontend page and verify connectivity.

The exact commands and configuration depend on the files included in this repository. Never commit passwords, API keys, or private credentials.

## 9. Testing and Validation

Testing should cover:

- Backend startup and API responses.
- Customer-cart QR pairing.
- RFID product detection and duplicate-scan handling.
- Correct bill calculation and item removal.
- Real-time communication and recovery after disconnection.
- Hardware operation and emergency-stop behavior, where implemented.
- Customer interface responsiveness on a phone.

Only report tests as passed when they have actually been performed.

## 10. Current Limitations

- Hardware integration and end-to-end testing may require further validation.
- QR pairing and live billing should be verified using the actual customer device.
- Autonomous following and safety mechanisms must be demonstrated on the physical cart before being claimed as operational.
- JSON-file storage may require a more robust database solution for larger deployments.

## 11. Future Enhancements

- Reliable offline operation and connection recovery.
- Improved inventory and billing validation.
- More robust data storage and multi-cart support.
- Enhanced autonomous navigation and obstacle avoidance.
- Comprehensive hardware and security testing.

## 12. Team

**Team Tech Titans — IEEE CS SYP TechX'26 Hackathon**

This repository contains the SmartCart project source code and documentation for technical review.
