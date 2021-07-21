#include "LocalPhoto.h"

// Include Inkplate library to the sketch
#include "Inkplate.h"
#include "SdFat.h"               //Include library for SD card
SdFile file;                     // Create SdFile object used for accessing files on SD card
char sdFileName[128];
// for the device. This could not be a class variable or function argument
// otherwise it will cause stack overflow
extern Inkplate display;

void LocalPhoto::draw() {
    // use 3 bit for image
    display.setDisplayMode(INKPLATE_3BIT);
    if (!display.sdCardInit()) {
      display.println(F("SD Card error!"));
      Serial.println(F("SD Card error!"));
      display.partialUpdate();
      return;
    }
    int numberFile = numberOfFiles();
    if (numberFile < 1) {
        Serial.println(F("No valid image is found! Please put image as eink/0.png, eink/1.png"));
        return;
    }
    Serial.print(F("SD Card OK! Reading image..."));
    Serial.println(numberFile);
    int sdFileIndex = random(numberFile);
    sprintf(sdFileName, "eink/%d.png", sdFileIndex);
    if (!display.drawImage(sdFileName, display.PNG, 0, 0)) {
      // unable open the image
      Serial.print(F("failed to display image "));
      Serial.println(sdFileName);
    } else {
      Serial.print(F("to display image "));
      Serial.println(sdFileName);
    }
    display.display();
}

int LocalPhoto::numberOfFiles() {
    int start = 0;
    // guess there will be 1000 files
    int end = 1000;
    while (end>start) {
        sprintf(sdFileName, "eink/%d.png", end);
        if (file.open(sdFileName, O_RDONLY)) {
            // file exists, try next files
            start = end;
            end *= 2;
        } else {
            // file doesn't exit, try previous file
            end = (start + end) / 2;
        }
    }
    return end + 1;
}