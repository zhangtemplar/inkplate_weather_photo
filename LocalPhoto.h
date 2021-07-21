#ifndef LOCAL_PHOTO_H
#define LOCAL_PHOTO_H

/**
 * @brief class for displaying a random photo from sdcard
 * 
 */
class LocalPhoto {
public:
    /**
     * @brief main function
     * 
     */
    void draw();
private:
    /**
     * @brief get number of files in the directory
     * 
     * @return int 
     */
    int numberOfFiles();
};
#endif