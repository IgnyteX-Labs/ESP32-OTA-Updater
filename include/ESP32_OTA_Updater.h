#ifndef ESP32_OTA_UPDATER_H_
#define ESP32_OTA_UPDATER_H_
/**
 * @file ESP32-OTA-Updater.h
 * @brief Header file for the ESP32_OTA_Updater class.
 */

#include <WString.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include "Errors.h"
#include "SemanticVersion.h"

#define ESP32_OTA_UPDATER_SHORTSTRING_LENGTH 50
#define ESP32_OTA_UPDATER_LONGSTRING_LENGTH 150

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
    char repositry_owner[ESP32_OTA_UPDATER_SHORTSTRING_LENGTH];     /**< The owner of the repository where the firmware is build an released. */
    char repositry_name[ESP32_OTA_UPDATER_SHORTSTRING_LENGTH];      /**< The name of the repository where the firmware is build an released. */
    char gh_api_key[ESP32_OTA_UPDATER_LONGSTRING_LENGTH];           /**< (Fine Grained) Github Personal Access Token. Required for private repositries! */
    bool api_key_defined;                                           /**< True if the Github API key is defined, false otherwise. */
    char firmware_asset_path[ESP32_OTA_UPDATER_SHORTSTRING_LENGTH]; /**< The path to the firmware binary file on the Github Release -> the asset name. */
    const char *http_useragent = "ESP32-OTA-Updater";

    bool new_version_available = false;                            /**< True if a new firmware version is available, false otherwise. */
    char binary_download_url[ESP32_OTA_UPDATER_LONGSTRING_LENGTH]; /**< The URL to download the firmware binary file. */
    int binary_size = 0;                                           /**< The size of the firmware binary file. */

    const Version current_version;       /**< The current semantic version of the firmware. */
    ESP32_OTA_Updater_Error error;       /**< The error status of the OTA updater. */
    WiFiClientSecure wifi_client_secure; /**< The WifiClientSecure object for HTTPS communication. */
    HTTPClient http_client;              /**< The HTTPClient object for making HTTP requests. */

    const char *rootCert;

    void updateProgressCallback(size_t progress, size_t size);
    int httpclientSendRequest(HTTPClient &http_client);
    inline void _begin(const char *owner, const char *repo, const char *firmware_path);

    Print *debugPrinter = NULL;
    void debugf(const char *format, ...);

public:
    /**
     * @brief Constructor for the ESP32_OTA_Updater class.
     *
     * This constructor initializes the OTA updater with the specified root certificates
     * for secure HTTPS communication.
     *
     * @note Please supply the all Certs for all Github endpoints concatenated in one string
     *
     * @param rootCertificate The root certificates for GitHub's HTTPS server and Githubs Download Server.
     * @param current_version The semantic version of the fimware currently installed, should be "v.0.0.0" or "0.0.0"
     */
    ESP32_OTA_Updater(const char *rootCertificate, const char *current_version);

    /**
     * @brief Initializes the ESP32 OTA Updater.
     *
     * @param owner The owner of the repository where the firmware is build an released.
     * @param repo The name of the repository where the firmware is build an released.
     * @param firmware_path The path to the firmware binary file on the Github Release -> the asset name.
     * @param gh_api_key The (Fine Grained) Github Personal Access Token.
     *
     * @return true if the initialization was successful, false otherwise.
     */
    bool begin(const char *owner, const char *repo, const char *firmware_path, const char *api_key);

    /**
     * @brief Initializes the ESP32 OTA Updater.
     *
     * @param owner The owner of the repository where the firmware is build an released.
     * @param repo The name of the repository where the firmware is build an released.
     * @param firmware_path The path to the firmware binary file on the Github Release -> the asset name.
     *
     * @return true if the initialization was successful, false otherwise.
     */
    bool begin(const char *owner, const char *repo, const char *firmware_path);

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
     * @brief Downloads and installs the firmware update. Use 'reboot()' for the esp to reboot and the update to take effect!
     *
     * This function downloads the firmware update file from the specified URL and installs it on the ESP32 device.
     *
     * @note This function should only be called if a firmware update is available (i.e., `available()` returns true).
     */
    bool downloadAndInstall();

    /**
     * @brief Initiates a reboot of the esp32, which effectively finished a previously installed update.
     */
    void reboot();

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

    /**
     * @brief Sets the debug output stream for logging messages.
     *
     * @param debugStream A pointer to a Print object, e.g. Serial.
     *                     Pass NULL to disable debug logging.
     */
    void setDebug(Print *debugStream);
};

#endif