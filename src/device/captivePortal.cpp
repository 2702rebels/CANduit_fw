/** File for the captive portal that uses Wifi to change the device number of the CANduit, created by Alex S, and modified for implementation by Alex E.
 */

#include "Arduino.h"
#include <WebServer.h>
#include <DNSServer.h>
#include "WiFi.h"
#include "freertos/task.h"
#include "device.h"

// ================== WIFI CONFIG ==================
const char* AP_SSID = "CANduit_Setup"; // 
const char* AP_PASS = "";  // open network

IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);

const byte DNS_PORT = 53;

// ================== GLOBAL OBJECTS ==================
WebServer server(80);
DNSServer dnsServer;

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
  <style>
  /* This CSS block defines styles for all buttons on the page */
  button {
    padding: 50px 76px;
    font-size: 45px;
    margin: 11px;
    border-radius: 2.5px;
    cursor: pointer;
  }
  input {
    padding: 50px 76px;
    font-size: 45px;
    margin: 11px;        
    border-radius: 2.5px; 
    cursor: pointer;
  }
</style>
</head>
<body>
  <h1 style='font-size: 100px;'>CANduit Control Panel</h1> 
)HTML";


  // ---- Current Device ID display ----
  page += "<p style='font-size: 75px;'><strong>Current Device ID:</strong> ";
  if (deviceID >= 0) {
    page += String(deviceID);
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

  if (deviceID >= 0) {
    Serial.print("[DEVICE ID] Current ID: ");
    Serial.println(deviceID);
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
  if (newID < 0 || newID > 63) {
    Serial.print("[DEVICE ID][ERROR] Invalid ID: ");
    Serial.println(newID);
    statusMessage = "Invalid Device ID (must be 0–63)";
    server.send(200, "text/html", buildHTMLPage());
    return;
  }

  // save to flash 
  setDeviceID(newID);

  Serial.print("[DEVICE ID] Saved: ");
  Serial.println(deviceID);

  statusMessage = "Device ID set successfully";

  server.send(200, "text/html", buildHTMLPage());
}

// ================== SETUP ==================

void captivePortalSetup() {
  Serial.println("===== Captive Portal Boot =====");

  // ---- LOAD STORED DEVICE ID ----
  deviceID = getDeviceID();

  if (deviceID >= 0) {
    Serial.print("[NVS] Stored Device ID: ");
    Serial.println(deviceID);
  } else {
    Serial.println("[NVS] No Device ID stored yet");
  }

  // ---- WIFI AP MODE ----
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, netMsk);


  // ---- Last 4 Digits For Unique SSID ---- code below was just to find the last 4 digtis of the MAC address of the CANduit

  Serial.print("ESP32 Soft AP MAC Address: ");
  
  String macAddress = (WiFi.softAPmacAddress()); // ssid + 0591 = name
  // grab last 2 .00 string var, take lst 4 chrcts
  Serial.print("SoftAP MAC Address: ");
  Serial.println(macAddress);


  String lastFourDigits = macAddress.substring(12);
  lastFourDigits.remove(2,1); // Removes 1 character starting at index 2 ('l')


  //Serial.print("Last 4 digits: "]);

  String AP_SSID_lastFourDigits = AP_SSID + lastFourDigits;

  Serial.println(AP_SSID_lastFourDigits);


  Serial.println("[WIFI] Starting AP...");
  bool apOK = WiFi.softAP(AP_SSID_lastFourDigits, AP_PASS);

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
  server.on("/type-input", handleUserIDInput);
  server.onNotFound(handleRoot);

  server.begin();
    

    TaskHandle_t portal_task;
    xTaskCreate(captivePortalLoop, "PWMTask",4096,NULL,0, &portal_task);

    Serial.println("Launched Captive portal event thread");
    // We never need to close the thread as we want it to stay open as long as the canduit runs
}

// ================== LOOP ==================

void captivePortalLoop(void *pvParameters) {
    while (1) {
      dnsServer.processNextRequest();
      server.handleClient();
      delay(100);
    }
}

