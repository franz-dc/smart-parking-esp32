#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include <addons/RTDBHelper.h>
#include <addons/TokenHelper.h>
#include <secrets.h>
#include <string.h>

// set 1
#define OCCUPIED_LED_1_PIN 2
#define RESERVED_LED_1_PIN 4
#define ECHO_1_PIN 25
#define TRIG_1_PIN 26

// set 2
#define OCCUPIED_LED_2_PIN 21
#define RESERVED_LED_2_PIN 23
#define ECHO_2_PIN 32
#define TRIG_2_PIN 33

#define POLLING_RATE_MS 1000
#define MAX_DISTANCE_CM 20

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

// variable declarations
bool isReserved = false;
bool isWithinRange = false;
bool withinRangeStatuses[] = {false, false};
unsigned long sendDataPrevMillis = 0;
float newDistance = 0;
String uid = "";

void initWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void getSensorInfo(
  int sensorNumber,
  String lotNumber,
  int trigPin,
  int echoPin,
  int occupiedLedPin,
  int reservedLedPin
) {
  // generate 10-us pulse to TRIG pin
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // get sensor data: distance in cm
  newDistance = pulseIn(echoPin, HIGH) / 29.1 / 2;

  // check if distance is within range
  isWithinRange = newDistance <= MAX_DISTANCE_CM;

  // check if lot is reserved
  if (Firebase.RTDB.getBool(
        &fbdo, String("lotReservationStatuses/" + lotNumber)
      )) {
    if (fbdo.dataType() == "boolean") {
      digitalWrite(reservedLedPin, fbdo.boolData());
    }
  } else {
    Serial.println("READ FAILED: " + fbdo.errorReason());
  }

  // outputs
  digitalWrite(occupiedLedPin, isWithinRange);

  Serial.println("Lot " + lotNumber + ": " + newDistance + "cm");

  if (withinRangeStatuses[sensorNumber] != isWithinRange) {
    if (not Firebase.RTDB.setBool(
          &fbdo, String("lotStatuses/" + lotNumber), isWithinRange
        )) {
      Serial.println("WRITE FAILED: " + fbdo.errorReason());
    }
  }

  withinRangeStatuses[sensorNumber] = isWithinRange;

  delay(50);
}

void setup() {
  Serial.begin(115200);

  initWifi();

  // Firebase setup
  config.api_key = FIREBASE_API_KEY;
  config.database_url = FIREBASE_DATABASE_URL;

  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_PASSWORD;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task
  config.token_status_callback = tokenStatusCallback;

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  Firebase.begin(&config, &auth);

  // Get and print user UID
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  uid = auth.token.uid.c_str();
  Serial.print("User UID: " + uid + "\n");

  // Pin setup 1
  pinMode(OCCUPIED_LED_1_PIN, OUTPUT);
  pinMode(RESERVED_LED_1_PIN, OUTPUT);
  pinMode(ECHO_1_PIN, INPUT);
  pinMode(TRIG_1_PIN, OUTPUT);

  // Pin setup 2
  pinMode(OCCUPIED_LED_2_PIN, OUTPUT);
  pinMode(RESERVED_LED_2_PIN, OUTPUT);
  pinMode(ECHO_2_PIN, INPUT);
  pinMode(TRIG_2_PIN, OUTPUT);
}

void loop() {
  if (Firebase.ready() && millis() - sendDataPrevMillis > POLLING_RATE_MS || sendDataPrevMillis == 0) {
    sendDataPrevMillis = millis();

    getSensorInfo(
      0,
      "1A-001",
      TRIG_1_PIN,
      ECHO_1_PIN,
      OCCUPIED_LED_1_PIN,
      RESERVED_LED_1_PIN
    );

    getSensorInfo(
      1,
      "1A-002",
      TRIG_2_PIN,
      ECHO_2_PIN,
      OCCUPIED_LED_2_PIN,
      RESERVED_LED_2_PIN
    );

    Serial.println("----------------------------------------");
  }
}
