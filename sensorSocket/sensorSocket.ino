#include <WiFiManager.h>

#include <SocketIoClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
Preferences preferences;
char socket_server[40];
int socket_port;
int serial;
bool shouldSaveConfig = false;
void saveConfigCallback() {
  shouldSaveConfig = true;
}

const byte waterRelay = 12;
bool waterFlag = 0;
const byte waterSensor = 2;
bool doorFlag = 0;
const byte doorSensor = 16;
bool pirFlag = 0;
const byte pirSensor = 5;

SocketIoClient sc;
void setup() {
  Serial.begin(115200);
  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setConfigPortalTimeout(180);

  preferences.begin("socket_config", false);
  strcpy(socket_server, preferences.getString("socket_server", "192.168.1.222").c_str());
  socket_port = preferences.getInt("socket_port", 8000);
  serial = preferences.getInt("serial", 106);
  WiFiManagerParameter custom_socket_server("server", "Socket.io Server", socket_server, 40);
  WiFiManagerParameter custom_socket_port("port", "Socket.io Port", String(socket_port).c_str(), 6);
  WiFiManagerParameter custom_serial("serial", "Serial of device", String(serial).c_str(), 6);
   wifiManager.addParameter(&custom_socket_server);
  wifiManager.addParameter(&custom_socket_port);
  wifiManager.addParameter(&custom_serial);
bool res = wifiManager.autoConnect("2relay1dimmer");

  if (!res) {
    Serial.println("Failed to connect");
  }
  else {
    Serial.println("Connected to Wi-Fi");
  }
  strcpy(socket_server, custom_socket_server.getValue());
  socket_port = atoi(custom_socket_port.getValue());

  if (shouldSaveConfig) {
    preferences.putString("socket_server", socket_server);
    preferences.putInt("socket_port", socket_port);
    serial = atoi(custom_serial.getValue());
    preferences.putInt("serial", serial);
    }
  sc.begin(socket_server,socket_port,"/socket.io/?transport=websocket&type=device");
  pinMode(waterSensor, INPUT);
  pinMode(doorSensor, INPUT);
  pinMode(pirSensor, INPUT_PULLUP);
  pinMode(waterRelay, OUTPUT);
  digitalWrite(waterSensor, LOW);
  sc.on("sensorOff", sensorOf);
}
  bool waterStoper = false;
  byte pirVal = 0;
void loop() {
    if(pirVal == 0 && pirFlag == 0){
    Serial.println("pir on");
    doorEmit("pir", "ON");
    pirFlag = 1;
  }
  if(pirVal == 1 && pirFlag == 1 ){
    Serial.println("pir off");
    doorEmit("pir", "OFF");
    pirFlag = 0;
  }
  if(digitalRead(doorSensor) == 0 && doorFlag == 0){
    Serial.println("door on");
    doorEmit("door", "ON");
    doorFlag = 1;
  }
  if(digitalRead(doorSensor) == 1 && doorFlag == 1 ){
    Serial.println("door off");
    doorEmit("door", "OFF");
    doorFlag = 0;
  }
  if(digitalRead(waterSensor) == 0 && waterFlag == 0){
    Serial.println("water on");
    warningEmit("water");
    waterStoper = true;
    waterFlag = 1;
  }
  if(digitalRead(waterSensor) == 1 && waterFlag == 1 ){
    waterFlag = 0;
  }
  sc.loop();
  if(waterStoper){
    digitalWrite(waterRelay, HIGH);
  }else{
    digitalWrite(waterRelay, LOW);
  }
}
void sensorOf(const char* payload, size_t length) {
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);
  String device = doc["device"];
  if(device == "water"){
    waterStoper = false;
  }
 }

void warningEmit(String device){
  String str = "{\"device\":\""+device+"\"}";
  Serial.println(str);
  sc.emit("warning", str.c_str());
}
void doorEmit(String device, String state){
  String str = "{\"device\":\""+device+"\", \"status\":\""+state+"\"}";
  Serial.println(str);
  sc.emit("warning", str.c_str());
}
