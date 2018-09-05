#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#include "secrets.h"

// Esp8266 pinouts
#define ESP8266_GPIO2   2  // LED
#define ESP8266_GPIO4   4  // Relay
#define ESP8266_GPIO5   5  // Optocoupler input
#define LED_PIN         ESP8266_GPIO2

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

volatile int gateState = LOW;

ESP8266WebServer server(80);
HTTPClient http;

void setRelayState(int nextState) {
  digitalWrite(ESP8266_GPIO4, nextState);
}

void cycleRelay() {
  setRelayState(1);
  delay(100);
  setRelayState(0);
}

int getState() {
  return digitalRead(ESP8266_GPIO5);
}

bool isClosed() {
  return getState() == LOW;
}

bool isOpen() {
  return getState() == HIGH;
}

void handleStatus() {
  server.send(200, "text/plain", String(getState()));
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
  server.on("/", handleStatus);
  server.on("/open", handleOpen);
  server.on("/close", handleClose);
  server.on("/cycle", handleCycle);
  server.onNotFound(handleNotFound);
}

long lastStateCheck = 0;

void checkState() {
  unsigned long currentStateCheck = millis();

  if (currentStateCheck - lastStateCheck > 1000) {
    lastStateCheck = currentStateCheck;

    int currentGateState = getState();
    bool didStateChange = gateState != currentGateState;

    if (didStateChange) {
      gateState = currentGateState;
      onStateChange(bool(currentGateState));
    }
  }
}

void onStateChange(bool value) {
  String body = "{ \
    \"service\": \"switch-service\", \
    \"characteristic\": \"On\", \
    \"value\": " + String(value) + "}";

  http.begin(NOTIFICATION_URL);
  http.addHeader("Content-Type", "application/json");
  http.POST(body);
  http.end();
}

void awaitWifiConnected() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Waiting to connect...");
  }
}

void setupHardware() {
  Serial.begin(9600);

  pinMode(ESP8266_GPIO4, OUTPUT);
  pinMode(ESP8266_GPIO5, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(ESP8266_GPIO4, gateState);
}

void setup() {
  awaitWifiConnected();
  setupHardware();
  configureRoutes();

  server.begin();
  Serial.println("Listening on http://" + String(WiFi.localIP()));
}

void loop() {
  server.handleClient();
}
