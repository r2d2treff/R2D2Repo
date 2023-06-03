//#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>

#include "RadMotor.h"
#include "SchrittMotor.h"

#ifndef STASSID
#define STASSID "R2D2Net"
#define STAPSK "R2D2Pass"
#endif

//#define LEFTMOTOR
#define RIGHTMOTOR
//#define FRONTMOTOR

ESP8266WebServer server(80);

RadMotor rad(4, 5);
#ifdef FRONTMOTOR
  Schrittmotor lenker(14, 12, 13);
#endif

WiFiUDP Udp;

uint32_t stopTime = 0;

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
    //on root
    server.send(200, "text/plain", "Rufe /Forward auf fuer forwaerts\nRufe /Backward auf fuer Rueckwaertz\nRufe /SetAngle auf fuer winkel Setzen\nRufe /Stop auf fuer Stop\nRufe /State auf fuer Status");
  });
  server.onNotFound([](){
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++) { message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; }
    server.send(404, "text/plain", message);
  });

  server.on("/Forward", []() {
    bool hasDrivingMillis = false;
    uint32_t drivingMillis = 0;
    bool hasSpeed = false;
    uint8_t speed = 0;
    for(int i = 0; i < server.args(); i++){
      if(String("distance") == server.argName(i)){
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
        rad.Backward(speed);
      #endif
      #ifdef RIGHTMOTOR
        rad.Forward(speed);
      #endif
      #ifdef FRONTMOTOR
        rad.Backward(speed);
      #endif
      stopTime = millis() + drivingMillis;
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
      if(String("distance") == server.argName(i)){
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
        rad.Forward(speed);
      #endif
      #ifdef RIGHTMOTOR
        rad.Backward(speed);
      #endif
      #ifdef FRONTMOTOR
        rad.Forward(speed);
      #endif
      stopTime = millis() + drivingMillis;
      server.send(200, "text/plain", "doing");
    }
    else
      server.send(404, "text/plain", "wrong Arguments");
  });
  
#ifdef FRONTMOTOR
  server.on("/SetAngle", []() {
    bool hasAngle = false;
    uint32_t angle = 0;
    for(int i = 0; i < server.args(); i++){
      if(String("angle") == server.argName(i)){
        hasAngle = true;
        angle = server.arg(i).toInt();
      }
    }
    if(hasAngle){
      server.send(200, "text/plain", "doing");
    }
    else
      server.send(404, "text/plain", "wrong Arguments");
  });
#endif
  server.on("/Stop", []() {

    rad.Stop();

    server.send(200, "text/plain", "done");
  });
  server.on("/State", []() {
    if(stopTime < millis())
      server.send(200, "text/plain", "stopped");
    else
      server.send(200, "text/plain", "not stopped");
  });

  server.begin();
  Serial.println("HTTP server started");
}

uint32_t naechsteUDPSendeZeit = 0;

void loop(void) {
  server.handleClient();
  if(naechsteUDPSendeZeit < millis()){
    naechsteUDPSendeZeit = millis() + 5000;
    Udp.beginPacket("255.255.255.255" ,90);
    #ifdef LEFTMOTOR
      Udp.write("Hi I Am Left");
    #endif
    #ifdef RIGHTMOTOR
      Udp.write("Hi I Am Right");
    #endif
    #ifdef FRONTMOTOR
      Udp.write("Hi I Am Front");
    #endif
    Udp.endPacket();
  }
  if(stopTime < millis()){
    rad.Stop();
  }
}