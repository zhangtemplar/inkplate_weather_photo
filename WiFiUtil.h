#ifndef WIFI_UTIL_H
#define WIFI_UTIL_H

#include <WiFi.h>

// Shared WiFi connection utility.
// Returns true if connected, false if failed after retries.
bool connectWiFi(int timeoutSeconds = 20);

#endif
