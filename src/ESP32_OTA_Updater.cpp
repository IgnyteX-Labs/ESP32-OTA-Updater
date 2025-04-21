#include "ESP32_OTA_Updater.h"
#include <ArduinoJson.h>
#include <Update.h>

ESP32_OTA_Updater::ESP32_OTA_Updater(const char *rootCertificate)
{
    error = ESP32_OTA_Updater_Error::NOT_INITIALIZED;
    rootCert = rootCertificate;
}

bool ESP32_OTA_Updater::begin(const char *owner, const char *repo, const char *firmware_path, const char *current_version, const char *api_key)
{
    _begin(owner, repo, firmware_path, current_version);

    strncpy(gh_api_key, api_key, ESP32_OTA_UPDATER_LONGSTRING_LENGTH);
    api_key_defined = true;

    error = ESP32_OTA_Updater_Error::NO_ERROR;
    return true;
}

bool ESP32_OTA_Updater::begin(const char *owner, const char *repo, const char *firmware_path, const char *current_version)
{
    _begin(owner, repo, firmware_path, current_version);

    api_key_defined = false;

    error = ESP32_OTA_Updater_Error::NO_ERROR;
    return true;
}

inline void ESP32_OTA_Updater::_begin(const char *owner, const char *repo, const char *firmware_path, const char *current_version)
{
    strncpy(repositry_owner, owner, ESP32_OTA_UPDATER_SHORTSTRING_LENGTH);
    strncpy(repositry_name, repo, ESP32_OTA_UPDATER_SHORTSTRING_LENGTH);
    strncpy(firmware_asset_path, firmware_path, ESP32_OTA_UPDATER_SHORTSTRING_LENGTH);
    this->current_version = Version(current_version);

    wifi_client_secure.setCACert(rootCert);

    Update.onProgress(std::bind(&ESP32_OTA_Updater::updateProgressCallback, this, std::placeholders::_1, std::placeholders::_2));
}

void ESP32_OTA_Updater::updateProgressCallback(size_t progress, size_t size)
{
    // Update Progress
    Serial.printf("Installing %d of %d bytes.\n", progress, size);
}

int ESP32_OTA_Updater::httpclientSendRequest(HTTPClient &http_client)
{
    http_client.useHTTP10(true); // use HTTP/1.0 for update since the update handler not support any transfer Encoding
    http_client.setTimeout(15000);
    http_client.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http_client.setUserAgent(http_useragent);

    // Add Authorization Header for Github API if defined.
    if (api_key_defined)
    {
        http_client.setAuthorizationType("Bearer");
        http_client.setAuthorization(gh_api_key);
    }

    http_client.addHeader("X-GitHub-Api-Version", "2022-11-28"); // Set Github Api Version

    int code = http_client.GET();
    int len = http_client.getSize();

    if (code == HTTP_CODE_OK)
    {
        if (len <= 0)
        {
            error = OTA_RESPONSE_INVALID;
            return 0;
        }
        return len; // return the length of the response
    }
    else if (code < 0)
    {
        error = ESP32_OTA_Updater_Error::WIFI_NOT_CONNECTED;
        return code; // Return a negative value to indicate failure
    }
    else
    {
        error = ESP32_OTA_Updater_Error::OTA_NOT_AVAILABLE;
        return -code; // Return (minus) the response code to indicate failure although a successful request was made
    }
}

