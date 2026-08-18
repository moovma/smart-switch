#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

// ============================================================
// PINS
// ============================================================

#define TRIAC_PIN 2
#define LED_PIN   3

// ============================================================
// WIFI
// ============================================================

const char* AP_SSID = "Moovma-SmartSwitch";
const char* AP_PASSWORD = "12345678";

WebServer server(80);
Preferences preferences;

// ============================================================
// STATE
// ============================================================

bool loadState = false;

String savedSSID;
String savedPassword;

// ============================================================
// LED / TRIAC
// ============================================================

void setLoad(bool state)
{
  loadState = state;

  digitalWrite(TRIAC_PIN, state ? HIGH : LOW);
  digitalWrite(LED_PIN, state ? HIGH : LOW);
}

// ============================================================
// HTML - PROVISIONING PAGE
// ============================================================

String provisioningPage()
{
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Moovma Smart Switch</title>

  <style>
    body {
      font-family: Arial, sans-serif;
      background: #f2f2f2;
      margin: 0;
      padding: 30px;
      text-align: center;
    }

    .card {
      max-width: 420px;
      margin: auto;
      background: white;
      padding: 30px;
      border-radius: 16px;
      box-shadow: 0 4px 20px rgba(0,0,0,0.1);
    }

    h1 {
      margin-bottom: 10px;
    }

    input {
      width: 90%;
      padding: 12px;
      margin: 10px 0;
      border: 1px solid #ccc;
      border-radius: 8px;
    }

    button {
      width: 95%;
      padding: 14px;
      margin-top: 15px;
      border: none;
      border-radius: 8px;
      background: #1565c0;
      color: white;
      font-size: 16px;
    }
  </style>
</head>

<body>

<div class="card">

  <h1>Moovma Smart Switch</h1>

  <p>Connect your switch to your home Wi-Fi network.</p>

  <form action="/save" method="POST">

    <input
      type="text"
      name="ssid"
      placeholder="Wi-Fi Name (SSID)"
      required
    >

    <input
      type="password"
      name="password"
      placeholder="Wi-Fi Password"
    >

    <button type="submit">
      Connect
    </button>

  </form>

</div>

</body>
</html>
)rawliteral";

  return html;
}

// ============================================================
// HTML - CONTROL PAGE
// ============================================================

String controlPage()
{
  String stateText = loadState ? "ON" : "OFF";

  String html = R"rawliteral(
<!DOCTYPE html>
<html>

<head>

  <meta name="viewport" content="width=device-width, initial-scale=1">

  <title>Moovma Smart Switch</title>

  <style>

    body {
      font-family: Arial, sans-serif;
      background: #f2f2f2;
      margin: 0;
      padding: 30px;
      text-align: center;
    }

    .card {
      max-width: 420px;
      margin: auto;
      background: white;
      padding: 30px;
      border-radius: 16px;
      box-shadow: 0 4px 20px rgba(0,0,0,0.1);
    }

    h1 {
      margin-bottom: 5px;
    }

    .state {
      font-size: 32px;
      margin: 30px 0;
      font-weight: bold;
    }

    .button {
      display: block;
      width: 90%;
      margin: 15px auto;
      padding: 18px;
      border-radius: 10px;
      color: white;
      text-decoration: none;
      font-size: 20px;
    }

    .on {
      background: #2e7d32;
    }

    .off {
      background: #c62828;
    }

  </style>

</head>

<body>

<div class="card">

  <h1>Moovma Smart Switch</h1>

  <div class="state">
    Load: <span id="state">STATE_PLACEHOLDER</span>
  </div>

  <a class="button on" href="/on">
    TURN ON
  </a>

  <a class="button off" href="/off">
    TURN OFF
  </a>

</div>

</body>

</html>
)rawliteral";

  html.replace("STATE_PLACEHOLDER", stateText);

  return html;
}

// ============================================================
// CONFIGURATION MODE
// ============================================================

