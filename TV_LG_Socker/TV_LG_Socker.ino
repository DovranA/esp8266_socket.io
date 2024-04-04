#include <IRremote.h>

#include <WiFiManager.h>

#include <SocketIoClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
Preferences preferences;
char socket_server[40];
int socket_port;
int serial;
#define IR_SEND_PIN 4
#define DELAY_BETWEEN_COMMANDS 1000
IRsend irsend(IR_SEND_PIN);

const int led = BUILTIN_LED;

SocketIoClient sc;

void handleTv(const char* payload, size_t length){
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);
  String command = doc["command"];
  if (command == "down"){
      Serial.println("Sorround Sound Down");
      irsend.sendNEC(0xE0E08679   , 32);
    }

    if (command == "up"){
      Serial.println("Surround Sound Up");
      irsend.sendNEC(0xE0E006F9, 32);
    }
    if (command == "satok"){
      Serial.println("Sat Ok");
      irsend.sendNEC(0xE0E016E9, 32);
    }
    if (command == "tvpower"){
      Serial.println("TV power");
      irsend.sendNEC(0xE0E040BF, 32);
    }
 if (command == "mute"){
      Serial.println("mute");
      irsend.sendNEC(0xE0E0F00F, 32);
    }
    if (command == "volumedown"){
      Serial.println("volumedown");
      irsend.sendNEC(0xE0E0D02F, 32);
    }
 
   if (command == "upvolume"){
      Serial.println("volumeup");
      irsend.sendNEC(0xE0E0E01F, 32);
    }

    if (command == "menu"){
      Serial.println("menu");
      irsend.sendNEC(0xE0E058A7, 32);
    }

    if (command == "exit"){
      Serial.println("exit");
      irsend.sendNEC(0xE0E0B44B, 32);
    }

    if (command == "right"){
      Serial.println("Surround Sound Channel 3");
      irsend.sendNEC(0xE0E046B9, 32);
    }

    if (command == "left"){
      Serial.println("Surround Sound Channel 4");
      irsend.sendNEC(0xE0E0A659, 32);
  }
}

void setup(){

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
bool res = wifiManager.autoConnect("tvRemote");

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
 irsend.begin();
 pinMode(led, OUTPUT);
 digitalWrite(led, 1);
  
  sc.on("tv",handleTv);
}

void loop() {
  sc.loop();
}
