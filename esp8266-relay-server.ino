#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "secrets.h"
#include "notifications.h"

const String VERSION = "2";

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

void reply(int status, String body = "") {
  server.sendHeader("FirmwareVersion", VERSION);
  server.send(status, "text/plain", body);
}

void sendState(int state) {
  reply(200, String(state));
}

void handleHealth() {
  reply(204);
}

void handleState() {
  sendState(getCurrentState());
}

void handleOpen() {
  if (isClosed()) {
    cycleRelay();
    sendState(getTargetState());
  } else {
    sendState(getCurrentState());
  }
}

void handleClose() {
  if (isOpen()) {
    cycleRelay();
    sendState(getTargetState());
  } else {
    sendState(getCurrentState());
  }
}

void handleCycle() {
  cycleRelay();
  sendState(getTargetState());
}

void handleNotFound() {
  reply(404);
}

void configureRoutes() {
  server.on("/", handleHealth);
  server.on("/state", handleState);
  server.on("/open", handleOpen);
  server.on("/close", handleClose);
  server.on("/cycle", handleCycle);
  server.onNotFound(handleNotFound);
}

void awaitWifiConnected() {
  Serial.setDebugOutput(true);
  Serial.println("Trying to connect " + String(WIFI_SSID));

  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME);
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

  if (strlen(NOTIFICATION_URL)) {
    listenForStateChange(&getCurrentState);
  }
}
