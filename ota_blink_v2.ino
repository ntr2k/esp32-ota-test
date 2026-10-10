/*
  ESP32 OTA TEST - VERSION 2 (fast blink, 200 ms)
  -------------------------------------------------
  This version is NEVER flashed by USB. Only its compiled .bin is uploaded to GitHub.
  It blinks the built-in LED fast and checks GitHub for a newer version.

  Only 2 lines differ from v2:  FW_VERSION and BLINK_DELAY.

  How the update works:
    1. ESP32 downloads version.txt from GitHub (a file containing one number).
    2. If that number is HIGHER than FW_VERSION below, it downloads firmware.bin
       from GitHub, flashes itself and reboots.

  Replace before use:  USERNAME, YOUR_WIFI_NAME, YOUR_WIFI_PASSWORD
  Do NOT upload this .ino with real passwords to a public repo.
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>

const char* ssid     = "YOUR_WIFI_NAME";       // 2.4 GHz WiFi only
const char* password = "YOUR_WIFI_PASSWORD";

const int FW_VERSION  = 2;      // <<< CHANGE per version (1, 2, 3 ...)
const int BLINK_DELAY = 200;    // <<< CHANGE per version (ms)

const char* VERSION_URL = "https://raw.githubusercontent.com/USERNAME/esp32-ota-test/main/version.txt";
const char* BIN_URL     = "https://raw.githubusercontent.com/USERNAME/esp32-ota-test/main/firmware.bin";

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

unsigned long lastCheck = 0;

void checkForUpdate() {
  int serverVersion = 0;
  {
    WiFiClientSecure c;
    c.setInsecure();   // TEST ONLY: skips certificate check
    HTTPClient http;
    http.begin(c, VERSION_URL);
    int code = http.GET();
    if (code == 200) serverVersion = http.getString().toInt();
    Serial.printf("Check: HTTP %d, server=%d, mine=%d\n", code, serverVersion, FW_VERSION);
    http.end();
  }

  if (serverVersion > FW_VERSION) {
    Serial.println("Updating... (do not unplug)");
    WiFiClientSecure c2;
    c2.setInsecure();  // TEST ONLY
    httpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    t_httpUpdate_return ret = httpUpdate.update(c2, BIN_URL);
    if (ret == HTTP_UPDATE_FAILED) Serial.println(httpUpdate.getLastErrorString());
    // on success the board reboots by itself
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.printf("\nFirmware v%d, blink %d ms\n", FW_VERSION, BLINK_DELAY);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  Serial.println("WiFi connected");
  checkForUpdate();   // check once right at boot
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH); delay(BLINK_DELAY);
  digitalWrite(LED_BUILTIN, LOW);  delay(BLINK_DELAY);

  if (millis() - lastCheck > 30000UL) {   // every 30 s (test value)
    lastCheck = millis();
    checkForUpdate();
  }
}
