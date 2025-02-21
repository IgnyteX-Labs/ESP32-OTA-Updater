#include "ESP32_OTA_Updater.h"
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <Update.h>

ESP32_OTA_Updater::ESP32_OTA_Updater(const char owner[], const char repo[], const char firmware_path[], const char current_version[]): current_version(Version(current_version))
{
    strncpy(repositry_owner, owner, ESP32_OTA_UPDATER_SHORTSTRING_LENGTH);
    strncpy(repositry_name, repo, ESP32_OTA_UPDATER_SHORTSTRING_LENGTH);
    strncpy(firmware_asset_path, firmware_path, ESP32_OTA_UPDATER_SHORTSTRING_LENGTH);
    wifi_client_secure = new WiFiClientSecure();
    api_key_defined = false;
    
    error = ESP32_OTA_Updater_Error::NO_ERROR;
}

ESP32_OTA_Updater::ESP32_OTA_Updater(const char owner[], const char repo[], const char firmware_path[], const char current_version[], const char api_key[]): current_version(Version(current_version))
{
    strncpy(repositry_owner, owner, ESP32_OTA_UPDATER_SHORTSTRING_LENGTH);
    strncpy(repositry_name, repo, ESP32_OTA_UPDATER_SHORTSTRING_LENGTH);
    strncpy(firmware_asset_path, firmware_path, ESP32_OTA_UPDATER_SHORTSTRING_LENGTH);
    wifi_client_secure = new WiFiClientSecure();
    strncpy(gh_api_key, api_key, ESP32_OTA_UPDATER_LONGSTRING_LENGTH);
    api_key_defined = true;    

    error = ESP32_OTA_Updater_Error::NO_ERROR;
}

bool ESP32_OTA_Updater::begin() {
    if(!LittleFS.begin(ESP32_OTA_UPDATER_FORMAT_LITTLEFS_IF_FAILED))
    {
        error = ESP32_OTA_Updater_Error::FS_FAILED;
        return false;
    }
}

bool ESP32_OTA_Updater::begin(fs::FS *customFS) {
    filesystem = customFS;
    return true;
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
    if(api_key_defined)
    {
        http_client.addHeader("Authorization: Bearer %s", gh_api_key);
    }
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
    
    if(latest_version <= current_version) {
        return false;
    }
    // Find the asset with the binary:
    if(!doc.containsKey("assets"))
    {
        error = ESP32_OTA_Updater_Error::OTA_RESPONSE_INVALID;
        return false;
    }
    JsonArray assets = doc["assets"].as<JsonArray>();
    bool assetFound = false;
    for(JsonObject jsonAsset: assets) {
        if(jsonAsset.containsKey("name") && strcmp(jsonAsset["name"].as<const char*>(), firmware_asset_path) == 0)
        {
            // Found the download URL
            strncpy(binary_download_url, jsonAsset["url"].as<const char*>(), ESP32_OTA_UPDATER_LONGSTRING_LENGTH);
            binary_size = jsonAsset["size"].as<int>();
            
            new_version_available = true;
            new_version = latest_version;
            assetFound = true;
            break;
        }
    }
    if(!assetFound)
    {
        error = ESP32_OTA_Updater_Error::OTA_RESPONSE_INVALID;
        return false;
    }
    
   return true;
}

bool ESP32_OTA_Updater::downloadAndInstall()
{
    if(error != ESP32_OTA_Updater_Error::NO_ERROR)
    {
        return false;
    }

    // Download the firmware update
    
    // Open the file to write the firmware to
    char firmware_path[strlen(firmware_asset_path)+19+1];
    sprintf(firmware_path, "/esp32_ota_updater_%s", firmware_asset_path);
    fs::File file = filesystem->open(firmware_path, "w");
    if(!file)
    {
        error = ESP32_OTA_Updater_Error::FS_FAILED;
        return false;
    }

    // Download the firmware from the URL
    if(!http_client.begin(*wifi_client_secure, binary_download_url))
    {
        error = ESP32_OTA_Updater_Error::OTA_DOWNLOAD_FAILED;
        return false;
    }

    http_client.addHeader("Accept", "application/octet-stream");
    if(api_key_defined)
    {
        http_client.addHeader("Authorization: Bearer %s", gh_api_key);
    }
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
    // Directly write the download stream to the file stream...
    http_client.writeToStream(&file);

    file.close();
    
    // Install the firmware...

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
        case ESP32_OTA_Updater_Error::FS_FAILED:
            return "A Problem with the filesystem occurred";
        default:
            return "Unknown error";
    }
}