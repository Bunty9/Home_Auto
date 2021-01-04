// Code by Bipin S C
// Github : https://github.com/Bunty9

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsClient.h> //  https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <ArduinoJson.h> // https://github.com/kakopappa/sinric/wiki/How-to-add-dependency-libraries
#include <StreamString.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;

#include <AceButton.h> // https://github.com/bxparks/AceButton
using namespace ace_button;

#define MyApiKey ".....-........-.........-......." // TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard
#define MySSID "......." // TODO: Change to your Wifi network SSID
#define MyWifiPassword "......" // TODO: Change to your Wifi network password

#define HEARTBEAT_INTERVAL 300000 // 5 Minutes 
//Switch
const int BUTTON1_PIN = 14;
const int BUTTON2_PIN = 12;
const int BUTTON3_PIN = 13;
const int BUTTON4_PIN = 3;
//Relay
const int RELAY1_PIN = 5;
const int RELAY2_PIN = 4;
const int RELAY3_PIN = 0;
const int RELAY4_PIN = 2;

//deviceId
String device_ID_1 = "device id 1";
String device_ID_2 = "device id 2";
String device_ID_3 = "device id 3";
String device_ID_4 = "device id 4";

uint64_t heartbeatTimestamp = 0;
bool isConnected = false;

ButtonConfig config1;
AceButton button1(&config1);
ButtonConfig config2;
AceButton button2(&config2);
ButtonConfig config3;
AceButton button3(&config3);
ButtonConfig config4;
AceButton button4(&config4);


void handleEvent1(AceButton*, uint8_t, uint8_t);
void handleEvent2(AceButton*, uint8_t, uint8_t);
void handleEvent3(AceButton*, uint8_t, uint8_t);
void handleEvent4(AceButton*, uint8_t, uint8_t);

void setPowerStateOnServer(String deviceId, String value); // deviceId is the ID assgined to your smart-home-device in sinric.com dashboard. Copy it from dashboard and paste it here

 
void turnOn(String deviceId) {
  if (deviceId == device_ID_1) // Device ID of first device
  {
    Serial.print("Turn on device id: ");
    Serial.println(deviceId);
    digitalWrite(RELAY1_PIN, LOW);
  }
  if (deviceId == device_ID_2) // Device ID of second device
  {
    Serial.print("Turn on device id: ");
    Serial.println(deviceId);
    digitalWrite(RELAY2_PIN, LOW);
  }
  if (deviceId == device_ID_3) // Device ID of third device
  {
    Serial.print("Turn on device id: ");
    Serial.println(deviceId);
    digitalWrite(RELAY3_PIN, LOW);
  }
  if (deviceId == device_ID_4) // Device ID of fourth device
  {
    Serial.print("Turn on device id: ");
    Serial.println(deviceId);
    digitalWrite(RELAY4_PIN, LOW);
  }else {
    Serial.print("Turn on for unknown device id: ");
    Serial.println(deviceId);    
  }
}

   
void turnOff(String deviceId) {
  if (deviceId == device_ID_1) // Device ID of first device
  {
    Serial.print("Turn off Device ID: ");
    Serial.println(deviceId);
    digitalWrite(RELAY1_PIN, HIGH);
  }
  if (deviceId == device_ID_2) // Device ID of second device
  {
    Serial.print("Turn off Device ID: ");
    Serial.println(deviceId);
    digitalWrite(RELAY2_PIN, HIGH);
  }
  if (deviceId == device_ID_3) // Device ID of third device
  {
    Serial.print("Turn off Device ID: ");
    Serial.println(deviceId);
    digitalWrite(RELAY3_PIN, HIGH);
  }
  if (deviceId == device_ID_4) // Device ID of fourth device
  {
    Serial.print("Turn off Device ID: ");
    Serial.println(deviceId);
    digitalWrite(RELAY4_PIN, HIGH);
  }else {
     Serial.print("Turn off for unknown device id: ");
     Serial.println(deviceId);
  }
}



void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      isConnected = false;    
      Serial.printf("[WSc] Webservice disconnected from sinric.com!\n");
      break;
    case WStype_CONNECTED: {
      isConnected = true;
      Serial.printf("[WSc] Service connected to sinric.com at url: %s\n", payload);
      Serial.printf("Waiting for commands from sinric.com ...\n");        
      }
      break;
    case WStype_TEXT: {
        Serial.printf("[WSc] get text: %s\n", payload);
        // Example payloads

        // For Switch  types
        // {"deviceId":"xxx","action":"action.devices.commands.OnOff","value":{"on":true}} // https://developers.google.com/actions/smarthome/traits/onoff
        // {"deviceId":"xxx","action":"action.devices.commands.OnOff","value":{"on":false}}

#if ARDUINOJSON_VERSION_MAJOR == 5
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject((char*)payload);
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6        
        DynamicJsonDocument json(1024);
        deserializeJson(json, (char*) payload);      
#endif        
        String deviceId = json ["deviceId"];     
        String action = json ["action"];
        
        if(action == "action.devices.commands.OnOff") { // Switch 
            String value = json ["value"]["on"];
            Serial.println(value); 
            
            if(value == "true") {
                turnOn(deviceId);
            } else {
                turnOff(deviceId);
            }
        }
        else if (action == "test") {
            Serial.println("[WSc] received test command from sinric.com");
        }
      }
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      break;
    default: break;
  }
}

