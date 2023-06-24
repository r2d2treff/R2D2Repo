//#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>

#include "RadMotor.h"

#ifndef STASSID
#define STASSID "R2D2Net"
#define STAPSK "R2D2Pass"
#endif

#define REFRESHRATE 5000

//#define LEFTMOTOR
//#define RIGHTMOTOR
#define FRONTMOTOR

ESP8266WebServer server(80);

RadMotor rad(4, 5, 14);


void setup(void) {
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
    server.send(200, "text/plain", "Forward,Backward,Stop,State");
  });

  server.onNotFound([](){
    server.send(404, "text/plain", "Forward,Backward,Stop,State");
  });

  server.on("/Forward", []() {
    bool hasDrivingMillis = false;
    uint32_t drivingMillis = 0;
    bool hasSpeed = false;
    uint8_t speed = 0;
    for(int i = 0; i < server.args(); i++){
      if(String("steps") == server.argName(i)){
        hasDrivingMillis = true;
        drivingMillis = server.arg(i).toInt();
      }
      if(String("speed") == server.argName(i)){
        hasSpeed = true;
        speed = server.arg(i).toInt();
      }
    }
    if(hasDrivingMillis && hasSpeed){
      #ifdef LEFTMOTOR
        rad.Backward(speed, drivingMillis);
      #endif
      #ifdef RIGHTMOTOR
        rad.Forward(speed, drivingMillis);
      #endif
      #ifdef FRONTMOTOR
        rad.Backward(speed, drivingMillis);
      #endif
      server.send(200, "text/plain", "doing");
    }
    else
      server.send(404, "text/plain", "wrong Arguments");
  });

  server.on("/Backward", []() {
    bool hasDrivingMillis = false;
    uint32_t drivingMillis = 0;
    bool hasSpeed = false;
    uint8_t speed = 0;
    for(int i = 0; i < server.args(); i++){
      if(String("steps") == server.argName(i)){
        hasDrivingMillis = true;
        drivingMillis = server.arg(i).toInt();
      }
      if(String("speed") == server.argName(i)){
        hasSpeed = true;
        speed = server.arg(i).toInt();
      }
    }
    if(hasDrivingMillis && hasSpeed){
      
      #ifdef LEFTMOTOR
        rad.Forward(speed, drivingMillis);
      #endif
      #ifdef RIGHTMOTOR
        rad.Backward(speed, drivingMillis);
      #endif
      #ifdef FRONTMOTOR
        rad.Forward(speed, drivingMillis);
      #endif
      server.send(200, "text/plain", "doing");
    }
    else
      server.send(404, "text/plain", "wrong Arguments");
  });

  server.on("/Stop", []() {
    rad.Stop();
    server.send(200, "text/plain", "done");
  });

  server.on("/State", []() {
    if(rad.isDriving())
      server.send(200, "text/plain", "not stopped");
    else
      server.send(200, "text/plain", "stopped");
  });

  server.begin();
  Serial.println("HTTP server started");
}

uint32_t naechsteUDPSendeZeit = 0;

void loop(void) {
  server.handleClient();
  
  if(naechsteUDPSendeZeit < millis()){
    naechsteUDPSendeZeit = millis() + REFRESHRATE;
    Udp.beginPacket("255.255.255.255" ,90);
    #ifdef LEFTMOTOR
      Udp.write(("typeId=1,refreshRate=" + String(REFRESHRATE)).c_str());
    #endif
    #ifdef RIGHTMOTOR
      Udp.write(("typeId=2,refreshRate=" + String(REFRESHRATE)).c_str());
    #endif
    #ifdef FRONTMOTOR
      Udp.write(("typeId=3,refreshRate=" + String(REFRESHRATE)).c_str());
    #endif
    Udp.endPacket();
  }

  rad.update();
}