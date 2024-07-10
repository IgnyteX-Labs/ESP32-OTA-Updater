#include "stdint.h"

enum ESP32_OTA_Updater_Error: uint8_t
{
    NO_ERROR = 0,
    WIFI_NOT_CONNECTED,
    NOT_INITIALIZED,
    OTA_NOT_AVAILABLE,
    OTA_DOWNLOAD_FAILED,
    OTA_INSTALL_FAILED
};