# Smart Parking System (ESP32)

## Getting Started

1. Install [Visual Studio Code](https://code.visualstudio.com).
2. Install the extension [C/C++ Pre-release version](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools). Pre-release version is a must to utilize clang-format v14.
3. Install the extension [PlatformIO IDE](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide).
4. Create `secrets.h` on the `src` folder.
5. Add the following lines to `secrets.h` then fill in the values:

   ```cpp
   // WIFI
   #define WIFI_SSID ""
   #define WIFI_PASSWORD ""

   // Firebase
   #define FIREBASE_DATABASE_URL ""
   #define FIREBASE_API_KEY ""
   #define FIREBASE_EMAIL ""
   #define FIREBASE_PASSWORD ""
   ```

6. Open the project on PIO Home.
7. Build or upload the project.
