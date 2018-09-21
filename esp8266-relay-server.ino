#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "secrets.h"

// SONOFF SV
const int RELAY_PIN = 12;
const int LED_PIN = 13;
const int SENSOR_PIN = 5;

ESP8266WebServer server(80);

void setRelayState(int nextState) {
  digitalWrite(RELAY_PIN, nextState);
}

void cycleRelay() {
  setRelayState(HIGH);
  delay(100);
  setRelayState(LOW);
}

int getSensorState() {
  return digitalRead(SENSOR_PIN);
}

int getGateState() {
  return 1 - getSensorState();
}

int setLEDState() {
  digitalWrite(LED_PIN, getSensorState());
}

bool isClosed() {
  return getGateState() == HIGH;
}

bool isOpen() {
  return getGateState() == LOW;
}

void sendStatus(int status) {
  server.send(200, "text/plain", String(status));
}

void handleStatus() {
  sendStatus(getGateState());
}

void handleOpen() {
  if (isClosed()) {
    cycleRelay();
  }

  handleStatus();
}

void handleClose() {
  if (isOpen()) {
    cycleRelay();
  }

  handleStatus();
}

void handleCycle() {
  cycleRelay();
  handleStatus();
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

void awaitWifiConnected() {
  Serial.setDebugOutput(true);
  Serial.println("Trying to connect " + String(WIFI_SSID));

  WiFi.mode(WIFI_STA);
  wifi_station_disconnect();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 9);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }
}

void setupHardware() {
  pinMode(LED_PIN, OUTPUT);
  setLEDState();

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  pinMode(SENSOR_PIN, INPUT_PULLUP);
}

void setup() {
  Serial.begin(9600);

  awaitWifiConnected();
  setupHardware();
  configureRoutes();

  server.begin();
}

void loop() {
  server.handleClient();
  setLEDState();
}
