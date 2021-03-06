#include <SPI.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>


#define PWM1   5   // PWM1 output
#define DIR1   0   // Dir 1
#define LED_OUT    16

#define PMW_VAL 1024

#define DEBUG

const char* ssid = "xxxxxx"; //your wifi ssid
const char* password = "yyyyyyyy";  //your wifi password 
const char* ssid_ap = "SafetyLock";
const char* password_ap = "esp8266Testing";

int ledState = LOW;

unsigned long previousMillis = 0;
unsigned long previousMillis2 = 0;
const long interval = 1000;
const long interval2 = 1;

const int PACKAGE_SIZE = 64;
byte packetBuffer[PACKAGE_SIZE];

//Creating UDP Listener Object. 
WiFiUDP UDPServer;
unsigned int UDPPort = 6868;
unsigned int clientPort = 6969;

int seed = 0;
int key = 0;

String state = "";

void setup() {
  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();

  pinMode(LED_OUT, OUTPUT);
  digitalWrite(LED_OUT, 1);

  //motor setting
  pinMode(PWM1, OUTPUT);
  pinMode(DIR1, OUTPUT);
  analogWrite(PWM1, LOW);
  digitalWrite(DIR1, LOW);

  setLock(0);

#if 1
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
#else
  WiFi.softAP(ssid_ap, password_ap);
  Serial.println("");

  Serial.println();
  Serial.print("Server IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("Server MAC address: ");
  Serial.println(WiFi.softAPmacAddress());
#endif

  UDPServer.begin(UDPPort); 

  digitalWrite(LED_OUT, 0);
  setLock(PMW_VAL);

  randomSeed(millis());

  memset(packetBuffer, 0, PACKAGE_SIZE);

  Serial.println("Server Start\n");
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (ledState == LOW)
      ledState = HIGH;  // Note that this switches the LED *off*
    else
      ledState = LOW;   // Note that this switches the LED *on*
    digitalWrite(LED_OUT, ledState);
  }
  processPostData();
}

/* Process post request from HTTP client */
void processPostData(){
  int cb = UDPServer.parsePacket();
  if (cb) {
    UDPServer.read(packetBuffer, PACKAGE_SIZE);
    // UDPServer.flush();
    IPAddress addr = UDPServer.remoteIP();
    String message = String((char*)packetBuffer);
#ifdef DEBUG
    Serial.print("receive: ");
    Serial.println(message);
    Serial.print("from: ");
    Serial.println(addr);
#endif
    String strResponse; 
    if (message == "requestLock"){
      strResponse = "Locked";
      UDPServer.beginPacket(addr, clientPort);
      UDPServer.write(strResponse.c_str());
      UDPServer.endPacket();
      setLock(0);
    } else if( message == "requestUnlock"){
      strResponse = "Unlocked";
      UDPServer.beginPacket(addr, clientPort);
      UDPServer.write(strResponse.c_str());
      UDPServer.endPacket();
      setLock(PMW_VAL);
    }
    memset(packetBuffer, 0, PACKAGE_SIZE);
  }
}

/* Set motor speed */
void setLock(int val){
   analogWrite(PWM1, val);
   digitalWrite(DIR1, LOW);
}
