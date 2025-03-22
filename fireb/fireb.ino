#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <DHT.h>
// #include <NTPClient.h>
// #include <WiFiUdp.h>

// Wi-Fi credentials
#define WIFI_SSID "iQOOZ6"
#define WIFI_PASSWORD "!@#$%^&*()-w"

// Firebase API Key and URL
#define API_KEY "AIzaSyCkswTVYH0Kfw4fEIYnht-BdaSNRB--LR8"
#define DATABASE_URL "irrigreat-9ab11-default-rtdb.firebaseio.com"

// DHT Sensor configuration
#define DHT_SENSOR_PIN 4
#define DHT_SENSOR_TYPE DHT11
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// // NTP configuration
// WiFiUDP ntpUDP;
// NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // GMT+5:30 (India Standard Time)

// Variables
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

void setup() {
  Serial.begin(115200);

  // Initialize Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nConnected to Wi-Fi!");

  // Initialize Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase initialized successfully.");
    signupOK = true;
  } else {
    Serial.printf("Firebase sign-up error: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Initialize DHT sensor
  dht_sensor.begin();

  // Initialize NTP
  //timeClient.begin();
}

void loop() {
  // Ensure NTP is synchronized
  // timeClient.update();

  // // Get current date and time using epoch
  // unsigned long epochTime = timeClient.getEpochTime();
  // String currentDate = getFormattedDate(epochTime); // Format epoch into a readable date
  // String currentTime = timeClient.getFormattedTime(); // HH:MM:SS

  // Read sensor data
  float temperature = dht_sensor.readTemperature();
  float humidity = dht_sensor.readHumidity();

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // Unique path using timestamp
    String path = "DHT_11/" + String(millis());

    // Add Temperature
    if (Firebase.RTDB.setFloat(&fbdo, path + "/Temperature", temperature)) {
      Serial.print("Temperature uploaded: ");
      Serial.println(temperature);
    } else {
      Serial.println("Failed to upload temperature");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    // Add Humidity
    if (Firebase.RTDB.setFloat(&fbdo, path + "/Humidity", humidity)) {
      Serial.print("Humidity uploaded: ");
      Serial.println(humidity);
    } else {
      Serial.println("Failed to upload humidity");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    // // Add Date
    // if (Firebase.RTDB.setString(&fbdo, path + "/Date", currentDate)) {
    //   Serial.print("Date uploaded: ");
    //   Serial.println(currentDate);
    // } else {
    //   Serial.println("Failed to upload date");
    //   Serial.println("REASON: " + fbdo.errorReason());
    // }

    // // Add Time
    // if (Firebase.RTDB.setString(&fbdo, path + "/Time", currentTime)) {
    //   Serial.print("Time uploaded: ");
    //   Serial.println(currentTime);
    // } else {
    //   Serial.println("Failed to upload time");
    //   Serial.println("REASON: " + fbdo.errorReason());
    // }

    delay(60000); // Wait 1 second
  }
}

// Helper function to convert epoch time into date format YYYY-MM-DD
String getFormattedDate(unsigned long epochTime) {
  int year = 1970 + epochTime / 31556926; // Seconds in a year
  int month = (epochTime % 31556926) / 2629743 + 1; // Seconds in a month (approx.)
  int day = ((epochTime % 31556926) % 2629743) / 86400; // Seconds in a day

  char formattedDate[11];
  sprintf(formattedDate, "%04d-%02d-%02d", year, month, day);
  return String(formattedDate);
}
