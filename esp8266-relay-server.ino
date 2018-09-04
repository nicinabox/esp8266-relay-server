#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Esp8266 pinouts
#define ESP8266_GPIO2   2  // LED
#define ESP8266_GPIO4   4  // Relay
#define ESP8266_GPIO5   5  // Optocoupler input
#define LED_PIN         ESP8266_GPIO2

const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASSWORD";

ESP8266WebServer server(80);

void setRelayState(int nextState) {
  digitalWrite(ESP8266_GPIO4, nextState);
}

void cycleRelay() {
  setRelayState(1);
  delay(100);
  setRelayState(0);
}

int getStatus() {
  return digitalRead(ESP8266_GPIO5);
}

bool isClosed() {
  return getStatus() == LOW;
}

bool isOpen() {
  return getStatus() == HIGH;
}

void handleRoot() {
  server.send(200, "text/plain", String(getStatus()));
}

void handleOpen() {
  if (isClosed()) {
    cycleRelay();
  }

  server.send(204);
}

void handleClose() {
  if (isOpen()) {
    cycleRelay();
  }

  server.send(204);
}

void handleCycle() {
  cycleRelay();

  server.send(204);
}

void handleNotFound() {
  server.send(404);
}

void configureRoutes() {
  server.on("/", handleRoot);
  server.on("/open", handleOpen);
  server.on("/close", handleClose);
  server.on("/cycle", handleCycle);
  server.onNotFound(handleNotFound);
}

void awaitWifiConnected() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Waiting to connect...");
  }
}

void setupRelay() {
  Serial.begin(9600);

  pinMode(ESP8266_GPIO4, OUTPUT);
  pinMode(ESP8266_GPIO5, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(ESP8266_GPIO4, 0);
}

void setup() {
  awaitWifiConnected();
  setupRelay();
  configureRoutes();

  server.begin();
  Serial.println("Listening on http://" + String(WiFi.localIP()));
}

void loop() {
  server.handleClient();
}
