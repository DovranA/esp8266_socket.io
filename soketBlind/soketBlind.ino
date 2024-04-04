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

const byte motorPin1 = 14;
const byte upButtonPin = 16;
const byte motorPin2 = 12;
const byte downButtonPin = 13;
bool upButtonPressed = false;
bool downButtonPressed = false;
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
bool res = wifiManager.autoConnect("blind");

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
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(upButtonPin, INPUT);
  pinMode(downButtonPin, INPUT);
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  upButtonPressed = false;
  downButtonPressed = false;
  sc.on("blind",blind);
}

void loop(){
  sc.loop();
  if(digitalRead(upButtonPin) == 1){
    upButtonPressed = true;
    downButtonPressed = false;
  }else if(digitalRead(downButtonPin) == 1){
    upButtonPressed = false;
    downButtonPressed = true;
  }else{
    upButtonPressed = false;
    downButtonPressed = false;
  }

  if(upButtonPressed){
    Serial.println("upButtonPressed");
    digitalWrite(motorPin1,HIGH);
    digitalWrite(motorPin2, LOW);
    delay(200);
  }else if(downButtonPressed){
    Serial.println("downButtonPressed");
    digitalWrite(motorPin1,LOW);
    digitalWrite(motorPin2, HIGH);
    delay(200);
  }else{
    digitalWrite(motorPin1,LOW);
    digitalWrite(motorPin2, LOW);
  }
}

unsigned long currentMillis =0;
unsigned long previousMillis=0;
void blind(const char* payload, const size_t length){
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);

  String command = doc["command"];
  if(command == "up"){
    Serial.println("blindUp");
  
  digitalWrite(motorPin1,HIGH);
    digitalWrite(motorPin2, LOW);
    delay(100);
  } 
  if(command == "down"){
  Serial.println("blindDown");
  digitalWrite(motorPin1,LOW);
    digitalWrite(motorPin2, HIGH);
    delay(100);
  }
  upButtonPressed = false;
  downButtonPressed = false;
    digitalWrite(motorPin1,LOW);
    digitalWrite(motorPin2, LOW);
}
