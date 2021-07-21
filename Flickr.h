#ifndef FLICKR_H
#define FLICKR_H

#include <ArduinoJson.h>

/**
 * @brief this class queries the images from flickr and select proper image to show
 * 
 */
class Flickr {
public:
    /**
     * @brief main function
     * 
     */
    void draw();
    Flickr();
private:
    // Static Json from ArduinoJson library
    StaticJsonDocument<6000> doc;
    /**
     * @brief list the images
     */
    bool list();
    /**
     * @brief select the optimal size
     * 
     */
    bool querySize();
};
#endif