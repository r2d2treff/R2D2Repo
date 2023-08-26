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

#define steppDadMaxCounter 155
int8_t winkel = 91;
uint16_t steppDadCounter = 0;
int8_t sollWinkel = 0;
int8_t winkelLookUp[7] = {
  -90,
  0,
  -60,
  90,
  -30,
  30,
  60
};
enum class Sensorwert : uint8_t{
  nichts = 0,   //000
  rechts =  1,  //001
  rechts1 = 3,  //011
  rechts2 = 5,  //101
  grade = 2,    //010
  links2 = 6,   //110
  links1 = 7,   //111
  links = 4     //100
};

Sensorwert readState(){
  bool ct0 = !digitalRead(T0) ? 0 : 4;
  bool ct1 = !digitalRead(T1) ? 0 : 1;
  bool ct2 = !digitalRead(T2) ? 0 : 2;
  
  return (Sensorwert)((!digitalRead(T1) ? 0 : 2) + (!digitalRead(T2) ? 0 : 4) + (!digitalRead(T0) ? 0 : 1) );
}

#define STEPTIME 2500
uint32_t nextStepTime = 0;


void choseAktion(){
  if(nextStepTime > micros())
    return;

  nextStepTime = micros() + STEPTIME;

  Sensorwert currentState = readState();

  if(currentState != Sensorwert::nichts){
    if(winkel != winkelLookUp[(uint8_t)currentState-1]){
      Serial.print(winkel); Serial.print(";"); Serial.println(winkelLookUp[(uint8_t)currentState-1]);
    }
    winkel = winkelLookUp[(uint8_t)currentState-1];
  }

  if(winkel == 91){
    sm.Enable(false);
    sm.Forward();
  }else{
    if(sollWinkel < winkel){
      sm.Forward();
      steppDadCounter--;
      if(steppDadCounter > steppDadMaxCounter){
        winkel--;
        steppDadCounter = steppDadMaxCounter;
      }
    }else if(sollWinkel > winkel){
      sm.Backward();
      steppDadCounter++;
      if(steppDadCounter == steppDadMaxCounter){
        winkel++;
        steppDadCounter = 0;
      }
    }
  }

}

void setup() {
  Serial.begin(115200);

  pinMode(T0, INPUT);
  pinMode(T1, INPUT);
  pinMode(T2, INPUT);

  sm.Enable(true);

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
    int8_t state = 0;
    for(int i = 0; i < server.args(); i++){
      if(String("state") == server.argName(i)){
        hasState = true;
        state = server.arg(i).toInt();
      }
    }
    if(hasState && state <=90 && state >= -90){
      sollWinkel = state;
      server.send(200, "text/plain", "doing");
    }
    else
      server.send(404, "text/plain", "wrong Arguments");
  });
  server.on("/State", []() {
    String s = String(winkel);
    s += ",";
    s += winkel == sollWinkel ? 's' : 'm';
    server.send(200, "text/plain", s.c_str());
  });

  server.begin();
  Serial.println("HTTP server started");
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
