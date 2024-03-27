#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define SS_PIN 5  // Pin de sélection du lecteur RFID
#define RST_PIN 4 // Pin de réinitialisation du lecteur RFID
#define INTERRUPT_PIN 2 // Pin de l'interrupteur poussoir
#define LED_PIN 13 // Pin de la LED pour les indications

#define WIFI_SSID "Nom_du_Réseau_WiFi"
#define WIFI_PASSWORD "Mot_de_passe_WiFi"
#define MQTT_BROKER "Adresse_IP_du_Broker_MQTT"
#define MQTT_PORT 1883
#define MQTT_USER "Nom_d_utilisateur_MQTT"
#define MQTT_PASSWORD "Mot_de_passe_MQTT"
#define MQTT_TOPIC "topic/serrure"

MFRC522 mfrc522(SS_PIN, RST_PIN); // Initialisation du lecteur RFID

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  pinMode(INTERRUPT_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  connectWiFi();
  client.setServer(MQTT_BROKER, MQTT_PORT);
}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
  
  if (digitalRead(INTERRUPT_PIN) == LOW) {
    // Si le bouton est enfoncé, déverrouiller la porte
    unlockDoor();
  }

  // Vérifier si une carte RFID est présentée
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    // Lire l'identifiant de la carte RFID
    String rfidTag = getRFIDTag();
    // Envoyer l'identifiant au serveur MQTT
    publishRFIDTag(rfidTag);
  }
}

String getRFIDTag() {
  String rfidTag = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    rfidTag.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
    rfidTag.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  rfidTag.toUpperCase();
  return rfidTag;
}

void publishRFIDTag(String rfidTag) {
  String message = "{\"rfid\": \"" + rfidTag + "\"}";
  client.publish(MQTT_TOPIC, message.c_str());
  digitalWrite(LED_PIN, HIGH);
  delay(1000); // Indicateur LED
  digitalWrite(LED_PIN, LOW);
}

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void reconnectMQTT() {
  while (!client.connected()) {
    if (client.connect("ArduinoClient", MQTT_USER, MQTT_PASSWORD)) {
      Serial.println("Connected to MQTT Broker");
    } else {
      Serial.print("MQTT connection failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void unlockDoor() {
  client.publish(MQTT_TOPIC, "OPEN");
  delay(5000); // Déverrouiller la porte pendant 5 secondes
}
