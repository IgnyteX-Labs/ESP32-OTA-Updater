#include <Arduino.h> // Remove this import when using Arduino IDE
#include <WiFi.h>
#include <ESP32_OTA_Updater.h>

// Find all SSL Certificates from Githubs endpoints and concatenate them into the char as follows (currently github's ECC and RSA cert are required)
static const char *ROOT_CA_CERTIFICATE_GITHUB =
    "-----BEGIN CERTIFICATE-----\n"
    // Cert 1 here
    "-----END CERTIFICATE-----\n"
    "-----BEGIN CERTIFICATE-----\n"
    // Cert 2 here
    "-----END CERTIFICATE-----\n";
// Define the github repository to download the update from
#define OWNER ""
#define REPO ""
#define FIRMWARE "firmware.bin"
// (Optional)
#define GITHUB_ACCESS_TOKEN ""

// Initialize the OTA Updater (Note that the PROJ_GIT_TAG property has to be set in the pio build flags, the example Github Worklfow shows how to do that. (look at the README example for how to do that))
#ifndef PROJ_GIT_TAG
#define PROJ_GIT_TAG "v0.0.0" // If not set in a build flag the version is set to 0.0.0
#endif
ESP32_OTA_Updater ota(ROOT_CA_CERTIFICATE_GITHUB, PROJ_GIT_TAG);
void setup()
{
    Serial.begin(115200);
    Serial.println("Startup!");
    // Setup WiFi Connection
    WiFi.begin("NETSMITH", "granby06060");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");

    // Initialize the OTA Updater
    // Setup debug
    ota.setDebug(&Serial);

    if (!ota.begin(OWNER, REPO, FIRMWARE, GITHUB_ACCESS_TOKEN))
    {
        Serial.printf("An error occurred when trying to initialize the OTA Updater, Error Code: %d, Description: %s\n", ota.getErrorCode(), ota.getErrorDescription());
    }
    else
    {
        Serial.println("OTA Updater initialized successfully");
    }

    Serial.printf("Running version %s.\n", PROJ_GIT_TAG);
}

void loop()
{
    // This has to be run repeatedly (the library uses millis internally to not make an api call to often...)
    if (ota.available())
    {
        Serial.println("Update Available!");
        // Try to install the available update
        if (!ota.downloadAndInstall())
        {
            Serial.printf("An error occurred when trying to install an ota update: Error Code: %d, Description: %s\n", ota.getErrorCode(), ota.getErrorDescription());
        }
        // The update was successfully downloaded and preprared for installation:
        Serial.println("Successfully downloaded and installed the update, reboot now!");
        // PUT ANY WORK THAT SHOULD BE DONE BEFORE REBOOTING HERE!

        ota.reboot();
    }
    else
    {
        Serial.println("No new update");
    }
    if (ota.getErrorCode() != NO_ERROR)
    {
        Serial.printf("Error occured code: %d, desc.: %s\n", ota.getErrorCode(), ota.getErrorDescription());
        delay(100);
    }
    delay(2000);
}