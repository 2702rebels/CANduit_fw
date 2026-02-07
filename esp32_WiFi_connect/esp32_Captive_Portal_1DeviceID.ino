// ================== INCLUDES ==================
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
// above 3 are needed for the webpage + captive portal

#include <Preferences.h>
// used for non-volatile storage (survives power loss)

// ================== NVS ==================
Preferences preferences;

// ================== LED CONFIG ==================
// onboard LED is ALWAYS GPIO 2 on this Nano ESP32
#define STATUS_LED 2

// ================== WIFI CONFIG ==================
const char* AP_SSID = "CANduit_Setup";
const char* AP_PASS = "";  // open network

IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

const byte DNS_PORT = 53;

// ================== GLOBAL OBJECTS ==================
WebServer server(80);
DNSServer dnsServer;

// ================== DEVICE STATE ==================
// lives in RAM, loaded from flash on boot, this just a placeholder, by the time of multiple
// testing, it allready has it's own device ID unless we hard rest it, but we would want to 
// check if the device id = -1 somehwere assuming after a power rest or something happen
// then we would prompt them to give a device ID before anything because well -1 isn't really
// ideal for value between 0-31
int currentDeviceID = -1;


// used to show status text on webpage
String statusMessage = "";

// ================== HTML PAGE ==================
// dynamically built so we can inject device ID + status text

String buildHTMLPage() {
  String page = R"HTML(
<!DOCTYPE html>
<html>
<head>
  <title>CANduit Setup</title>
</head>
<body>
  <h1>CANduit Control Panel</h1>
)HTML";

  // ---- Current Device ID display ----
  page += "<p><strong>Current Device ID:</strong> ";
  if (currentDeviceID >= 0) {
    page += String(currentDeviceID);
  } else {
    page += "Not set";
  }
  page += "</p>";

  // ---- Status message (set success / error) ----
  if (statusMessage.length() > 0) {
    page += "<p><em>" + statusMessage + "</em></p>";
  }

  // ---- Buttons ----
  page += R"HTML(
  <form action="/serial">
    <button type="submit">Print to Serial</button>
  </form>

  <form action="/flash">
    <button type="submit">Flash LED</button>
  </form>

  <form action="/original">
    <button type="submit">Original Button</button>
  </form>

  <form action="/type-input" method="GET">
    <input type="number" name="userSetDeviceID" min="0" max="31"
           placeholder="Device ID (0–31)">
    <button type="submit">Set Device ID</button>
  </form>

</body>
</html>
)HTML";

  return page;
}

// ================== HTTP HANDLERS ==================

void handleRoot() {
  Serial.println("[HTTP] Homepage served");
  server.send(200, "text/html", buildHTMLPage());
  
  /*statusCode: 200 = "OK"

- Standard HTTP code meaning the request succeeded.

- Other codes exist: 404 = Not Found, 500 = Server Error, etc.*/
}

void handleSerialOnly() {
  Serial.println("[BUTTON] Serial-only button pressed");

  if (currentDeviceID >= 0) {
    Serial.print("[DEVICE ID] Current ID: ");
    Serial.println(currentDeviceID);
  } else {
    Serial.println("[DEVICE ID] Not set");
  }

  server.send(200, "text/html", buildHTMLPage());
}

void handleFlashLED() {
  Serial.println("[BUTTON] Flash LED");

  // also print device ID (requested)
  if (currentDeviceID >= 0) {
    Serial.print("[DEVICE ID] Current ID: ");
    Serial.println(currentDeviceID);
  } else {
    Serial.println("[DEVICE ID] Not set");
  }

  for (int i = 0; i < 10; i++) {
    digitalWrite(STATUS_LED, HIGH);
    delay(100 + i * 80);
    digitalWrite(STATUS_LED, LOW);
    delay(50 + i * 30);
  }

  server.send(200, "text/html", buildHTMLPage());
}

void handleOriginal() {
  Serial.println("[BUTTON] Original button pressed");

  // button 3 ALSO prints device ID
  if (currentDeviceID >= 0) {
    Serial.print("[DEVICE ID] Current ID: ");
    Serial.println(currentDeviceID);
  } else {
    Serial.println("[DEVICE ID] Not set");
  }

  server.send(200, "text/html", buildHTMLPage());
}

// ================== DEVICE ID HANDLER ==================

void handleUserIDInput() {
  Serial.println("[DEVICE ID] Submission received");

  statusMessage = "";  // clear old messages

  if (!server.hasArg("userSetDeviceID")) {
    statusMessage = "No Device ID provided";
    server.send(200, "text/html", buildHTMLPage());
    return;
  }

  int newID = server.arg("userSetDeviceID").toInt();

  // HARD RULE: 6-bit value only
  if (newID < 0 || newID > 31) {
    Serial.print("[DEVICE ID][ERROR] Invalid ID: ");
    Serial.println(newID);
    statusMessage = "Invalid Device ID (must be 0–31)";
    server.send(200, "text/html", buildHTMLPage());
    return;
  }

  // save to flash
  preferences.begin("canduit", false);
  preferences.putInt("device_id", newID);
  preferences.end();

  currentDeviceID = newID;

  Serial.print("[DEVICE ID] Saved: ");
  Serial.println(currentDeviceID);

  statusMessage = "Device ID set successfully";

  server.send(200, "text/html", buildHTMLPage());
}

// ================== SETUP ==================

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("===== CANduit Boot =====");

  // ---- LOAD STORED DEVICE ID ----
  preferences.begin("canduit", true);
  currentDeviceID = preferences.getInt("device_id", -1);
  preferences.end();

  if (currentDeviceID >= 0) {
    Serial.print("[NVS] Stored Device ID: ");
    Serial.println(currentDeviceID);
  } else {
    Serial.println("[NVS] No Device ID stored yet");
  }

  // ---- LED INIT ----
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW);

  // ---- WIFI AP MODE ----
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, netMsk);

  Serial.println("[WIFI] Starting AP...");
  bool apOK = WiFi.softAP(AP_SSID, AP_PASS);

  if (!apOK) {
    Serial.println("[WIFI][ERROR] AP failed to start");
    while (true) delay(1000);
  }

  Serial.print("[WIFI] AP IP: ");
  Serial.println(WiFi.softAPIP());

  // ---- DNS (CAPTIVE PORTAL) ----
  dnsServer.start(DNS_PORT, "*", apIP);
  Serial.println("[DNS] Captive portal active");

  // ---- ROUTES ----
  server.on("/", handleRoot);
  server.on("/serial", handleSerialOnly);
  server.on("/flash", handleFlashLED);
  server.on("/original", handleOriginal);
  server.on("/type-input", handleUserIDInput);
  server.onNotFound(handleRoot);

  server.begin();
  Serial.println("[HTTP] Server started");
  Serial.println("[SYSTEM] Setup complete");
}

// ================== LOOP ==================

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}


