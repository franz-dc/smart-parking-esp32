#include <Firebase_ESP_Client.h>
#include <WiFi.h>
#include <addons/RTDBHelper.h>
#include <addons/TokenHelper.h>
#include <secrets.h>
#include <string.h>

// set 1
#define LOT_NUMBER_1 "1A-001"
#define RED_LED_PIN_1 2
#define GREEN_LED_PIN_1 4
#define BLUE_LED_PIN_1 16
#define ECHO_PIN_1 25
#define TRIG_PIN_1 26

// set 2
#define LOT_NUMBER_2 "1A-002"
#define RED_LED_PIN_2 21
#define GREEN_LED_PIN_2 22
#define BLUE_LED_PIN_2 23
#define ECHO_PIN_2 32
#define TRIG_PIN_2 33

#define SEND_POLLING_RATE_MS 1000
#define RECEIVE_POLLING_RATE_MS 3000
#define MAX_DISTANCE_CM 20

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

// variable declarations
bool isReserved = false;
bool isWithinRange = false;
bool occupiedStatuses[] = {false, false};
bool reservedStatuses[] = {false, false};
short ledPins[][3] = {
  {RED_LED_PIN_1, GREEN_LED_PIN_1, BLUE_LED_PIN_1},
  {RED_LED_PIN_2, GREEN_LED_PIN_2, BLUE_LED_PIN_2}};
unsigned long sendDataPrevMillis = 0;
unsigned long receiveDataPrevMillis = 0;
float newDistance = 0;
String uid = "";
String lotNumbers[] = {LOT_NUMBER_1, LOT_NUMBER_2};

void initWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.print("\nConnected with IP: " + WiFi.localIP());
  Serial.println();
}

void setLed(short ledPins[3], short red, short green, short blue) {
  analogWrite(ledPins[0], red);
  analogWrite(ledPins[1], green);
  analogWrite(ledPins[2], blue);
}

void getSensorInfo(int sensorNumber, short trigPin, short echoPin) {
  // generate 10-us pulse to TRIG pin
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // get sensor data: distance in cm
  newDistance = pulseIn(echoPin, HIGH) / 29.1 / 2;

  // check if distance is within range
  isWithinRange = newDistance <= MAX_DISTANCE_CM;

  // outputs

  if (isWithinRange) {
    setLed(ledPins[sensorNumber], 255, 0, 0);
  } else {
    if (!reservedStatuses[sensorNumber]) {
      setLed(ledPins[sensorNumber], 0, 255, 0);
    }
  }

  Serial.println("Lot " + lotNumbers[sensorNumber] + ": " + newDistance + "cm");

  if (occupiedStatuses[sensorNumber] != isWithinRange) {
    if (not Firebase.RTDB.setBool(
          &fbdo,
          String("lotStatuses/" + lotNumbers[sensorNumber]),
          isWithinRange
        )) {
      Serial.println("WRITE FAILED: " + fbdo.errorReason());
    }
  }

  occupiedStatuses[sensorNumber] = isWithinRange;

  delay(50);
}

void getReservationStatus(int sensorNumber) {
  if (Firebase.RTDB.getBool(
        &fbdo, String("lotReservationStatuses/" + lotNumbers[sensorNumber])
      )) {
    if (fbdo.dataType() == "boolean") {
      reservedStatuses[sensorNumber] = fbdo.boolData();
      if (fbdo.boolData() && !occupiedStatuses[sensorNumber]) {
        setLed(ledPins[sensorNumber], 0, 0, 255);
      }
    }
  } else {
    Serial.println("READ FAILED: " + fbdo.errorReason());
  }
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
  pinMode(RED_LED_PIN_1, OUTPUT);
  pinMode(GREEN_LED_PIN_1, OUTPUT);
  pinMode(BLUE_LED_PIN_1, OUTPUT);
  pinMode(ECHO_PIN_1, INPUT);
  pinMode(TRIG_PIN_1, OUTPUT);

  // Pin setup 2
  pinMode(RED_LED_PIN_2, OUTPUT);
  pinMode(GREEN_LED_PIN_2, OUTPUT);
  pinMode(BLUE_LED_PIN_2, OUTPUT);
  pinMode(ECHO_PIN_2, INPUT);
  pinMode(TRIG_PIN_2, OUTPUT);
}

void loop() {
  if (Firebase.ready()) {
    if (millis() - sendDataPrevMillis > SEND_POLLING_RATE_MS || sendDataPrevMillis == 0) {
      sendDataPrevMillis = millis();

      getSensorInfo(0, TRIG_PIN_1, ECHO_PIN_1);
      getSensorInfo(1, TRIG_PIN_2, ECHO_PIN_2);

      Serial.println("----------------------------------------");
    }

    if (millis() - receiveDataPrevMillis > RECEIVE_POLLING_RATE_MS || receiveDataPrevMillis == 0) {
      receiveDataPrevMillis = millis();

      getReservationStatus(0);
      getReservationStatus(1);
    }
  }
}
