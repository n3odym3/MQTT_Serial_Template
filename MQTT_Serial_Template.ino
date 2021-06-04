#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "wifi.h"
WiFiClient espClient;
PubSubClient client(espClient);

int maxtry = 30; //Amount of retries before using the "Serial Only" mode
bool WiFiConnected = false;

//WIFI SETUP=============================
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(HostName);
  WiFi.begin(ssid, password);

  for (int i = 0; i < maxtry; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      WiFiConnected = true;
      digitalWrite(LED_BUILTIN, HIGH);
      return;
    }
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    Serial.print(".");
  }
  WiFiConnected = false;
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("WiFi not connected");
  Serial.println("Serial Only mode");
}
//WIFI SETUP=============================

//MQTT RECONNECT====================================================
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(HostName)) {
      Serial.println("connected");
      
      //Don't forget to subscribe if you add a new Topic on wifi.h
      //SUBSCRIBE to Topics-------------
      client.subscribe(Topic1);
      client.subscribe(Topic2);
      digitalWrite(LED_BUILTIN, HIGH);
      //--------------------------------
      
    } else {
      digitalWrite(LED_BUILTIN, LOW);
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
//MQTT RECONNECT====================================================

//MQTT LOOP========================
//Only loop if WiFi is connected
void MQTT_loop() {
  if (WiFiConnected == true) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  }
}
//MQTT LOOP========================

//CALLBACK==================================================================
//Callback function called when a new payload is sent on a subscribed topic
void callback(String topic, byte* message, unsigned int length) {
  String messageTemp;
  //Read the Payload
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  reaction(topic, messageTemp);
}
//CALLBACK==================================================================

//REACTION======================================================================================
void reaction(String topic, String payload) {
  if (topic == Topic1) {
    Serial.print("Topic1 : ");
    Serial.println(payload);
  }

  else if (topic == Topic2) {
    Serial.println("Topic2");
    if(payload == "0"){
      Serial.println("Response to 0");
    }
    else if(payload == "1"){
      Serial.println("Response to 1");
    }
    else{
      Serial.println("Payload should be 0 or 1"); 
    }
  }

  else{
    //Publish the "payload" on the povided "topic" if the topic doesn't match Topic1 or Topic2
    Serial.print("Publishing : ");
    Serial.println(payload);
    Serial.print("On topic : ");
    Serial.println(topic);
    client.publish(topic.c_str() ,payload.c_str());
  }
}
//REACTION======================================================================================

//SETUP=================================
void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);
  pinMode(LED_BUILTIN, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}
//SETUP=================================

//LOOP===========================================================================
void loop() {
  MQTT_loop();
  if (Serial.available()) {
    String input = Serial.readString();
    Serial.print("Input : ");
    Serial.println(input);
    String topic = input.substring(0,input.indexOf(":"));
    String payload = input.substring(input.indexOf(":")+1,input.indexOf("\n"));
    reaction(topic, payload);
    Serial.read();
  }
}
//LOOP===========================================================================
