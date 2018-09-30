#include <ESP8266HTTPClient.h>
#include "secrets.h"

HTTPClient http;

int const INTERVAL = 1000;
long lastStateCheck = 0;
volatile int lastGateState = HIGH;

void onStateChange(int gateState) {
  // To be used with homebridge-http-notification-server
  // https://github.com/Supereg/homebridge-http-notification-server
  String body = "{ \
    \"characteristic\": \"TargetDoorState\", \
    \"value\": " + String(gateState) + "}";

  http.begin(NOTIFICATION_URL);
  http.addHeader("Content-Type", "application/json");
  http.POST(body);
  http.end();
}

bool didStateChange(int gateState) {
  return lastGateState != gateState;
}

void listenForStateChange(int (*getState)()) {
  unsigned long currentStateCheck = millis();

  if (currentStateCheck - lastStateCheck > INTERVAL) {
    lastStateCheck = currentStateCheck;
    int gateState = getState();

    if (didStateChange(gateState)) {
      lastGateState = gateState;
      onStateChange(gateState);
    }
  }
}
