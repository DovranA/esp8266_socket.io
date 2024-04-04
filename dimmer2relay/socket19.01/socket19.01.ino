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
const byte dimmerPin = 14; // GPIO pin connected to the dimmer module
int dimmerValue = 0;      // Remove "const" to allow modification
const byte relay1 = 4;
const byte relay1Inp = 12;
bool relay1Flag = 0;
const byte relay2 = 2;
const byte relay2Inp = 13;
bool relay2Flag = 0;
int deviceIds[]={-2,-1,0};
int pinArr[]={relay1, relay2, dimmerPin};
const String deviceType = "2relay1dimmer";
SocketIoClient sc;
void setup() {
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
bool res = wifiManager.autoConnect("dimmer2relay");

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
  
  pinMode(dimmerPin, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay1Inp, INPUT);
  pinMode(relay2Inp, INPUT);
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  analogWrite(dimmerPin, 0);


  initializeEmit(deviceType, serial);
  sc.on("getId",handleId);
  sc.on("deviceReceiver",handleDevice);
  
}

void loop(){

  boolean switcher1 = digitalRead(relay1Inp);
  if(switcher1 == 1 && relay1Flag == 0 ){
      relay1Flag = 1;
      digitalWrite(relay1, HIGH); 
      deviceEmit(deviceIds[0], 1);   
  }
  if(switcher1 == 0 && relay1Flag == 1){
      relay1Flag = 0;
      digitalWrite(relay1, LOW); 
      deviceEmit(deviceIds[0], 0); 
  }
  boolean switcher2 = digitalRead(relay2Inp);
  if(switcher2 == 1 && relay2Flag == 0 ){
      relay2Flag = 1;
      digitalWrite(relay2, HIGH); 
      deviceEmit(deviceIds[1], 1);    
  }
  if(switcher2 == 0 && relay2Flag == 1){
      relay2Flag = 0;
      digitalWrite(relay2, LOW); 
      deviceEmit(deviceIds[1], 0); 
  }
  sc.loop();
}

void handleId(const char* payload, const size_t length) {
  Serial.println(payload);
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);
  Serial.println();
  int deviceSerial = doc["serial"];
  Serial.println(serial);
  if(serial == deviceSerial){
  JsonArray devices = doc["devices"].as<JsonArray>();
  byte relayNum = 0;
    for(JsonObject device : devices){
    int id = device["id"];
    String deviceType = device["device_type"];
    if(deviceType == "relay"){
      deviceIds[relayNum] = id;
      relayNum++;
    }else{
      if(deviceType == "dimmer"){
        deviceIds[2] = id;
      }
      else{
        deviceIds[1] = 0;
      }
    }
  }
 }
}
void handleDevice(const char* payload, const size_t length) {
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);

  int id = doc["id"];
  int State = doc["state"];
for (int i = 0; i < 3; i++) {
  if(deviceIds[i] == id){
    Serial.println(String(i));
  if(i == 2){
    analogWrite(pinArr[i], State);
  }else{
    digitalWrite(pinArr[i], State);
  }}
}
}
void initializeEmit(String dType, int seria){
  String str = "{\"type\":\""+dType+"\", \"serial\":"+String(seria)+"}";
  sc.emit("initialize", str.c_str());
}
const char * commandEmit(int id, int State){
  String str = "{\"command\":"+String(id)+", \"state\":"+String(State)+"}";
  return str.c_str();
}
void deviceEmit(int id, int State){
  String str = "{\"id\":"+String(id)+", \"state\":"+String(State)+"}";
  sc.emit("deviceSend", str.c_str());
}
