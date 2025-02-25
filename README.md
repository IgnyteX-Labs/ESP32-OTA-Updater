# ESP32 Over The Air Updater

This library provides a simple solution for performing Over-The-Air (OTA) updates on ESP32 devices using the Arduino framework. It integrates with GitHub CI/CD workflows to automate firmware updates using PlatformIO. 

## Quick Start

### PlatformIO

To get started with the ESP32 Over The Air Updater using PlatformIO, follow these steps:

1. **Install PlatformIO**: If you haven't already, install PlatformIO IDE for your preferred code editor (VSCode, Atom, etc.). You can find the installation instructions [here](https://platformio.org/install).

2. **Create a New Project**:
    - Open PlatformIO IDE.
    - Click on `New Project`.
    - Enter your project name, select `Espressif 32` as the board, and choose the appropriate ESP32 board model.
    - Click `Finish`.

3. **Add the Library**:
    - Open the `platformio.ini` file in your project.
    - The settings in this file are also important for the Github Action to compile your project.
    - Add the following lines to include the ESP32 OTA Updater library:
      ```ini
      [env:esp32dev]
      platform = espressif32
      board = esp32dev
      framework = arduino
      # Add the 'ESP32-OTA-Updater' library to the project.
      lib_deps =
        https://github.com/IgnyteX-Labs/ESP32-OTA-Updater
      # Add the current git tag (e.g. v0.0.1) as macro (#define) to the source code.
      build_flags =
        !echo '-D GIT_TAG=\\"'$(git describe --tags)'\\"'
      ```
4. **Implement OTA Update**:

    To implement the OTA update functionality, follow these steps:

    #### Include Libraries and Define Variables

    First, include the necessary libraries and define the required global variables:
    ```cpp
    #include <Arduino.h> // Remove this import when using Arduino IDE
    #include <WiFi.h>
    #include <ESP32_OTA_Updater.h>

    // Define the GitHub repository details
    #define OWNER "github_username"
    #define REPO "github_repo"
    #define FIRMWARE "firmware.bin"
    #define GITHUB_ACCESS_TOKEN "finegrained_gh_token" // Optional

    ESP32_OTA_Updater ota;
    ```

    #### Setup Function
    In the `setup` function, initialize the WiFi connection and the OTA updater:
    ```cpp
    void setup() {
        WiFi.begin("SSID", "PASSWORD");
        while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
            Serial.println("Connecting to WiFi..");
        }
        Serial.println("Connected to WiFi");

        if (!ota.begin(OWNER, REPO, FIRMWARE, GIT_TAG, GITHUB_ACCESS_TOKEN)) {
            Serial.printf("OTA Updater init failed: %d, %s\n", ota.getErrorCode(), ota.getErrorDescription());
        } else {
            Serial.println("OTA Updater initialized");
        }
    }
    ```

    #### Loop Function
    In the `loop` function, check for available updates and install them if found:
    ```cpp
    void loop() {
        if (ota.available()) {
            if (!ota.downloadAndInstall()) {
                Serial.printf("OTA update failed: %d, %s\n", ota.getErrorCode(), ota.getErrorDescription());
            }
            // Perform any pre-reboot tasks here
            ota.reboot();
        }
    }
    ```

5. **Upload Your Code**:
    - Connect your ESP32 board to your computer.
    - Click on the `Upload` button in PlatformIO to upload your code to the ESP32.
    - This step is only required for the initial upload. Subsequent updates can be done over the air (OTA).

6. **Trigger OTA Update**:
    - Once your code is running on the ESP32, you can trigger an OTA update by pushing a new firmware version to your GitHub repository.
    - The ESP32 will automatically check for updates and install the new firmware.

That's it! You've successfully set up OTA updates for your ESP32 using PlatformIO.

## Documentation

For detailed documentation and usage instructions, please refer to the [Doxygen documentation](https://ignytex-labs.github.io/ESP32-OTA-Updater/) of this project. It provides comprehensive information on the library's functions to help you integrate OTA updates and Github CI seamlessly into your ESP32 projects.