void setup() {
  Serial.begin(115200);
  //setup


  pinMode(BUTTON1_PIN, INPUT_PULLUP); // INPUT_PULLUP so no need a 10K resistor
  pinMode(BUTTON2_PIN, INPUT_PULLUP); // INPUT_PULLUP so no need a 10K resistor
  pinMode(BUTTON3_PIN, INPUT_PULLUP); // INPUT_PULLUP so no need a 10K resistor
  pinMode(BUTTON4_PIN, INPUT_PULLUP); // INPUT_PULLUP so no need a 10K resistor
  
  //setup the relays
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);
  
  digitalWrite(RELAY1_PIN,HIGH);
  digitalWrite(RELAY2_PIN,HIGH);
  digitalWrite(RELAY3_PIN,HIGH);
  digitalWrite(RELAY4_PIN,HIGH);

  
  WiFiMulti.addAP(MySSID, MyWifiPassword);
  Serial.println();
  Serial.print("Connecting to Wifi: ");
  Serial.println(MySSID);  

  // Waiting for Wifi connect
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if(WiFiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("WiFi connected. ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  config1.setEventHandler(button1Handler);
  config2.setEventHandler(button2Handler);
  config3.setEventHandler(button3Handler);
  config4.setEventHandler(button4Handler);


  button1.init(BUTTON1_PIN);
  button2.init(BUTTON2_PIN);
  button3.init(BUTTON3_PIN);
  button4.init(BUTTON4_PIN);



  // server address, port and URL
  webSocket.begin("iot.sinric.com", 80, "/"); //"iot.sinric.com", 80

  // event handler
  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("apikey", MyApiKey);
  
  // try again every 5000ms if connection has failed
  webSocket.setReconnectInterval(5000);   // If you see 'class WebSocketsClient' has no member named 'setReconnectInterval' error update arduinoWebSockets
}

void loop() {
  webSocket.loop();
  
  if(isConnected) {
      uint64_t now = millis();
      
      // Send heartbeat in order to avoid disconnections during ISP resetting IPs over night. Thanks @MacSass
      if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
          heartbeatTimestamp = now;
          webSocket.sendTXT("H");          
      }
  }  

  button1.check();
  button2.check();
  button3.check();
  button4.check();

}


void setPowerStateOnServer(String deviceId, String value) {
#if ARDUINOJSON_VERSION_MAJOR == 5
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6
  DynamicJsonDocument root(1024);
#endif

  root["deviceId"] = deviceId;
  root["action"] = "setPowerState";
  root["value"] = value;
  StreamString databuf;
#if ARDUINOJSON_VERSION_MAJOR == 5
  root.printTo(databuf);
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6
  serializeJson(root, databuf);
#endif
  webSocket.sendTXT(databuf);
}
  
void button1Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("EVENT1");
  switch (eventType) {
    case AceButton::kEventPressed:
      Serial.println("kEventPressed");
      setPowerStateOnServer(device_ID_1, "ON");
      digitalWrite(RELAY1_PIN, LOW);
      break;
    case AceButton::kEventReleased:
      Serial.println("kEventReleased");
      setPowerStateOnServer(device_ID_1, "OFF");
      digitalWrite(RELAY1_PIN, HIGH);
      break;
  }
}
void button2Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("EVENT2");
  switch (eventType) {
    case AceButton::kEventPressed:
      Serial.println("kEventPressed");
      setPowerStateOnServer(device_ID_2, "ON");
      digitalWrite(RELAY2_PIN, LOW);
      break;
    case AceButton::kEventReleased:
      Serial.println("kEventReleased");
      setPowerStateOnServer(device_ID_2, "OFF");
      digitalWrite(RELAY2_PIN, HIGH);
      break;
  }
}

void button3Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("EVENT3");
  switch (eventType) {
    case AceButton::kEventPressed:
      Serial.println("kEventPressed");
      setPowerStateOnServer(device_ID_3, "ON");
      digitalWrite(RELAY3_PIN, LOW);
      break;
    case AceButton::kEventReleased:
      Serial.println("kEventReleased");
      setPowerStateOnServer(device_ID_3, "OFF");
      digitalWrite(RELAY3_PIN, HIGH);
      break;
  }
}

void button4Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("EVENT4");
  switch (eventType) {
    case AceButton::kEventPressed:
      Serial.println("kEventPressed");
      setPowerStateOnServer(device_ID_4, "ON");
      digitalWrite(RELAY4_PIN, LOW);
      break;
    case AceButton::kEventReleased:
      Serial.println("kEventReleased");
      setPowerStateOnServer(device_ID_4, "OFF");
      digitalWrite(RELAY4_PIN, HIGH);
      break;
  }
}
