/*
  Smart Displays : 8x64 LED Matrix
  ESP8266 Code:
       Sends data received from client to Arduino which is controlling Matrix using
       hardware serial

  Code by:
  K S S M KAMAL
  NIT SURATHKAL
  Dept of ECE
*/

#include<ESP8266WiFi.h>
#include<ESP8266WebServer.h>
#include<ESP8266mDNS.h>

ESP8266WebServer server(80);

char* username = "NITK-NET";
char* password = "2K16NITK";
String msg = "kamal";
String html_page;

unsigned long present_ms = 0;
unsigned long last_ms = 0;
unsigned long send_time = 60000;

String slice = "0";
String font = "1";

void setup()
{
  WiFi.begin(username, password);
  Serial.begin(115200);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.localIP());
  if (MDNS.begin("smartdisplays"))
  {
  }
  server.on("/", send_html);
  server.on("/message", update_msg);
  server.begin();
}


void loop()
{
  server.handleClient();
}

void send_html()
{
  String html_page ="<!DOCTYPE html> <html> <head> <title>Smart Displays</title> </head> <style type=\"text/css\"> .center{ text-align: center; } .head-color{ color : #092040; } </style> <body> <h1 class = \"center head-color\"><strong>TECHNITES - SMART DISPLAYS</strong></h1><br><br> <form action = \"message\" class=\"center\" method=\"GET\"> <p>Message being Dispalyed : ";
html_page += msg;
html_page +="</p> <p><strong>Type the message to be displayed and press send</strong></p> <input type=\"text\" name=\"msg\"/> <br> <p><strong>Select the animation</strong></p> <br> <input type = \"radio\" name = \"slice\" value = \"1\">Slice <input type = \"radio\" name = \"slice\" value = \"0\">Scroll <br> <p><strong>Select the font</strong></p> <input type = \"radio\" name = \"font\" value = \"0\">8x8 Font <input type = \"radio\" name = \"font\" value = \"1\">7x5 Font <br><br> <input type=\"submit\" value=\"send\"/> </body> </html>";

   server.send(200, "text/html", html_page);
}

void update_msg()
{
  msg = server.arg("msg");
  slice = server.arg("slice");
  font = server.arg("font");
  Serial.println(msg + "#" + slice + "#" + font);
  server.send(200, "text/html", html_page);
}

