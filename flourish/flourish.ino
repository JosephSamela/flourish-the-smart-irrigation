#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

const char* ssid = "<ssid>";
const char* password = "<password";

WebServer server(80);

const int led = 13;
const int relay = 5;

void setup(void) {
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);

  Serial.begin(115200);

  // Connect to wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", []() {
    server.send(200, "text/plain", "Welcome to Joe's irrogation system!\n\nSend a GET request to `/on` to start irrigation.\nSend a GET request to `/off` to stop irrigation");
  });

  server.on("/on", []() {
    digitalWrite(relay, LOW);
    server.send(200, "text/plain", "turn irrigation on");
  });

  server.on("/off", []() {
    digitalWrite(relay, HIGH);
    server.send(200, "text/plain", "turn irrigation off");
  });

  server.onNotFound([]() {
    digitalWrite(led, 1);
    String message = "Route Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
    digitalWrite(led, 0);
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(100);//allow the cpu to switch to other tasks
}
