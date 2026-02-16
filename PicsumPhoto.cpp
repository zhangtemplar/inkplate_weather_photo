#include "PicsumPhoto.h"
#include "Inkplate.h"
#include "WiFiUtil.h"
#include "LocalPhoto.h"

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
    // picsum.photos returns a random JPEG image; drawImage follows redirects
    if (!display.drawImage("https://picsum.photos/1200/825", display.JPG, 0, 0, true)) {
        Serial.println(F("Failed to draw Picsum image, falling back to local photo"));
        LocalPhoto localPhoto;
        localPhoto.draw();
        return;
    }
    display.display();
}
