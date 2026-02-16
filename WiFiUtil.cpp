#include "WiFiUtil.h"

extern char SECRET_SSID[128];
extern char SECRET_PASS[128];

bool connectWiFi(int timeoutSeconds) {
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin(SECRET_SSID, SECRET_PASS);

    int cnt = 0;
    Serial.print(F("Waiting for WiFi to connect..."));
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(F("."));
        delay(1000);
        ++cnt;
        if (cnt >= timeoutSeconds) {
            Serial.println(F(" failed"));
            return false;
        }
    }
    Serial.println(F(" connected"));
    return true;
}
