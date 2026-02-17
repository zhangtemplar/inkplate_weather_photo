#include "PicsumPhoto.h"
#include "Inkplate.h"
#include "WiFiUtil.h"
#include "LocalPhoto.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "SdFat.h"

extern Inkplate display;

void PicsumPhoto::draw() {
    display.setDisplayMode(INKPLATE_3BIT);

    if (!connectWiFi(20)) {
        Serial.println(F("WiFi failed for Picsum, falling back to local photo"));
        LocalPhoto localPhoto;
        localPhoto.draw();
        return;
    }

    Serial.println(F("Fetching image from picsum.photos"));

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.begin(client, "https://picsum.photos/1200/825?grayscale");

    int httpCode = http.GET();
    if (httpCode != 200) {
        Serial.print(F("Picsum HTTP error: "));
        Serial.println(httpCode);
        http.end();
        LocalPhoto localPhoto;
        localPhoto.draw();
        return;
    }

    // Save to SD card, then display from file
    if (!display.sdCardInit()) {
        Serial.println(F("SD card init failed for picsum"));
        http.end();
        LocalPhoto localPhoto;
        localPhoto.draw();
        return;
    }

    SdFile file;
    if (!file.open("picsum_temp.jpg", O_WRONLY | O_CREAT | O_TRUNC)) {
        Serial.println(F("Failed to create temp file"));
        http.end();
        LocalPhoto localPhoto;
        localPhoto.draw();
        return;
    }

    WiFiClient *stream = http.getStreamPtr();
    int len = http.getSize();
    uint8_t buf[512];
    int totalRead = 0;
    while (http.connected() && (len > 0 || len == -1)) {
        size_t avail = stream->available();
        if (avail) {
            int toRead = (int)avail;
            if (toRead > (int)sizeof(buf)) toRead = sizeof(buf);
            int c = stream->readBytes(buf, toRead);
            file.write(buf, c);
            totalRead += c;
            if (len > 0) len -= c;
        } else {
            delay(1);
        }
    }
    file.close();
    http.end();

    Serial.print(F("Downloaded picsum image: "));
    Serial.print(totalRead);
    Serial.println(F(" bytes"));

    if (totalRead == 0 || !display.drawImage("picsum_temp.jpg", 0, 0, true, false)) {
        Serial.println(F("Failed to draw Picsum image, falling back to local photo"));
        LocalPhoto localPhoto;
        localPhoto.draw();
        return;
    }
    display.display();
}
