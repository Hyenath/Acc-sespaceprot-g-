#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>


// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";


// MQTT broker details
const char* mqtt_server = "YOUR_MQTT_BROKER_IP";
const int mqtt_port = 1883;
const char* mqtt_username = "YOUR_MQTT_USERNAME";
const char* mqtt_password = "YOUR_MQTT_PASSWORD";
const char* mqtt_topic = "rfid/access";


// RFID reader
#define RST_PIN 22 // Adjust according to your pinout
#define SS_PIN 21  // Adjust according to your pinout
MFRC522 mfrc522(SS_PIN, RST_PIN);


WiFiClient espClient;
PubSubClient client(espClient);


// Database functions (replace with your actual database functions)
bool storeRFIDInDatabase(String rfid) {
  // Implement your database storage logic here
  // Return true if the RFID is stored successfully, false otherwise
  return true;
}


bool isRFIDAuthorized(String rfid) {
  // Implement your database lookup logic here
  // Return true if the RFID is authorized, false otherwise
  return true;
}


void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();


  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connection au Wifi...");
  }
  Serial.println("Connecte au Wifi");


  // Connect to MQTT broker
  client.setServer(mqtt_server, mqtt_port);
  while (!client.connected()) {
    if (client.connect("ESPClient", mqtt_username, mqtt_password)) {
      break;
    }
    delay(500);
    Serial.println("Connection au broker MQTT...");
  }
  Serial.println("Connecte au broker MQTT");


  // Set MQTT callback function
  client.setCallback(callback);
}


void loop() {
  client.loop();


  // Look for new RFID cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }


  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }


  // Prepare the RFID tag UID
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) {
      content.concat("0");
    }
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();


  // Store RFID in the database
  bool stored = storeRFIDInDatabase(content);
  if (stored) {
    Serial.println("RFID enregistre dans la base de donnees: " + content);
  } else {
    Serial.println("Echec de l'enregistrement dans la base de donnees: " + content);
  }


  // Check if the RFID is authorized
  bool authorized = isRFIDAuthorized(content);
  if (authorized) {
    // Publish authorization status to MQTT topic
    client.publish("rfid/access/response", "1");
  } else {
    // Publish authorization status to MQTT topic
    client.publish("rfid/access/response", "0");
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  // Not used in this example
}
