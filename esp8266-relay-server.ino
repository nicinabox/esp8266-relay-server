#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "secrets.h"

// SONOFF SV
const int RELAY_PIN = 12;
const int LED_PIN = 13;
const int SENSOR_CLOSED_PIN = 5;

const int GATE_OPEN = 0;
const int GATE_CLOSED = 1;

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
  // 1 - switch open
  // 0 - switch closed
  return digitalRead(SENSOR_CLOSED_PIN);
}

int getCurrentState() {
  return 1 - getSensorState();
}

int getTargetState() {
  return getSensorState();
}

bool isClosed() {
  return getCurrentState() == GATE_CLOSED;
}

bool isOpen() {
  return getCurrentState() == GATE_OPEN;
}

int setLEDState() {
  digitalWrite(LED_PIN, getSensorState());
}

void sendStatus(int status) {
  server.send(200, "text/plain", String(status));
}

void handleStatus() {
  sendStatus(getCurrentState());
}

void handleOpen() {
  if (isClosed()) {
    cycleRelay();
    sendStatus(getTargetState());
  } else {
    sendStatus(getCurrentState());
  }
}

void handleClose() {
  if (isOpen()) {
    cycleRelay();
    sendStatus(getTargetState());
  } else {
    sendStatus(getCurrentState());
  }
}

void handleCycle() {
  cycleRelay();
  sendStatus(getTargetState());
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

  pinMode(SENSOR_CLOSED_PIN, INPUT_PULLUP);
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
