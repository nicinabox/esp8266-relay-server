#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Esp8266 pinouts
#define ESP8266_GPIO2   2  // Blue LED.
#define ESP8266_GPIO4   4  // Relay control.
#define ESP8266_GPIO5   5  // Optocoupler input.
#define LED_PIN         ESP8266_GPIO2

const int PORT = 80;
const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASSWORD";

// 0 - open
// 1 - closed
volatile int state = 0;

ESP8266WebServer server(PORT);

void setState(int nextState) {
  state = nextState;
  digitalWrite(ESP8266_GPIO4, state);
}

void handleRoot() {
  String payload = "{ \"state\": " + String(state) + " }";
  server.send(200, "text/json", payload);
}

void handleOpen() {
  setState(0);
  server.send(204);
}

void handleClose() {
  setState(1);
  server.send(204);
}

void handleCycle() {
  setState(1);
  delay(100);
  setState(0);
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
  digitalWrite(ESP8266_GPIO4, state);
}

void setup() {
  awaitWifiConnected();
  setupRelay();
  configureRoutes();

  server.begin();
  Serial.println("Listening on http://" + String(WiFi.localIP()) + ":" + String(PORT));
}

void loop() {
  server.handleClient();
}