void startAccessPoint()
{
  Serial.println();
  Serial.println("=================================");
  Serial.println(" WIFI CONFIGURATION MODE");
  Serial.println("=================================");

  WiFi.mode(WIFI_AP);

  WiFi.softAP(AP_SSID, AP_PASSWORD);

  IPAddress ip = WiFi.softAPIP();

  Serial.print("AP SSID: ");
  Serial.println(AP_SSID);

  Serial.print("AP Password: ");
  Serial.println(AP_PASSWORD);

  Serial.print("Open: http://");
  Serial.println(ip);

  // Configuration page
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", provisioningPage());
  });

  // Save credentials
  server.on("/save", HTTP_POST, []() {

    if (!server.hasArg("ssid")) {
      server.send(400, "text/plain", "SSID missing");
      return;
    }

    savedSSID = server.arg("ssid");
    savedPassword = server.arg("password");

    // Save in flash
    preferences.begin("wifi", false);

    preferences.putString("ssid", savedSSID);
    preferences.putString("password", savedPassword);

    preferences.end();

    String response = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
</head>

<body style="font-family:Arial;text-align:center;padding:40px;">

<h1>Wi-Fi credentials saved</h1>

<p>The smart switch is connecting to your network.</p>

<p>You can close this page.</p>

</body>
</html>
)rawliteral";

    server.send(200, "text/html", response);

    delay(1500);

    WiFi.softAPdisconnect(true);

    connectToWiFi();
  });

  // Captive portal-like redirects
  server.onNotFound([]() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });

  server.begin();

  Serial.println("Configuration web server started.");
}

// ============================================================
// CONNECT TO HOME WIFI
// ============================================================

bool connectToWiFi()
{
  Serial.println();
  Serial.println("Connecting to home Wi-Fi...");

  WiFi.mode(WIFI_STA);

  WiFi.begin(savedSSID.c_str(), savedPassword.c_str());

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startTime < 20000)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println();
    Serial.println("=================================");
    Serial.println(" WIFI CONNECTED");
    Serial.println("=================================");

    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    Serial.print("Signal strength: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    startControlServer();

    return true;
  }

  Serial.println("Wi-Fi connection failed.");

  WiFi.disconnect(true);

  return false;
}

// ============================================================
// CONTROL SERVER
// ============================================================

void startControlServer()
{
  server.on("/", HTTP_GET, []() {

    server.send(
      200,
      "text/html",
      controlPage()
    );

  });

  // ON
  server.on("/on", HTTP_GET, []() {

    setLoad(true);

    server.sendHeader(
      "Location",
      "/",
      true
    );

    server.send(
      303,
      "text/plain",
      ""
    );

  });

  // OFF
  server.on("/off", HTTP_GET, []() {

    setLoad(false);

    server.sendHeader(
      "Location",
      "/",
      true
    );

    server.send(
      303,
      "text/plain",
      ""
    );

  });

  // JSON status
  server.on("/status", HTTP_GET, []() {

    String json = "{";

    json += "\"state\":";
    json += loadState ? "true" : "false";

    json += "}";

    server.send(
      200,
      "application/json",
      json
    );

  });

  server.onNotFound([]() {

    server.send(
      404,
      "text/plain",
      "Not found"
    );

  });

  server.begin();

  Serial.println("Control web server started.");
}

// ============================================================
// LOAD WIFI CREDENTIALS
// ============================================================

bool loadWiFiCredentials()
{
  preferences.begin("wifi", true);

  savedSSID =
    preferences.getString("ssid", "");

  savedPassword =
    preferences.getString("password", "");

  preferences.end();

  if (savedSSID.length() == 0)
  {
    return false;
  }

  return true;
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(115200);

  delay(1000);

  pinMode(TRIAC_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Always start with load OFF
  setLoad(false);

  Serial.println();
  Serial.println("=================================");
  Serial.println("     MOOVMA SMART SWITCH");
  Serial.println("=================================");

  // Try stored Wi-Fi credentials
  if (loadWiFiCredentials())
  {
    Serial.println("Stored Wi-Fi credentials found.");

    if (!connectToWiFi())
    {
      Serial.println("Starting configuration AP...");

      startAccessPoint();
    }
  }
  else
  {
    Serial.println("No Wi-Fi credentials found.");

    startAccessPoint();
  }
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
  server.handleClient();

  // If connected, keep monitoring Wi-Fi
  if (WiFi.getMode() == WIFI_STA)
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      static unsigned long lastReconnect = 0;

      if (millis() - lastReconnect > 10000)
      {
        lastReconnect = millis();

        Serial.println("Wi-Fi disconnected. Reconnecting...");

        WiFi.reconnect();
      }
    }
  }
}
