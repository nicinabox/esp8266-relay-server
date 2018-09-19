#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "notifications.h"
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

int getState() {
  return digitalRead(SENSOR_PIN);
}

bool isClosed() {
  return getState() == LOW;
}

bool isOpen() {
  return getState() == HIGH;
}

void sendStatus(int status) {
  server.send(200, "text/plain", String(status));
}

void handleStatus() {
  sendStatus(getState());
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

void setInputLED() {
  digitalWrite(LED_PIN, getState());
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
  setInputLED();

  if (strlen(NOTIFICATION_URL)) {
    listenForStateChange(&getState);
  }
}
