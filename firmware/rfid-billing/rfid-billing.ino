/*
  Smart Cart - RFID-only firmware (no load cell)
  Two MFRC522 readers -> WiFi -> MQTT -> server -> phone + tablet web pages

  Publishes:
    cart/<CART_ID>/rfid_scan    {"tagId":"04A1E649BC2A81","data":"SC|Amul Milk|60|1050|2026-10-20"}   (ADD reader)
    cart/<CART_ID>/rfid_remove  {"tagId":"04A1E649BC2A81"}                                           (REMOVE reader)
  "data" is the product text written on the sticker with tag_writer.ino (NTAG stickers only).

  Libraries (Arduino IDE > Library Manager):
    - MFRC522 (by GithubCommunity)
    - PubSubClient (by Nick O'Leary)
  Board: "ESP32 Dev Module"

  Built-in LED: ON = connected to the server, short blink = sticker read.
*/
#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>

// ================= EDIT THESE =================
const char*    WIFI_SSID = "YOUR_2_4_GHZ_WIFI_SSID"; // configure locally; do not commit real credentials
const char*    WIFI_PASS = "YOUR_WIFI_PASSWORD";
const char*    MQTT_HOST = "192.168.1.100"; // replace with your laptop's LAN IP (ipconfig)
const uint16_t MQTT_PORT = 1883;
const char*    CART_ID   = "CART01";
// ==============================================

// Pins
#define RFID1_SS_PIN   5    // ADD reader
#define RFID1_RST_PIN  22
#define RFID2_SS_PIN   21   // REMOVE reader
#define RFID2_RST_PIN  27
#define LED_PIN        2
// Shared by both readers: SCK = GPIO 18, MOSI = GPIO 23, MISO = GPIO 19

const unsigned long UID_COOLDOWN_MS = 3000;  // ignore the same sticker read again on the same reader within this time

MFRC522 rfid1(RFID1_SS_PIN, RFID1_RST_PIN);  // ADD reader
MFRC522 rfid2(RFID2_SS_PIN, RFID2_RST_PIN);  // REMOVE reader
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

char topicAdd[48], topicRemove[48];

String lastUid1 = "", lastUid2 = "";
unsigned long lastTime1 = 0, lastTime2 = 0;

void connectWiFi() {
  Serial.print("WiFi connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi OK, IP: ");
  Serial.println(WiFi.localIP());
}

void ensureConnections() {
  static unsigned long lastWifiTry = 0, lastMqttTry = 0;

  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_PIN, LOW);
    if (millis() - lastWifiTry > 5000) {
      lastWifiTry = millis();
      Serial.println("WiFi lost, reconnecting...");
      WiFi.reconnect();
    }
    return;
  }

  if (!mqtt.connected()) {
    digitalWrite(LED_PIN, LOW);
    if (millis() - lastMqttTry > 3000) {
      lastMqttTry = millis();
      Serial.print("MQTT connecting... ");
      String clientId = String("rfid-") + CART_ID;
      if (mqtt.connect(clientId.c_str())) {
        Serial.println("OK");
      } else {
        Serial.print("failed, rc=");
        Serial.println(mqtt.state());
      }
    }
    return;
  }

  digitalWrite(LED_PIN, HIGH);
}

void blinkLed() {
  digitalWrite(LED_PIN, LOW);
  delay(80);
  digitalWrite(LED_PIN, HIGH);
}

// Read the product text stored on an NTAG sticker (written with tag_writer.ino)
String readTagData(MFRC522 &r) {
  String out = "";
  byte buffer[18];
  for (int p = 0; p < 12; p += 4) {              // pages 4..15 (48 bytes)
    byte size = sizeof(buffer);
    if (r.MIFARE_Read(4 + p, buffer, &size) != MFRC522::STATUS_OK) return "";
    for (int i = 0; i < 16; i++) {
      if (buffer[i] == 0) return out.startsWith("SC|") ? out : "";
      char c = (char)buffer[i];
      if (c == '"' || c == '\\') c = '\'';       // keep the JSON message valid
      out += c;
    }
  }
  return out.startsWith("SC|") ? out : "";
}

void readReader(MFRC522 &reader, const char* topic, bool sendData, String &lastUid, unsigned long &lastTime, const char* label) {
  if (!reader.PICC_IsNewCardPresent() || !reader.PICC_ReadCardSerial()) return;

  String uid = "";
  for (byte i = 0; i < reader.uid.size; i++) {
    if (reader.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(reader.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();

  String tagData = "";
  if (sendData && reader.PICC_GetType(reader.uid.sak) == MFRC522::PICC_TYPE_MIFARE_UL) {
    tagData = readTagData(reader);
  }

  reader.PICC_HaltA();
  reader.PCD_StopCrypto1();

  if (uid == lastUid && millis() - lastTime < UID_COOLDOWN_MS) return;
  lastUid = uid;
  lastTime = millis();

  Serial.println(String(label) + " reader: " + uid);

  char msg[160];
  if (tagData.length() > 0) {
    Serial.println("   data: " + tagData);
    snprintf(msg, sizeof(msg), "{\"tagId\":\"%s\",\"data\":\"%s\"}", uid.c_str(), tagData.c_str());
  } else {
    if (sendData) Serial.println("   (no product data on this sticker)");
    snprintf(msg, sizeof(msg), "{\"tagId\":\"%s\"}", uid.c_str());
  }

  if (mqtt.connected()) {
    mqtt.publish(topic, msg);
    blinkLed();
  } else {
    Serial.println("   MQTT down - scan was NOT sent");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  delay(500);
  Serial.println("\n=== Smart Cart RFID (no load cell) ===");

  snprintf(topicAdd, sizeof(topicAdd), "cart/%s/rfid_scan", CART_ID);
  snprintf(topicRemove, sizeof(topicRemove), "cart/%s/rfid_remove", CART_ID);

  SPI.begin();
  rfid1.PCD_Init();
  rfid2.PCD_Init();
  Serial.print("ADD reader    - ");
  rfid1.PCD_DumpVersionToSerial();    // expect Version: 0x91 or 0x92
  Serial.print("REMOVE reader - ");
  rfid2.PCD_DumpVersionToSerial();    // 0x00 / 0xFF = wiring problem

  WiFi.mode(WIFI_STA);
  connectWiFi();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
}

void loop() {
  ensureConnections();
  mqtt.loop();
  readReader(rfid1, topicAdd,    true,  lastUid1, lastTime1, "ADD");
  readReader(rfid2, topicRemove, false, lastUid2, lastTime2, "REMOVE");
}
