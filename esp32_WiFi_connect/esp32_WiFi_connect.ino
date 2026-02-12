#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

/*
  ============================================================
  CANduit – ESP32-WROOM-32 Dev Board
  Board type: ESP32-WROOM-32 (generic, USB-C, CP2102)
  
  IMPORTANT LED NOTE (DO NOT FORGET THIS AGAIN):
  - One LED is POWER ONLY (always on, NOT controllable)
  - The OTHER LED is user-controllable
  - That controllable LED is ALMOST ALWAYS GPIO 2
  ============================================================
*/

// ================== LED CONFIG ==================

// This is the onboard "user" LED (blue on most ESP32-WROOM boards)
#define STATUS_LED 2   // <-- THIS is the fix

// ================== WIFI CONFIG ==================

const char* AP_SSID = "CANduit_Setup";
const char* AP_PASS = "";   // open network (setup mode)

IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

const byte DNS_PORT = 53;

// ================== GLOBAL OBJECTS ==================

// Web server on port 80 (normal HTTP)
WebServer server(80);

// DNS server for captive portal behavior
DNSServer dnsServer;

// ================== HTML PAGE ==================
// yes it's inline for now
// later we can move this to SPIFFS / LittleFS

const char* htmlPage = R"HTML(
<!DOCTYPE html>
<html>
<head>
  <title>CANduit Setup</title>
</head>
<body>
  <h1>CANduit Control Panel</h1>

  <!-- Button 1: ONLY prints to Serial -->
  <form action="/serial">
    <button type="submit">Print to Serial</button>
  </form>

  <!-- Button 2: Flash the onboard LED -->
  <form action="/flash">
    <button type="submit">Flash LED</button>
  </form>

  <!-- Button 3: Placeholder / original logic -->
  <form action="/original">
    <button type="submit">Original Button</button>
  </form>

</body>
</html>
)HTML";

// ================== HTTP HANDLERS ==================

void handleRoot() {
  Serial.println("[HTTP] Homepage served");
  server.send(200, "text/html", htmlPage);
}

void handleSerialOnly() {
  Serial.println("[BUTTON] Serial-only button pressed");
  // stays on same page
  server.send(200, "text/html", htmlPage);
}

void handleFlashLED() {
  Serial.println("[BUTTON] Flash LED");

  /*
    VERY IMPORTANT:
    This board does NOT have:
    - RGB_BUILTIN
    - rgbLedWrite
    - NeoPixel anything

    It ONLY has a simple GPIO LED.
    So we do normal digitalWrite blinking.
  */

  for (int i = 0; i < 3; i++) {
    digitalWrite(STATUS_LED, HIGH); // LED ON
    delay(200);
    digitalWrite(STATUS_LED, LOW);  // LED OFF
    delay(200);
  }

  server.send(200, "text/html", htmlPage);
}

void handleOriginal() {
  Serial.println("[BUTTON] Original button pressed");
  server.send(200, "text/html", htmlPage);
}

// ================== SETUP ==================

void setup() {
  Serial.begin(115200);
  delay(1500); // give serial monitor time to attach

  // ---- LED INIT ----
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW); // LED off at boot (clean start)

  Serial.println();
  Serial.println("===== CANduit Boot =====");
  Serial.println("[BOOT] Serial OK");

  // ---- WIFI ACCESS POINT MODE ----
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, netMsk);

  Serial.println("[WIFI] Starting AP...");
  bool apOK = WiFi.softAP(AP_SSID, AP_PASS);

  if (!apOK) {
    Serial.println("[WIFI][ERROR] AP failed to start");
    // hard stop if WiFi fails
    while (true) delay(1000);
  }

  Serial.println("[WIFI] AP started");
  Serial.print("[WIFI] AP IP: ");
  Serial.println(WiFi.softAPIP());

  // ---- DNS (CAPTIVE PORTAL) ----
  dnsServer.start(DNS_PORT, "*", apIP);
  Serial.println("[DNS] Captive portal active");

  // ---- HTTP ROUTES ----
  server.on("/", handleRoot);

  server.on("/serial", handleSerialOnly);
  server.on("/flash", handleFlashLED);
  server.on("/original", handleOriginal);

  // any unknown URL -> homepage
  server.onNotFound(handleRoot);

  server.begin();
  Serial.println("[HTTP] Server started");
  Serial.println("[SYSTEM] Setup complete");
}

// ================== LOOP ==================

void loop() {
  // keep DNS redirect working
  dnsServer.processNextRequest();

  // handle incoming HTTP requests
  server.handleClient();
}
