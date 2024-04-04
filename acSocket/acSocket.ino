#include <IRremoteESP8266.h>
#include <ir_Samsung.h>
#include <IRremote.h>

#include <WiFiManager.h>

#include <SocketIoClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
Preferences preferences;
char socket_server[40];
int socket_port;
int serial;


#define DELAY_BETWEEN_COMMANDS 1000
const uint16_t kIrLed = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRSamsungAc ac(kIrLed);     // Set the GPIO used for sending messages.

SocketIoClient sc;

const int led = BUILTIN_LED;

void printState() {
  // Display the settings.
  Serial.println("Samsung A/C remote is in the following state:");
  Serial.printf("  %s\n", ac.toString().c_str());
}



void handleCommand(const char* payload, size_t length){
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);
  String command = doc["command"];
  int tempr = doc["tempr"];
  command.trim();
  if (command.length() > 0){
    if (command == "on"){
      ac.on();
      ac.send();
       printState();  
    }
    if (command == "off"){
     ac.off();
     ac.send();
     printState();  
    }
    if (command == "cool"){
     ac.setMode(kSamsungAcCool);
     ac.send();
     printState();  
    }
   if (command == "low"){
     ac.setFan(kSamsungAcFanLow);
     ac.send();
     printState();  
    }
    if (command == "high"){
     ac.setFan(kSamsungAcFanHigh);
     ac.send();
     printState();  
    }
    if (command == "swingon"){
     ac.setSwing(true);
     ac.send();
      printState();  
    }
     if (command == "swingoff"){
     ac.setSwing(false);
     ac.send();
      server.send(200, "text/plain", "swingoff");
    }
  }
  if(tempr > 15 && 33 < tempr){
     ac.setTemp(tempr);
     ac.send();
  }
}


void setup(){
 
  /*Serial.begin(115200);
  delay(200);

  // Set up what we want to send. See ir_Samsung.cpp for all the options.
  Serial.println("Default state of the remote.");
  printState();
  Serial.println("Setting initial state for A/C.");
  ac.off();
  ac.setFan(kSamsungAcFanLow);
  ac.setMode(kSamsungAcCool);
  ac.setTemp(25);
  ac.setSwing(false);
  printState();
*/
 Serial.begin(115200);
  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConfigPortalTimeout(180);

  preferences.begin("socket_config", false);
  strcpy(socket_server, preferences.getString("socket_server", "192.168.1.222").c_str());
  socket_port = preferences.getInt("socket_port", 8000);
  serial = preferences.getInt("serial", 104);
  WiFiManagerParameter custom_socket_server("server", "Socket.io Server", socket_server, 40);
  WiFiManagerParameter custom_socket_port("port", "Socket.io Port", String(socket_port).c_str(), 6);
  WiFiManagerParameter custom_serial("serial", "Serial of device", String(serial).c_str(), 6);
   wifiManager.addParameter(&custom_socket_server);
  wifiManager.addParameter(&custom_socket_port);
  wifiManager.addParameter(&custom_serial);
bool res = wifiManager.autoConnect("acRemote");

  if (!res) {
    Serial.println("Failed to connect");
  }
  else {
    Serial.println("Connected to Wi-Fi");
  }
  strcpy(socket_server, custom_socket_server.getValue());
  socket_port = atoi(custom_socket_port.getValue());
  serial = atoi(custom_serial.getValue());
  if (shouldSaveConfig) {
    preferences.putString("socket_server", socket_server);
    preferences.putInt("socket_port", socket_port);
    preferences.putInt("serial", serial);
    }

  Serial.println("Server:");
  Serial.println(socket_server);
  Serial.println("Port:");
  Serial.println(socket_port);
  Serial.println("Serial:");
  Serial.println(serial);
  sc.begin(socket_server,socket_port,"/socket.io/?transport=websocket&type=device");

  ac.begin();
  pinMode(led, OUTPUT);
  digitalWrite(led, 1);
    
  sc.on("ac",handleCommand);
}

void loop() {
  sc.loop();
}
