#include "ESP32_OTA_Updater.h"
#include <Wifi.h>

ESP32_OTA_Updater::ESP32_OTA_Updater()
{
    github_url = "";
    current_version = "";
    error = ESP32_OTA_Updater_Error::NOT_INITIALIZED;
}

bool ESP32_OTA_Updater::begin(const char url[], const char currentVersion[])
{
    github_url = url;
    current_version = currentVersion;

    if(!WiFi.isConnected())
    {
        error = ESP32_OTA_Updater_Error::WIFI_NOT_CONNECTED;
        return false;
    }
    error = ESP32_OTA_Updater_Error::NO_ERROR;
    return true;
}

bool ESP32_OTA_Updater::available()
{
    if(error != ESP32_OTA_Updater_Error::NO_ERROR)
    {
        return false;
    }

    // Check if a firmware update is available

    return true;
}

bool ESP32_OTA_Updater::downloadAndInstall()
{
    if(error != ESP32_OTA_Updater_Error::NO_ERROR)
    {
        return false;
    }

    // Download and install the firmware update
}

ESP32_OTA_Updater_Error ESP32_OTA_Updater::getErrorCode()
{
    return error;
}

String ESP32_OTA_Updater::getErrorDescription()
{
    switch(error)
    {
        case ESP32_OTA_Updater_Error::NO_ERROR:
            return "No error";
        case ESP32_OTA_Updater_Error::WIFI_NOT_CONNECTED:
            return "Wifi not connected";
        case ESP32_OTA_Updater_Error::NOT_INITIALIZED:
            return "Not initialized";
        case ESP32_OTA_Updater_Error::OTA_NOT_AVAILABLE:
            return "OTA Update not available, please configure the git repo!";
        case ESP32_OTA_Updater_Error::OTA_DOWNLOAD_FAILED:
            return "OTA download failed";
        case ESP32_OTA_Updater_Error::OTA_INSTALL_FAILED:
            return "OTA install failed";
        default:
            return "Unknown error";
    }
}