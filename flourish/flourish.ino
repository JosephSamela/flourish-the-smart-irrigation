#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

const char* ssid = "<ssid>";
const char* password = "<password";

WebServer server(80);

const int led = 2;
const int relay = 17;
const int reset = 26;

int requestedState = 0; // 0=OFF, 1=ON
int currentState = 0;   // 0=OFF, 1=ON

void setup(void) {
  // Setup GPIO pin modes
  pinMode(led, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(reset, OUTPUT);
  // Initialize GPIO positions
  digitalWrite(led, HIGH);
  digitalWrite(relay, HIGH);
  digitalWrite(reset, HIGH);
  
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
    server.send(200, "text/plain", "Welcome to Joe's irrogation system!\n\nSend a GET request to `/on` to start irrigation.\nSend a GET request to `/off` to stop irrigation\n");
  });

  server.on("/on", []() {
    digitalWrite(relay, LOW);
    server.send(200, "text/plain", "turn irrigation on\n");
  });

  server.on("/off", []() {
    digitalWrite(relay, HIGH);
    server.send(200, "text/plain", "turn irrigation off\n");
  });

  server.on("/status", []() {
    digitalWrite(led, HIGH);
    server.send(200, "text/plain", "[{\"zone\":1,\"state\":"+ String(currentState) + "}]\n");
    digitalWrite(led, LOW);
  });

  server.on("/setState", []() {
    digitalWrite(led, HIGH);
    
    if (server.arg("zone").toInt() == 1) {
      requestedState = server.arg("value").toInt();  
    }

    server.send(200, "text/plain", "[{\"zone\":1,\"state\":"+ String(currentState) + "}]\n");
    digitalWrite(led, LOW);
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

void toggleState() {
  if (currentState == 0){
    digitalWrite(relay, LOW);
  } else if (currentState == 1){
    digitalWrite(relay, HIGH);
  }
}

void loop(void) {
  server.handleClient();
  
  if (requestedState != currentState) {
      toggleState();
      currentState = requestedState;
  }
  
  delay(10);//allow the cpu to switch to other tasks
}
