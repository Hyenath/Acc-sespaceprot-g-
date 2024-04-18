#include <SPI.h>             // SPI Library for RFID Module
#include <MFRC522.h>         // Library for the RFID Module
#include <Stepper.h>         // Library for the Stepper Motor
#include <WiFi.h>            // Library for WiFi communication
#include <PubSubClient.h>    // Library for MQTT communication


// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";


// MQTT broker details
const char* mqtt_server = "YOUR_MQTT_BROKER_IP";
const int mqtt_port = 1883;
const char* mqtt_username = "YOUR_MQTT_USERNAME";
const char* mqtt_password = "YOUR_MQTT_PASSWORD";
const char* mqtt_topic = "rfid/access";


// Pins used by the RFID Module
#define SS_PIN 10
#define RST_PIN 9


// Define RGB LED pins
int led_red = 8;
int led_green = 7;
int led_blue = 4;


// Create an object for the RFID Module
MFRC522 mfrc522(SS_PIN, RST_PIN);


// Create an object for the Stepper motor
Stepper myStepper(500, 2, 4, 3, 5);


// Initialize WiFi client and MQTT client
WiFiClient espClient;
PubSubClient client(espClient);


// Function to connect to WiFi and MQTT broker
void setup_wifi() {
  delay(10);
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  // Connect to MQTT broker
  client.setServer(mqtt_server, mqtt_port);
  while (!client.connected()) {
    if (client.connect("ArduinoClient", mqtt_username, mqtt_password)) {
      break;
    }
    delay(500);
  }
}


// Function to set RGB LED color
void setColor(int red, int green, int blue) {
  digitalWrite(led_red, red ? HIGH : LOW);
  digitalWrite(led_green, green ? HIGH : LOW);
  digitalWrite(led_blue, blue ? HIGH : LOW);
}


// Callback function for MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
  // Handle authentication response from server
  if (strcmp(topic, mqtt_topic) == 0) {
    if (payload[0] == '1') {
      // Authorized access
      setColor(0, 255, 0); // Turn on green
      // Rotate the stepper motor to unlock
      myStepper.step(200); // Change this to the necessary steps for unlocking
    } else {
      // Access denied
      setColor(255, 0, 0); // Turn on red
    }
  }
}


// Setup function, executed once at the start
void setup() {
  // Set the speed of the Stepper motor
  myStepper.setSpeed(60);


  // Initialize SPI communication
  SPI.begin();


  // Initialize the RFID Module
  mfrc522.PCD_Init();


  // Define RGB LED pins as output
  pinMode(led_red, OUTPUT);
  pinMode(led_green, OUTPUT);
  pinMode(led_blue, OUTPUT);


  // Connect to WiFi and MQTT broker
  setup_wifi();


  // Set MQTT callback function
  client.setCallback(callback);
}


// Main loop function
void loop() {
  // Reconnect to WiFi and MQTT if connection is lost
  if (!client.connected()) {
    setup_wifi();
  }
  client.loop();


  // Set RGB LED to off state
  setColor(0, 0, 0);


  // Check if a new RFID card is present
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }


  // Read the data from the RFID card
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }


  // Decode the TAG code
  String content = "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase(); // Convert the text to uppercase


  // Publish RFID code to MQTT topic for authentication
  client.publish(mqtt_topic, content.c_str());


  // Subscribe to the MQTT topic to receive the authentication response
  client.subscribe(mqtt_topic);


  // Wait for 1 second for authentication response from server
  delay(1000);
}
