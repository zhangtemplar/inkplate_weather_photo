#include "Flickr.h"

// Include Inkplate library to the sketch
#include "Inkplate.h"
#include <HTTPClient.h>
#include <WiFi.h>

extern char FLICKR_KEY[128];
extern char SECRET_SSID[128];
extern char SECRET_PASS[128];

#define WIDTH 1200
#define HEIGHT 825

char flickr_query_url[256];
char flickr_size_url[256];
char flickr_photo_url[256];

// for the device. This could not be a class variable or function argument
// otherwise it will cause stack overflow
extern Inkplate display;

Flickr::Flickr() {
    sprintf(flickr_query_url, "https://www.flickr.com/services/rest/?method=flickr.photos.search&api_key=%s&tags=BW&format=json&nojsoncallback=1&per_page=1&page=10", FLICKR_KEY);
}

void Flickr::draw() {
    // use 3 bit for image
    display.setDisplayMode(INKPLATE_3BIT);
    // Initiating wifi, like in BasicHttpClient example
    WiFi.mode(WIFI_STA);
    WiFi.begin(SECRET_SSID, SECRET_PASS);

    int cnt = 0;
    Serial.print(F("Waiting for WiFi to connect..."));
    while ((WiFi.status() != WL_CONNECTED))
    {
        Serial.print(F("."));
        delay(1000);
        ++cnt;

        if (cnt == 20)
        {
            Serial.println(F("Can't connect to WIFI, restarting"));
            delay(100);
            ESP.restart();
        }
    }
    Serial.println(F(" connected"));

    // Wake wifi module and save initial state
    bool sleep = WiFi.getSleep();
    WiFi.setSleep(false);
    int trial = 0;
    while (!list() || !querySize()) {
        trial += 1;
        if (trial >= 10) {
            Serial.println(F("failed to get images from flickr"));
            return;
        }
        Serial.println(F("failed to get images from flickr"));
        display.println(F("failed to get images from flickr"));
    }
    Serial.print(F("to display image from flickr "));
    Serial.println(flickr_photo_url);
    // apply dither
    Serial.println(display.drawImage(F("https://live.staticflickr.com/8304/7922047866_ccd2886b40_c_d.jpg"), 0, 0, 1));
    display.display();
    delay(100);
}

bool Flickr::list() {
    // Http object
    HTTPClient http;
    http.getStream().setNoDelay(true);
    http.getStream().setTimeout(1);
    http.begin(flickr_query_url);
    Serial.println(flickr_query_url);

    // Do get request
    int httpCode = http.GET();
    if (httpCode == 200) { // 200: http success
        int32_t len = http.getSize();

        if (len > 0) {
            // Try to parse JSON object
            DeserializationError error = deserializeJson(doc, http.getStream());

            // Print error to Serial monitor if one exsists
            if (error) {
                Serial.print(F("deserializeJson() failed for listing flickr photos: "));
                Serial.println(error.c_str());
            } else
            {
                // Empty list means no matches
                if (doc.size() == 0) {
                    Serial.println(F("Failed to find any photos"));
                } else
                {
                    Serial.print(F("pick photo id "));
                    Serial.println(doc["photos"]["photo"][0]["id"].as<const char*>());
                    sprintf(flickr_size_url, "https://www.flickr.com/services/rest/?method=flickr.photos.getSizes&api_key=%s&photo_id=%s&format=json&nojsoncallback=1", FLICKR_KEY, doc["photos"]["photo"][0]["id"].as<const char*>());
                    return true;
                }
            }
        } else {
            Serial.println(F("empty response when listing photo from flickr"));
        }
    } else {
        Serial.println(F("error code when listing photo from flickr"));
    }
    http.end();
    return false;
}

bool Flickr::querySize() {
    // Http object
    HTTPClient http;
    http.getStream().setNoDelay(true);
    http.getStream().setTimeout(1);
    http.begin(flickr_size_url);
    Serial.println(flickr_size_url);

    // Do get request
    int httpCode = http.GET();
    if (httpCode == 200) { // 200: http success
        int32_t len = http.getSize();

        if (len > 0) {
            // Try to parse JSON object
            DeserializationError error = deserializeJson(doc, http.getStream());

            // Print error to Serial monitor if one exsists
            if (error) {
                Serial.print(F("deserializeJson() failed for getting size for flickr photo: "));
                Serial.println(error.c_str());
            } else {
                // Empty list means no matches
                if (doc["sizes"]["candownload"].as<int>() != 1) {
                    Serial.println("photo is not allowed to download");
                    return false;
                }
                // find the best size
                int numberSizes = doc["sizes"]["size"].size();
                if (numberSizes < 1) {
                    Serial.println(F("Failed to find any sizes"));
                    return false;
                }
                Serial.print(F("find size "));
                Serial.println(numberSizes);
                int i = 0;
                for (i = 0; i < numberSizes; i++) {
                    if (doc["sizes"]["size"][i]["width"].as<int>() >= WIDTH || doc["sizes"]["size"][i]["height"].as<int>() >= HEIGHT) {
                        break;
                    }
                }
                // use the largest size that is smaller than targeted resolution
                if (i > 0) {
                    i -= 1;
                }
                Serial.print(F("optimal size is found "));
                Serial.println(i);
                Serial.println(doc["sizes"]["size"][i]["source"].as<const char*>());
                strcpy(flickr_photo_url, doc["sizes"]["size"][i]["source"].as<const char*>());
                return true;
            }
        } else {
            Serial.println(F("empty response when getting size from flickr"));
        }
    } else {
        Serial.println(F("error code when getting size from flickr"));
    }
    return false;
}