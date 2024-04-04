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

int cooker1Pin = 16;
int cooker2Pin = 5;
int cooker3Pin = 4;
int cooker4Pin = 0;

bool old_state_manual = true;
int cookerPins[] = {16,5,4,0};
int numCookerPins = 4;
int powerPin = 2;
int auto_manual_selector_pin = 14;

long low_timingsList[] = {1000,50000,50000,47000,48000,38000,39000,35000,37000,32000,34000,28000,31000,20000,24000,9000,15000};
long high_timingsList[] = {0,1500,1500,4000,3000,5000,4000,8000,7000,11000,9000,15000,13000,23000,19000,35000,28000};
//String selectorsList[] = {"0","1","1.","2","2.","3","3.","4","4.","5","5.","6","6.","7","7.","8","8."};
int selectorsQty = 17;

long high_millis1 = millis();
long low_millis1 = millis();

long high_millis2 = millis();
long low_millis2 = millis();

long high_millis3 = millis();
long low_millis3 = millis();

long high_millis4 = millis();
long low_millis4 = millis();


long pin1HighTime = 0;
long pin1LowTime = 0;

long pin2HighTime = 0;
long pin2LowTime = 0;

long pin3HighTime = 0;
long pin3LowTime = 0;

long pin4HighTime = 0;
long pin4LowTime = 0;


void set_timing1( int value){
  Serial.println("set_timing1");
  Serial.println(value);
  if (value == 5){
    digitalWrite(cooker1Pin, 1);
    high_millis1 = millis();
    low_millis1 = millis();
    pin1LowTime = 0;
    pin1HighTime = 0;
  }
  else{
        high_millis1 = millis();
        low_millis1 = millis();
        pin1LowTime = low_timingsList[value];
        pin1HighTime = high_timingsList[value];
      }
    }


void set_timing2( int value){
  Serial.println("set_timing2");
  Serial.println(value);
  if (value == 5){
    digitalWrite(cooker2Pin, 1);
    high_millis2 = millis();
    low_millis2 = millis();
    pin2LowTime = 0;
    pin2HighTime = 0;
  }
  else{

        high_millis2 = millis();
        low_millis2 = millis();
        pin2LowTime = low_timingsList[value];
        pin2HighTime = high_timingsList[value];
      }
    }


void set_timing3( int value){
  Serial.println("set_timing3");
  Serial.println(value);
 if (value == 5){
    digitalWrite(cooker3Pin, 1);
    high_millis3 = millis();
    low_millis3 = millis();
    pin3LowTime = 0;
    pin3HighTime = 0;
  }
  else{

        high_millis3 = millis();
        low_millis3 = millis();
        pin3LowTime = low_timingsList[value];
        pin3HighTime = high_timingsList[value];
      }
}


void set_timing4( int value){
  Serial.println("set_timing4");
  Serial.println(value);
 if (value == 5){
    digitalWrite(cooker4Pin, 1);
    high_millis4 = millis();
    low_millis4 = millis();
    pin4LowTime = 0;
    pin4HighTime = 0;
  }
  else{
        high_millis4 = millis();
        low_millis4 = millis();
        pin4LowTime = low_timingsList[value];
        pin4HighTime = high_timingsList[value];
      }
}


void stove(const char* payload, const size_t length) {
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);
  String cooker = doc["cooker"];
  int value = doc["value"];
  if(cooker == "cooker1"){
    Serial.println("cooker1");
        set_timing1(value);
   }
     else if (cooker == "cooker2"){
        Serial.println("cooker2");
        set_timing2(value);
   } else if (cooker == "cooker3"){
        Serial.println("cooker3");
        set_timing3(value);
   } else if (cooker == "cooker4"){
        Serial.println("cooker4");
        set_timing4(value);
   } 
    String auto_manual_selector = doc["auto_manual_selector"];
    
    if( auto_manual_selector == "auto"){ 
    digitalWrite(auto_manual_selector_pin,0);
    delay(2000);
    digitalWrite(powerPin,1);
    old_state_manual = false;
    }else if(
    auto_manual_selector == "manual"){
      if (old_state_manual == false){
      digitalWrite(powerPin,0);
      for (int i=0; i<numCookerPins; i++){
        digitalWrite(cookerPins[i],0);
      }
      pin1LowTime = low_timingsList[0];
      pin1HighTime = high_timingsList[0];
      pin2LowTime = low_timingsList[0];
      pin2HighTime = high_timingsList[0];
      pin3LowTime = low_timingsList[0];
      pin3HighTime = high_timingsList[0];
      pin4LowTime = low_timingsList[0];
      pin4HighTime = high_timingsList[0];
      delay(2000);
      digitalWrite(auto_manual_selector_pin,1);

      old_state_manual = true;
    }
    }
 }  



SocketIoClient sc;
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
bool res = wifiManager.autoConnect("stove");

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
  for (int i=0; i<numCookerPins; i++){
    pinMode(cookerPins[i],OUTPUT);
    digitalWrite(cookerPins[i],0);
  }
  pinMode(powerPin,OUTPUT);
  pinMode(auto_manual_selector_pin,OUTPUT);
  digitalWrite(powerPin,0);
  digitalWrite(auto_manual_selector_pin,1);

  ////  Esp setup
//  initializeEmit(deviceType, serial);
  sc.on("stove",stove);

}

void loop(){
  sc.loop();
  Pins();
}

void Pins(){
  if (high_millis1 + pin1HighTime > millis()){
    digitalWrite(cooker1Pin,1);
    low_millis1 = millis();
  }
  else if (high_millis1 + pin1HighTime < millis()){
    if (low_millis1 + pin1LowTime > millis()){
      digitalWrite(cooker1Pin,0);
    }
    else{
      high_millis1 = millis();
      low_millis1 = millis();
    }
  }

  if (high_millis2 + pin2HighTime > millis()){
    digitalWrite(cooker2Pin,1);
    low_millis2 = millis();
  }
  else if (high_millis2 + pin2HighTime < millis()){
    if (low_millis2 + pin2LowTime > millis()){
      digitalWrite(cooker2Pin,0);
    }
    else{
      high_millis2 = millis();
      low_millis2 = millis();
    }
  }

  if (high_millis3 + pin3HighTime > millis()){
    digitalWrite(cooker3Pin,1);
    low_millis3 = millis();
  }
  else if (high_millis3 + pin3HighTime < millis()){
    if (low_millis3 + pin3LowTime > millis()){
      digitalWrite(cooker3Pin,0);
    }
    else{
      high_millis3 = millis();
      low_millis3 = millis();
    }
  }

  if (high_millis4 + pin4HighTime > millis()){
    digitalWrite(cooker4Pin,1);
    low_millis4 = millis();
  }
  else if (high_millis4 + pin4HighTime < millis()){
    if (low_millis4 + pin4LowTime > millis()){
      digitalWrite(cooker4Pin,0);
    }
    else{
      high_millis4 = millis();
      low_millis4 = millis();
    }
  }

//   Serial.print(" ");
//   Serial.println(high_millis2);
//  Serial.print(cooker1Pin);
//  Serial.print(" ");
//  Serial.print(cooker2Pin);
//  Serial.print(" ");
//  Serial.print(cooker3Pin);
//  Serial.print(" ");
//  Serial.print(cooker4Pin);
//  Serial.println(" ");
}