bool ESP32_OTA_Updater::available()
{
    /*
     * TODO: Implement a millis nonblocking return false to not always perform an http request
     */

    if (error != ESP32_OTA_Updater_Error::NO_ERROR)
    {
        return false;
    }

    // Check if a firmware update is available
    const size_t urlLen = 50 + strlen(repositry_owner) + strlen(repositry_name);
    char url[urlLen];
    snprintf(url, urlLen, "https://api.github.com/repos/%s/%s/releases/latest", repositry_owner, repositry_name);

    if (!http_client.begin(wifi_client_secure, url))
    {
        error = ESP32_OTA_Updater_Error::OTA_NOT_AVAILABLE;
        return false;
    }
    http_client.addHeader("Accept", "application/vnd.github+json");

    if (httpclientSendRequest(http_client) <= 0)
    {
        return false; // Error Codes are set in the method itself
    }

    // ***** Response Decoding and Handling ******
    JsonDocument doc;

    DeserializationError json_error = deserializeJson(doc, http_client.getStream());
    http_client.end();
    if (json_error)
    {
        error = ESP32_OTA_Updater_Error::OTA_FAILED_TO_DESERIALIZE;
        return false;
    }

    // Check version from the JSON response
    if (!doc.containsKey("tag_name"))
    {
        error = ESP32_OTA_Updater_Error::OTA_RESPONSE_INVALID;
        return false;
    }
    Version latest_version(Version(doc["tag_name"].as<const char *>()));

    if (latest_version <= current_version)
    {
        return false;
    }
    // Find the asset with the binary:
    if (!doc.containsKey("assets"))
    {
        error = ESP32_OTA_Updater_Error::OTA_RESPONSE_INVALID;
        return false;
    }
    JsonArray assets = doc["assets"].as<JsonArray>();
    bool assetFound = false;
    for (JsonObject jsonAsset : assets)
    {
        if (jsonAsset.containsKey("name") && strcmp(jsonAsset["name"].as<const char *>(), firmware_asset_path) == 0)
        {
            // Found the download URL
            strncpy(binary_download_url, jsonAsset["url"].as<const char *>(), ESP32_OTA_UPDATER_LONGSTRING_LENGTH);
            binary_size = jsonAsset["size"].as<int>();

            new_version_available = true;
            new_version = latest_version;
            assetFound = true;
            break;
        }
    }
    if (!assetFound)
    {
        error = ESP32_OTA_Updater_Error::OTA_RESPONSE_INVALID;
        return false;
    }

    return true;
}

bool ESP32_OTA_Updater::downloadAndInstall()
{
    if (error != ESP32_OTA_Updater_Error::NO_ERROR)
    {
        return false;
    }

    // Download the firmware from the URL
    if (!http_client.begin(wifi_client_secure, binary_download_url))
    {
        error = ESP32_OTA_Updater_Error::OTA_DOWNLOAD_FAILED;
        return false;
    }
    http_client.addHeader("Accept", "application/octet-stream");
    http_client.addHeader("Cache-Control", "no-cache");

    int update_size = httpclientSendRequest(http_client);
    if (update_size <= 0)
    {
        return false; // Error codes are set in the method itself!
    }

    // Update of size update_size is ready for download

    // Directly write the download stream to the file stream...
    NetworkClient *client = http_client.getStreamPtr();

    if (!Update.begin(update_size, U_FLASH))
    {
        error = ESP32_OTA_Updater_Error::OTA_INSTALL_FAILED;
        http_client.end();
        return false;
    }

    /*
     * TODO: Implemente Crypto at this point, to allow for encrypted firmware binaries.
     */

    if (Update.writeStream(*client) != update_size)
    {
        error = ESP32_OTA_Updater_Error::OTA_INSTALL_FAILED;
        http_client.end();
        return false;
    }

    if (!Update.end())
    {
        error = ESP32_OTA_Updater_Error::OTA_INSTALL_FAILED;
        http_client.end();
        return false;
    }

    http_client.end();

    return true;
}

void ESP32_OTA_Updater::reboot()
{
    ESP.restart();
}

ESP32_OTA_Updater_Error ESP32_OTA_Updater::getErrorCode()
{
    return error;
}

String ESP32_OTA_Updater::getErrorDescription()
{
    switch (error)
    {
    case ESP32_OTA_Updater_Error::NO_ERROR:
        return F("No error");
    case ESP32_OTA_Updater_Error::WIFI_NOT_CONNECTED:
        return F("Wifi not connected");
    case ESP32_OTA_Updater_Error::NOT_INITIALIZED:
        return F("Not initialized");
    case ESP32_OTA_Updater_Error::OTA_NOT_AVAILABLE:
        return F("OTA Update not available, please configure the git repo!");
    case ESP32_OTA_Updater_Error::OTA_DOWNLOAD_FAILED:
        return F("OTA download failed");
    case ESP32_OTA_Updater_Error::OTA_INSTALL_FAILED:
        return F("OTA install failed");
    case ESP32_OTA_Updater_Error::OTA_FAILED_TO_DESERIALIZE:
        return F("OTA failed to deserialize");
    case ESP32_OTA_Updater_Error::OTA_RESPONSE_INVALID:
        return F("OTA response invalid");
    default:
        return F("Unknown error");
    }
}