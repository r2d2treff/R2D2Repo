#include <ESP8266WebServer.h>
#include <WiFiUdp.h>

#ifndef STASSID
#define STASSID "R2D2Net"
#define STAPSK "R2D2Pass"
#endif

#define REFRESHRATE 5000
uint32_t naechsteUDPSendeZeit = 0;

ESP8266WebServer server(80);
WiFiUDP Udp;

#include "Schrittmotor.h"

Schrittmotor sm(14, 4, 5);
#define T0 16
#define T1 12
#define T2 13

uint8_t sollState = 0;
uint8_t currentState = 0;
uint8_t readState(){
  bool ct0 = digitalRead(T0) ? 0 : 2;
  bool ct1 = digitalRead(T1) ? 0 : 1;
  bool ct2 = digitalRead(T2) ? 0 : 4;
  if(ct0 == 0 && ct1 == 0 && ct2 == 0)
    currentState = 0;
  else{
    if(ct0)
      currentState = currentState | 2;
    if(ct1)
      currentState = currentState | 1;
    if(ct2)
      currentState = currentState | 4;
  }
  return currentState;
}

#define STEPTIME 5
uint32_t nextStepTime = 0;

uint8_t drehrichtung = 0;
void choseAktion(){
  if(nextStepTime > millis())
    return;

  nextStepTime = millis() + STEPTIME;

  uint8_t currentState = readState();
  
  if(currentState != sollState){
    if(currentState != 0)
      drehrichtung = sollState > currentState ? 1 : 2;
  }else {
    drehrichtung = 0;
  }

  switch(drehrichtung){
    case 0:
      break;
    case 1:
      sm.Forward();
      break;
    case 2:
      sm.Backward();
      break;
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);

  uint32_t VerbindungsVersucheCounter =0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    VerbindungsVersucheCounter++;
    if(VerbindungsVersucheCounter > 100)
      system_restart();
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(STASSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", [](){
    server.send(200, "text/plain", "Direction,State");
  });
  server.onNotFound([](){
    server.send(404, "text/plain", "Direction,State");
  });
  server.on("/Direction", []() {
    bool hasState = false;
    uint8_t state = 0;
    for(int i = 0; i < server.args(); i++){
      if(String("state") == server.argName(i)){
        hasState = true;
        state = server.arg(i).toInt();
      }
    }
    if(hasState && state != 0){
      sollState = state;
      drehrichtung = 1;
      server.send(200, "text/plain", "doing");
    }
    else
      server.send(404, "text/plain", "wrong Arguments");
  });
  server.on("/State", []() {
    switch(readState()){
      case 0:
        server.send(200, "text/plain", "moving");
        break;
      case 1:
        server.send(200, "text/plain", "1");
        break;
      case 2:
        server.send(200, "text/plain", "2");
        break;
      case 3:
        server.send(200, "text/plain", "3");
        break;
      case 4:
        server.send(200, "text/plain", "4");
        break;
      case 5:
        server.send(200, "text/plain", "5");
        break;
      case 6:
        server.send(200, "text/plain", "6");
        break;
      case 7:
        server.send(200, "text/plain", "7");
        break;
    }
  });

  pinMode(T0, INPUT);
  pinMode(T1, INPUT);
  pinMode(T2, INPUT);

  server.begin();
  Serial.println("HTTP server started");
  
  sm.Enable(false);
}

void loop() {
  server.handleClient();
  
  if(naechsteUDPSendeZeit < millis()){
    naechsteUDPSendeZeit = millis() + REFRESHRATE;
    Udp.beginPacket("255.255.255.255" ,90);
    Udp.write(("typeId=4,refreshRate=" + String(REFRESHRATE)).c_str());
    Udp.endPacket();
  }

  choseAktion();
}
