# Smart Parking System (ESP32)

## Getting Started

1. Install [Visual Studio Code](https://code.visualstudio.com).
2. Install the extension [PlatformIO IDE](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)
3. Create `secrets.h` on the `src` folder.
4. Add the following lines to `secrets.h` then fill in the values:

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

5. Open the project on PIO Home.
6. Build or upload the project.
