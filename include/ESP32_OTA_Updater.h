#ifndef ESP32_OTA_UPDATER_H_
#define ESP32_OTA_UPDATER_H_
/**
 * @file ESP32-OTA-Updater.h
 * @brief Header file for the ESP32_OTA_Updater class.
 */

#include <WString.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <FS.h>

#include "Errors.h"
#include "SemanticVersion.h"

#define ESP32_OTA_UPDATER_SHORTSTRING_LENGTH 25
#define ESP32_OTA_UPDATER_LONGSTRING_LENGTH 50

#ifndef ESP32_OTA_UPDATER_FORMAT_LITTLEFS_IF_FAILED
#define ESP32_OTA_UPDATER_FORMAT_LITTLEFS_IF_FAILED true
#endif

/**
 * @class ESP32_OTA_Updater
 * @brief Class for performing Over-The-Air (OTA) updates on ESP32 devices, with Github Actions and Releases.
 * 
 * This class checks for new Github Releases compares the semantic version and downloads/installs the firmware update. For now the System automatically uses the LittleFS file system, but a custom fs:FS can be specified on begin().
 * 
 */
class ESP32_OTA_Updater
{
private:
    char repositry_owner[ESP32_OTA_UPDATER_SHORTSTRING_LENGTH]; /**< The owner of the repository where the firmware is build an released. */
    char repositry_name[ESP32_OTA_UPDATER_SHORTSTRING_LENGTH]; /**< The name of the repository where the firmware is build an released. */
    char gh_api_key[ESP32_OTA_UPDATER_LONGSTRING_LENGTH]; /**< (Fine Grained) Github Personal Access Token. Required for private repositries! */
    bool api_key_defined; /**< True if the Github API key is defined, false otherwise. */
    char firmware_asset_path[ESP32_OTA_UPDATER_SHORTSTRING_LENGTH]; /**< The path to the firmware binary file on the Github Release -> the asset name. */
    
    bool new_version_available = false; /**< True if a new firmware version is available, false otherwise. */
    Version new_version = NULL; /**< The new semantic version of the firmware. */
    char binary_download_url[ESP32_OTA_UPDATER_SHORTSTRING_LENGTH]; /**< The URL to download the firmware binary file. */
    int binary_size = 0; /**< The size of the firmware binary file. */

    Version current_version; /**< The current semantic version of the firmware. */
    ESP32_OTA_Updater_Error error; /**< The error status of the OTA updater. */
    WiFiClientSecure *wifi_client_secure; /**< The WifiClientSecure object for HTTPS communication. */
    HTTPClient http_client; /**< The HTTPClient object for making HTTP requests. */
    fs::FS *filesystem; /**< The file system object for reading and writing files. */

public:
    
    /**
     * @brief Constructs an instance of the ESP32_OTA_Updater class.
     *
     * @param owner The owner of the repository where the firmware is build an released.
     * @param repo The name of the repository where the firmware is build an released.
     * @param firmware_path The path to the firmware binary file on the Github Release -> the asset name.
     * @param current_version The current version of the firmware.
     * @note Only public repositries are supported without the `gh_api
     */
    ESP32_OTA_Updater(const char owner[], const char repo[], const char firmware_path[], const char current_version[]);

    /**
     * @brief Constructs an instance of the ESP32_OTA_Updater class.
     *
     * @param owner The owner of the repository where the firmware is build an released.
     * @param repo The name of the repository where the firmware is build an released.
     * @param firmware_path The path to the firmware binary file on the Github Release -> the asset name.
     * @param current_version The current version of the firmware.
     * @param gh_api_key The (Fine Grained) Github Personal Access Token.
     */
    ESP32_OTA_Updater(const char owner[], const char repo[], const char firmware_path[], const char current_version[], const char gh_api_key[]);

    /**
     * @brief Initializes the ESP32 OTA Updater.
     * 
     * @note This tries to mount the LittleFS file system. If it fails, it will format the LittleFS file system, this can be turned off by overwriting the ESP32_OTA_UPDATER_FORMAT_LITTLEFS_IF_FAILED macro. 
     */
    bool begin();

    /**
     * @brief Initializes the ESP32 OTA Updater with a custom file system.
     * 
     * @note The filesystem passed must be initialized and mounted. 
     */    
    bool begin(fs::FS *customFS);

    /**
     * @brief Checks if a firmware update is available.
     * 
     * This function checks if a new firmware update is available by comparing the current version with the version
     * specified in the firmware update file.
     * 
     * @return True if a firmware update is available, false otherwise.
     */
    bool available();

    /**
     * @brief Downloads and installs the firmware update.
     * 
     * This function downloads the firmware update file from the specified URL and installs it on the ESP32 device.
     * 
     * @note This function should only be called if a firmware update is available (i.e., `available()` returns true).
     */
    bool downloadAndInstall();

    /**
     * @brief Get the error code.
     *
     * This function returns the error code associated with the last operation performed by the ESP32 OTA Updater.
     *
     * @return The error code.
     */
    ESP32_OTA_Updater_Error getErrorCode();
    /**
     * @brief Returns the description of the last error that occurred.
     *
     * This function returns a string containing the description of the last error that occurred during the execution of the program.
     *
     * @return A string containing the description of the last error.
     */
    String getErrorDescription();
};

#endif