#ifndef CAPTIVE_PORTAL_H
#define CAPTIVE_PORTAL_H

#include <WebServer.h>
#include <DNSServer.h>

class CaptivePortal {
public:
    // Start the captive portal. Blocks until user saves config or timeout (5 min).
    void start();
private:
    WebServer *server;
    DNSServer *dnsServer;

    void handleRoot();
    void handleSave();
    void handleUploadPage();
    void handleUpload();
    void handleUploadPost();
    void handleImageList();
    void handleDelete();
    void handleNotFound();
    void showSetupScreen();

    SdFile uploadFile;
    bool uploadInProgress;
    int nextFileIndex();
};

#endif
