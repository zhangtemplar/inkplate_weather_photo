#ifndef PICSUM_PHOTO_H
#define PICSUM_PHOTO_H

class PicsumPhoto {
public:
    // Fetch a random photo from Lorem Picsum and display it.
    // Falls back to local photo on failure.
    void draw();
};

#endif
