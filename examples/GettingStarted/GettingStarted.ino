#include <Arduino.h> // Remove this import when using Arduino IDE

#include <ESP32_OTA_Updater.h>

// Define the github repository to download the update from
#define OWNER "github_username"
#define REPO "github_repo"
#define FIRMWARE "firmware.bin"
// (Optional)
#define GITHUB_ACCESS_TOKEN "finegrained_gh_token" 

// Initialize the OTA Updater (Note that the GIT_TAG property has to be set in the pio build flags in platformio.ini (look at the README example for how to do that)) 
ESP32_OTA_Updater ota;
void setup() {
   // Setup WiFi Connection
    WiFi.begin("SSID", "PASSWORD");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");

    // Initialize the OTA Updater
    if(!ota.begin(OWNER, REPO, FIRMWARE, GIT_TAG, GITHUB_ACCESS_TOKEN)) {
        Serial.printf("An error occurred when trying to initialize the OTA Updater, Error Code: %d, Description: %s\n", ota.getErrorCode(), ota.getErrorDescription());
    }else {
        Serial.println("OTA Updater initialized successfully"); 
    }
}

void loop() {
    // This has to be run repeatedly (the library uses millis internally to not make an api call to often...)
    if(ota.available()) {
        // Try to install the available update
        if(!ota.downloadAndInstall()) {
            Serial.printf("An error occurred when trying to install an ota update: Error Code: %d, Description: %s\n", ota.getErrorCode(), ota.getErrorDescription());
        }
        // The update was successfully downloaded and preprared for installation:
        
        // PUT ANY WORK THAT SHOULD BE DONE BEFORE REBOOTING HERE!

        ota.reboot();
    }
}