#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "notifications.h"
#include "secrets.h"

// SONOFF SV
const int RELAY_PIN = 12;
const int LED_PIN = 13;
const int SENSOR_PIN = 5;

// GENERIC
// const int RELAY_PIN = 4;
// const int LED_PIN = 2;
// const int SENSOR_PIN = 5;

ESP8266WebServer server(80);

void setRelayState(int nextState) {
  digitalWrite(RELAY_PIN, nextState);
}

void cycleRelay() {
  setRelayState(HIGH);
  delay(100);
  setRelayState(LOW);
}

int getState() {
  return digitalRead(SENSOR_PIN);
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

void watchInputState() {
  if (isClosed()) {
    digitalWrite(LED_PIN, LOW);
  } else {
    digitalWrite(LED_PIN, HIGH);
  }
}

void setupHardware() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

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
  watchInputState();

  if (strlen(NOTIFICATION_URL)) {
    listenForStateChange(&getState);
  }
}
