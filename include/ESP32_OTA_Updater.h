#ifndef ESP32_OTA_UPDATER_H_
#define ESP32_OTA_UPDATER_H_
/**
 * @file ESP32-OTA-Updater.h
 * @brief Header file for the ESP32_OTA_Updater class.
 */

#include <WString.h>
#include "Errors.h"

/**
 * @class ESP32_OTA_Updater
 * @brief Class for performing Over-The-Air (OTA) updates on ESP32 devices, with Github Actions and Releases.
 * 
 * This class checks for new Github Releases compares the semantic version and downloads/installs the firmware update.
 * 
 */
class ESP32_OTA_Updater
{
private:
    const char *github_url; /**< The URL of the main Github Repository. */
    const char *current_version; /**< The current semantic version of the firmware. */
    ESP32_OTA_Updater_Error error; /**< The error status of the OTA updater. */

public:
    
    /**
     * @brief Constructs an instance of the ESP32_OTA_Updater class.
     */
    ESP32_OTA_Updater();

    /**
     * @brief Initializes the OTA updater.
     * 
     * This function should be called once at the beginning of the program to initialize the OTA updater.
     * 
     * @param url The URL of the main Github Repository.
     * @param currentVersion The current semantic version of the firmware. (e.g. "1.0.0")
     * @return True if the initialization is successful, false otherwise.
     * 
     * @note This functions requires the ESP32 Wifi class to be initialized and connected before calling.
     */
    bool begin(const char url[], const char currentVersion[]);

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