#include "CaptivePortal.h"
#include "Inkplate.h"
#include "SdFat.h"
#include <WiFi.h>
#include <ArduinoJson.h>

#include "Fonts/FreeSerifBold18pt7b.h"
#include "Fonts/FreeSerifBold12pt7b.h"

extern Inkplate display;
extern char SECRET_SSID[128];
extern char SECRET_PASS[128];
extern int SECRET_TIMEZONE;
extern char SECRET_CITY[128];
extern char WEATHER_API_KEY[128];
extern char CALENDARIFIC_KEY[128];

// HTML for the config page
static const char CONFIG_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<title>Inkplate Setup</title>
<style>
body{font-family:sans-serif;margin:20px;background:#f5f5f5}
h1{color:#333}
.tab{display:inline-block;padding:10px 20px;cursor:pointer;background:#ddd;border-radius:5px 5px 0 0}
.tab.active{background:#fff;font-weight:bold}
.panel{display:none;background:#fff;padding:20px;border-radius:0 5px 5px 5px}
.panel.active{display:block}
label{display:block;margin:10px 0 5px;font-weight:bold}
input[type=text],input[type=password],input[type=number]{width:100%;padding:8px;box-sizing:border-box}
button{background:#4CAF50;color:white;padding:12px 24px;border:none;cursor:pointer;margin:10px 5px;border-radius:4px;font-size:16px}
button.del{background:#f44336}
.msg{padding:10px;background:#dff0d8;border-radius:4px;margin:10px 0}
</style>
<script>
function showTab(n){
  document.querySelectorAll('.tab').forEach(t=>t.classList.remove('active'));
  document.querySelectorAll('.panel').forEach(p=>p.classList.remove('active'));
  document.querySelectorAll('.tab')[n].classList.add('active');
  document.querySelectorAll('.panel')[n].classList.add('active');
}
</script>
</head><body>
<h1>Inkplate Setup</h1>
<div class='tab active' onclick='showTab(0)'>WiFi Config</div>
<div class='tab' onclick='showTab(1)'>Image Upload</div>

<div class='panel active'>
<form action='/save' method='POST'>
<label>WiFi SSID</label><input type='text' name='ssid' value='%SSID%'>
<label>WiFi Password</label><input type='password' name='pass'>
<label>City (lat=xx&lon=yy)</label><input type='text' name='city' value='%CITY%'>
<label>Timezone (seconds offset)</label><input type='number' name='tz' value='%TZ%'>
<label>OpenWeather API Key</label><input type='text' name='weather_key' value='%WKEY%'>
<label>Calendarific API Key</label><input type='text' name='cal_key' value='%CKEY%'>
<button type='submit'>Save & Restart</button>
</form>
</div>

<div class='panel'>
<h2>Upload Image</h2>
<p>Upload PNG images for the photo frame (1200x825 recommended).</p>
<form action='/upload' method='POST' enctype='multipart/form-data'>
<input type='file' name='image' accept='.png'>
<button type='submit'>Upload</button>
</form>
<h2>Current Images</h2>
<div id='imglist'>Loading...</div>
<script>
fetch('/images').then(r=>r.json()).then(d=>{
  let h='<p>'+d.count+' images on SD card</p>';
  for(let i=0;i<d.count;i++){
    h+='<form action="/delete" method="POST" style="display:inline">';
    h+='<input type="hidden" name="idx" value="'+i+'">';
    h+='eink/'+i+'.png <button class="del" type="submit">Delete</button>';
    h+='</form><br>';
  }
  document.getElementById('imglist').innerHTML=h;
});
</script>
</div>
</body></html>
)rawliteral";

void CaptivePortal::showSetupScreen() {
    display.setDisplayMode(INKPLATE_1BIT);
    display.clearDisplay();
    display.setFont(&FreeSerifBold18pt7b);
    display.setTextSize(2);
    display.setTextColor(BLACK, WHITE);
    display.setCursor(100, 200);
    display.print(F("WiFi Setup Mode"));

    display.setFont(&FreeSerifBold12pt7b);
    display.setTextSize(1);
    display.setCursor(100, 300);
    display.print(F("1. Connect to WiFi: Inkplate-Setup"));
    display.setCursor(100, 340);
    display.print(F("2. Open browser to: 192.168.4.1"));
    display.setCursor(100, 380);
    display.print(F("3. Configure WiFi and upload images"));
    display.setCursor(100, 440);
    display.print(F("Portal will close after 5 minutes"));

    display.display();
}

int CaptivePortal::nextFileIndex() {
    if (!display.sdCardInit()) return 0;
    SdFile file;
    char fname[32];
    int idx = 0;
    while (true) {
        sprintf(fname, "eink/%d.png", idx);
        if (!file.open(fname, O_RDONLY)) break;
        file.close();
        idx++;
    }
    return idx;
}

void CaptivePortal::start() {
    showSetupScreen();

    // Start AP
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Inkplate-Setup");
    delay(100);
    Serial.print(F("AP IP: "));
    Serial.println(WiFi.softAPIP());

    dnsServer = new DNSServer();
    server = new WebServer(80);

    // DNS redirect all to our IP
    dnsServer->start(53, "*", WiFi.softAPIP());

    // Routes
    server->on("/", HTTP_GET, [this]() { handleRoot(); });
    server->on("/save", HTTP_POST, [this]() { handleSave(); });
    server->on("/upload", HTTP_POST,
        [this]() { handleUploadPost(); },
        [this]() { handleUpload(); }
    );
    server->on("/images", HTTP_GET, [this]() { handleImageList(); });
    server->on("/delete", HTTP_POST, [this]() { handleDelete(); });
    server->onNotFound([this]() { handleNotFound(); });

    server->begin();
    Serial.println(F("Captive portal started"));

    // Run for 5 minutes
    unsigned long startTime = millis();
    unsigned long timeout = 5 * 60 * 1000;
    while (millis() - startTime < timeout) {
        dnsServer->processNextRequest();
        server->handleClient();
        delay(10);
    }

    Serial.println(F("Portal timeout, going to deep sleep"));
    server->stop();
    delete server;
    dnsServer->stop();
    delete dnsServer;

    // Deep sleep after portal timeout
    esp_sleep_enable_timer_wakeup(60ULL * 60 * 1000 * 1000); // 1 hour
    esp_deep_sleep_start();
}

void CaptivePortal::handleRoot() {
    String html = FPSTR(CONFIG_HTML);
    html.replace("%SSID%", SECRET_SSID);
    html.replace("%CITY%", SECRET_CITY);
    html.replace("%TZ%", String(SECRET_TIMEZONE));
    html.replace("%WKEY%", WEATHER_API_KEY);
    html.replace("%CKEY%", CALENDARIFIC_KEY);
    server->send(200, "text/html", html);
}

void CaptivePortal::handleSave() {
    // Read form values
    String ssid = server->arg("ssid");
    String pass = server->arg("pass");
    String city = server->arg("city");
    String tz = server->arg("tz");
    String weatherKey = server->arg("weather_key");
    String calKey = server->arg("cal_key");

    // Update globals
    strlcpy(SECRET_SSID, ssid.c_str(), sizeof(SECRET_SSID));
    if (pass.length() > 0) {
        strlcpy(SECRET_PASS, pass.c_str(), sizeof(SECRET_PASS));
    }
    strlcpy(SECRET_CITY, city.c_str(), sizeof(SECRET_CITY));
    SECRET_TIMEZONE = tz.toInt();
    strlcpy(WEATHER_API_KEY, weatherKey.c_str(), sizeof(WEATHER_API_KEY));
    strlcpy(CALENDARIFIC_KEY, calKey.c_str(), sizeof(CALENDARIFIC_KEY));

    // Save to SD card
    if (display.sdCardInit()) {
        SdFile file;
        // Remove old file first
        if (file.open("settings.json", O_WRONLY | O_CREAT | O_TRUNC)) {
            StaticJsonDocument<1024> doc;
            doc["ssid"] = SECRET_SSID;
            doc["wifi_password"] = SECRET_PASS;
            doc["city"] = SECRET_CITY;
            doc["timezone"] = SECRET_TIMEZONE;
            doc["weather_api_key"] = WEATHER_API_KEY;
            doc["calendarific_key"] = CALENDARIFIC_KEY;
            serializeJson(doc, file);
            file.close();
            Serial.println(F("Settings saved to SD"));
        }
    }

    server->send(200, "text/html",
        "<html><body><h1>Settings Saved!</h1>"
        "<p>The device will restart now.</p></body></html>");
    delay(2000);
    ESP.restart();
}

void CaptivePortal::handleUpload() {
    HTTPUpload &upload = server->upload();
    if (upload.status == UPLOAD_FILE_START) {
        if (!display.sdCardInit()) {
            Serial.println(F("SD card init failed for upload"));
            return;
        }
        // Create eink directory if needed
        SdFile dir;
        if (!dir.open("eink", O_RDONLY)) {
            display.getSdFat().mkdir("eink");
        } else {
            dir.close();
        }

        int idx = nextFileIndex();
        char fname[32];
        sprintf(fname, "eink/%d.png", idx);
        Serial.print(F("Upload start: "));
        Serial.println(fname);
        uploadInProgress = uploadFile.open(fname, O_WRONLY | O_CREAT | O_TRUNC);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (uploadInProgress && upload.currentSize > 0) {
            uploadFile.write(upload.buf, upload.currentSize);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (uploadInProgress) {
            uploadFile.close();
            uploadInProgress = false;
            Serial.print(F("Upload complete, size: "));
            Serial.println(upload.totalSize);
        }
    }
}

void CaptivePortal::handleUploadPost() {
    server->send(200, "text/html",
        "<html><body><h1>Upload Complete!</h1>"
        "<p><a href='/'>Back to setup</a></p></body></html>");
}

void CaptivePortal::handleImageList() {
    int count = nextFileIndex();
    String json = "{\"count\":" + String(count) + "}";
    server->send(200, "application/json", json);
}

void CaptivePortal::handleDelete() {
    String idx = server->arg("idx");
    char fname[32];
    sprintf(fname, "eink/%s.png", idx.c_str());

    if (display.sdCardInit()) {
        SdFile file;
        if (file.open(fname, O_WRONLY)) {
            file.remove();
            Serial.print(F("Deleted: "));
            Serial.println(fname);

            // Renumber files to fill the gap
            int deleted = idx.toInt();
            int total = nextFileIndex();
            for (int i = deleted; i < total; i++) {
                char oldName[32], newName[32];
                sprintf(oldName, "eink/%d.png", i + 1);
                sprintf(newName, "eink/%d.png", i);
                SdFile f;
                if (f.open(oldName, O_RDWR)) {
                    f.rename(newName);
                    f.close();
                }
            }
        }
    }

    server->send(200, "text/html",
        "<html><body><h1>Deleted!</h1>"
        "<p><a href='/'>Back to setup</a></p></body></html>");
}

void CaptivePortal::handleNotFound() {
    // Redirect all unknown requests to root (captive portal behavior)
    server->sendHeader("Location", "http://192.168.4.1/", true);
    server->send(302, "text/plain", "");
}
