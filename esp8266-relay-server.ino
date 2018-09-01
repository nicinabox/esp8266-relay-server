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
  setState(1);
  server.send(204);
}

void handleClose() {
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

  // Relay control pin.
  pinMode(ESP8266_GPIO4, OUTPUT);

  // Input pin.
  pinMode(ESP8266_GPIO5, INPUT_PULLUP);

  // ESP8266 module blue LED.
  pinMode(LED_PIN, OUTPUT);

  // Set relay control pin low.
  digitalWrite( ESP8266_GPIO4, 0 );
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