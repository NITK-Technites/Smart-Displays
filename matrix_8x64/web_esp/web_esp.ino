#include<ESP8266WiFi.h>
#include<ESP8266WebServer.h>
#include<ESP8266mDNS.h>

ESP8266WebServer server(80);

char* username = "NITK-NET";
char* password = "2K16NITK";
String msg = "kamal";
String html_page;

void setup()
{
  WiFi.begin(username, password);
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(username);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("IP address of ESP8266 is: ");
  Serial.print(WiFi.localIP());
  if(MDNS.begin("smartdisplays"))
  {
    Serial.println("MDNS Responder Started");
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
  html_page = "<!DOCTYPE html> <html> <head> <title>Smart Displays</title> </head> <style type=\"text/css\"> .center{ text-align: center; } .head-color{ color : #092040; } </style> <body> <h1 class = \"center head-color\"><strong>TECHNITES - Smart Displays</strong></h1><br><br><br><br><br> <form action = \"message\" class=\"center\" method=\"GET\"> <p>Message being Dispalyed : ";
  html_page += msg;
  html_page += "</p> <p>Type the message to be displayed and press send</p> <input type=\"text\" name=\"msg\"/> <input type=\"submit\" value=\"send\"/> </body> </html>";
  server.send(200, "text/html", html_page);
}

void update_msg()
{
  msg = server.arg("msg");
  Serial.println(msg);
  server.send(200, "text/html", html_page);
}

