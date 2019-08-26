#include <SPI.h>
#include <MFRC522.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#define SS_PIN D4
#define RST_PIN D2

const char* ssid = "Familia Sanchez";
const char* password = "sanchez1";
const char* mqttServer = "3.83.223.148";
const int mqttPort = 16331;
const char* mqttUser = "user1";
const char* mqttPassword = "0000";

MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long getID(){

  if (!mfrc522.PICC_IsNewCardPresent()) {
    return -1;    
  }
  
  if (! mfrc522.PICC_ReadCardSerial()) { //Since a PICC placed get Serial and continue
    return -1;
  }
  
  unsigned long hex_num;
  hex_num =  mfrc522.uid.uidByte[0] << 24;
  hex_num += mfrc522.uid.uidByte[1] << 16;
  hex_num += mfrc522.uid.uidByte[2] <<  8;
  hex_num += mfrc522.uid.uidByte[3];
  mfrc522.PICC_HaltA(); // Stop reading
  return hex_num;
}
void callback(char* topic, byte* payload, unsigned int length) { 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
 
  Serial.println();
  Serial.println("-----------------------"); 
}

void reconnect() {
  uint8_t retries = 3;
  
  while (!client.connected()) {
    Serial.print("Intentando conexion MQTT...");   
    
    if (client.connect("ESP8266Client", mqttUser, mqttPassword)) {
      Serial.println("conectado");
      client.subscribe("/user1/rfid");
    } else {
      Serial.print("fallo, rc=");
      Serial.print(client.state());
      Serial.println(" intenta nuevamente en 5 segundos");
      // espera 5 segundos antes de reintentar
      delay(5000);
    }
    retries--;
    if (retries == 0) {
      // esperar a que el WDT lo reinicie
      while (1);
    }
  }
}

void setup() {
   Serial.begin(9600);
   SPI.begin();       // Init SPI bus
   mfrc522.PCD_Init(); // Init MFRC522

   WiFi.begin(ssid, password);
 
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
   }   
    
   client.setServer(mqttServer, mqttPort);
   client.setCallback(callback);

   while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) { 
      Serial.println("connected");  
 
    } else { 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000); 
    }
  }
 
  client.publish("/user1/rfid", "Hello from ESP8266");
  client.subscribe("/user1/rfid");
}

void loop() {
  unsigned long id = getID();
  char charId[12];

  if (!client.connected()) {
    Serial.println("Disconnected");
    reconnect();
  }
  
  client.loop();

  if (id != -1) {    
    itoa(id, charId, 16);
    client.publish("/user1/rfid", charId);
    Serial.println("Acceso Concedido");   
  }   
}
