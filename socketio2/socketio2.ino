#include <ArduinoJson.h>
#include <SocketIoClient.h>
#include <Arduino.h>
#include <WiFiManager.h>

#define USER_SERIAL Serial

//D8, D7, D5, D3, RX


const byte bulb = 16; //id_device 2       //D0
const byte fun = 12;  // id_device 6      //D6
const byte door = 5;   // id_device 7     //D1
const byte pir = 4;                       //D2
int pir_val = 0;
bool bulbS = false;
bool door_flag = 0;
bool doorS ;
bool eventHandled = false;
  
SocketIoClient webSocket;

void setup() {
  USER_SERIAL.begin(115200);

  pinMode(pir, INPUT_PULLUP);
  pinMode(door, INPUT_PULLUP);
  pinMode(bulb, OUTPUT);
  pinMode(fun, OUTPUT);
  digitalWrite(bulb, LOW);

  WiFiManager wm;
  bool res = wm.autoConnect("smart");

  if (!res) {
    Serial.println("Failed to connect");
  }
  else {
    Serial.println("Connected to Wi-Fi");
  }

  webSocket.begin("192.168.1.194", 8000);
  webSocket.on("receive_device", controlled);
}
bool test = true;
unsigned long lastTime;
void loop() {
  pir_val = digitalRead(pir);
  doorS = digitalRead(door);
  webSocket.loop();
  if(doorS == 1 && door_flag == 0){
    door_flag = 1;
    USER_SERIAL.println("door closed");
    webSocket.emit("send_device", "{\"id_device\":\"7\", \"device_state\":\"1\"}");
  }
  if(doorS == 0 && door_flag == 1 ){
    door_flag = 0;
    USER_SERIAL.println("door opened");
    webSocket.emit("send_device", "{\"id_device\":\"7\", \"device_state\":\"0\"}");
  }
  if (pir_val == 1 && bulbS == false && test == true) {
    digitalWrite(bulb, HIGH);
    USER_SERIAL.println("Bulb turned ON");
    bulbS = true;
    eventHandled = false; // reset event handling flag
    webSocket.emit("send_device", "{\"id_device\":\"2\", \"device_state\":\"1\"}");
  }
  if (pir_val == 0 && bulbS == true && test == true) {
    digitalWrite(bulb, LOW);
    USER_SERIAL.println("Bulb turned OFF");
    bulbS = false;
    eventHandled = false; // reset event handling flag
    webSocket.emit("send_device", "{\"id_device\":\"2\", \"device_state\":\"0\"}");
  }

  if (bulbS && !eventHandled) {
    // If bulb is ON and event is not handled
    // Perform additional event of the bulb here
    // For example, you can send a message or perform any action
    USER_SERIAL.println("Performing additional event of the bulb");
    eventHandled = true; // Set the flag to indicate event is handled
  }
}

void controlled(const char* device, size_t length) {
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, device);

  int idDevice = doc["id_device"];
  int deviceState = doc["device_state"];

  USER_SERIAL.print("id_device: ");
  USER_SERIAL.println(idDevice);
  USER_SERIAL.print("device_state: ");
  USER_SERIAL.println(deviceState);

  switch (idDevice) {
    case 2:
    if (deviceState == 1&& bulbS == false) {
      digitalWrite(bulb, HIGH);
      USER_SERIAL.print("DevS: ");
      USER_SERIAL.println(deviceState);
      USER_SERIAL.println("Bulb turned ON DEV");
      webSocket.emit("send_device", "{\"id_device\":\"2\", \"device_state\":\"1\"}");
      bulbS = true;
      eventHandled = false; // reset event handling flag
      test = false;
    }
     if (deviceState == 0 && bulbS == true) {
      digitalWrite(bulb, LOW);
      USER_SERIAL.print("DevS: ");
      USER_SERIAL.println(deviceState);
      USER_SERIAL.println("Bulb turned OFF DEV");
      webSocket.emit("send_device", "{\"id_device\":\"2\", \"device_state\":\"0\"}");
      bulbS = true;
      eventHandled = false; // reset event handling flag
      test = true;
    } break;
    case 6: 
if (deviceState == 1) {
      digitalWrite(fun, HIGH);
    }
     if (deviceState == 0) {
      digitalWrite(fun, LOW);
    }
    break;
  }
}
