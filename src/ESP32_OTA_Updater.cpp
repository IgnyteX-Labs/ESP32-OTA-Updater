#include "ESP32_OTA_Updater.h"
#include <ArduinoJson.h>

ESP32_OTA_Updater::ESP32_OTA_Updater(const char owner[], const char repo[], const char firmware_path[], const char current_version[]): current_version(Version(current_version))
{
    repositry_owner = owner;
    repositry_name = repo;
    firmware_asset_path = firmware_path;
    error = ESP32_OTA_Updater_Error::NO_ERROR;
}

bool ESP32_OTA_Updater::available()
{
    if(error != ESP32_OTA_Updater_Error::NO_ERROR)
    {
        return false;
    }

    // Check if a firmware update is available
    char url[50+strlen(repositry_owner)+strlen(repositry_name)];
    sprintf(url, "https://api.github.com/repos/%s/%s/releases/latest", repositry_owner, repositry_name);
    
    if(!http_client.begin(*wifi_client_secure, url))
    {
        error = ESP32_OTA_Updater_Error::OTA_NOT_AVAILABLE;
        return false;
    }
    http_client.addHeader("Accept", "application/vnd.github+json");
    http_client.addHeader("X-GitHub-Api-Version", "2022-11-28");
    http_client.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    
    int http_code = http_client.GET();

    if(http_code != HTTP_CODE_OK)
    {
        if(http_code < 0)
        {
            error = ESP32_OTA_Updater_Error::WIFI_NOT_CONNECTED;
        }
        else
        {
            error = ESP32_OTA_Updater_Error::OTA_NOT_AVAILABLE;
        }
        return false;
    }

    JsonDocument doc;

    DeserializationError json_error = deserializeJson(doc, http_client.getStream());
    http_client.end();
    if(json_error)
    {
        error = ESP32_OTA_Updater_Error::OTA_FAILED_TO_DESERIALIZE;
        return false;
    }

    // Check version from the JSON response
    if(!doc.containsKey("tag_name")) {
        error = ESP32_OTA_Updater_Error::OTA_RESPONSE_INVALID;
        return false;
    }
    
    Version latest_version(Version(doc["tag_name"].as<const char*>()));

    // Store the asset 
    /*
        Please NOTE: For now the available function only checks availability and does not store or cache any asset path etc.
        This is very inefficient but should not be a problem as the available function should only be called once in a while. 
        Additionally this makes the download and install function more reliable to always use the newes version...available
    */
   return latest_version > current_version;
}

bool ESP32_OTA_Updater::downloadAndInstall()
{
    if(error != ESP32_OTA_Updater_Error::NO_ERROR)
    {
        return false;
    }

    // Download and install the firmware update
    

    // Update the firmware

    return true;
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
        case ESP32_OTA_Updater_Error::OTA_FAILED_TO_DESERIALIZE:
            return "OTA failed to deserialize";
        case ESP32_OTA_Updater_Error::OTA_RESPONSE_INVALID:
            return "OTA response invalid";
        default:
            return "Unknown error";
    }
}