#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ESP32Servo.h>
#include "time.h"

// Wi-Fi and AWS IoT Core credentials
#define WIFI_SSID "iQOOZ6"
#define WIFI_PASSWORD "!@#$%^&*()-w"
#define AWS_IOT_ENDPOINT "a2cwkf2smvixie-ats.iot.eu-north-1.amazonaws.com"

// MQTT topics
#define TEMPERATURE_TOPIC "esp32/dht11/temperature"
#define HUMIDITY_TOPIC "esp32/dht11/humidity"
#define GAS_TOPIC "esp32/dht11/gasLevel"
#define SERVO_CONTROL_TOPIC "servo/control"

// DHT sensor configuration
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// MQ6 gas sensor configuration
#define MQ6_PIN 36

// Servo motor configuration
#define SERVO_PIN 14
Servo servo;

// NTP configuration
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800; // GMT+5:30 (India Standard Time)
const int daylightOffset_sec = 0;

// Certificates for AWS IoT Core
const char* rootCA ="-----BEGIN CERTIFICATE-----\n"\
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"\
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"\
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"\
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"\
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"\
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"\
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"\
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"\
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"\
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"\
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"\
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"\
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"\
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"\
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"\
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"\
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"\
"rqXRfboQnoZsG4q5WTP468SQvvG5\n"\
"-----END CERTIFICATE-----";

const char* deviceCert ="-----BEGIN CERTIFICATE-----\n"\
"MIIDWTCCAkGgAwIBAgIUcxjrMoJlfY56zk6JbRhRJZ3RXrYwDQYJKoZIhvcNAQEL\n"\
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n"\
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI1MDMyMDE4NDIw\n"\
"MloXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n"\
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANLU9klj0zAQTvgMOSDu\n"\
"KQxL29/AuUFQFcG9hii0QQ1DdrzdaVM6XXRrknSjUWPEvvnCbqEINBb4Q4tEVfMS\n"\
"lww6w4Vl73p9AqdEdaIqySIUQ9eENSYly2FUkrcr/g8uXDUDOxHALRAd6CYpkybL\n"\
"L3NAQC9UXRnEHPRNIW7H19W+Y1CQs9Hb0/XUCW7GCH7sXAh/0QvkCD2OLIF2OeBM\n"\
"WLiXElZOUY5Bp2/KGpjIQHnr5gt9tdUDFaxngN8G/jLkryD3nuMdZXcLIZckIkMC\n"\
"SvOaJ2vQ+UOBgdKGvSJEK4cvFvQtvqnVszFBQ1m2frTtcijusZDrrJtCivSn0wGS\n"\
"wgECAwEAAaNgMF4wHwYDVR0jBBgwFoAUY7PFHiVkkmVx0TOVyK0a1e16MCkwHQYD\n"\
"VR0OBBYEFB/qY2rwd4c+tIAfeULNjIE4nAaDMAwGA1UdEwEB/wQCMAAwDgYDVR0P\n"\
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBkEMsaSEVE2AxYhDAU6HJqCzod\n"\
"rqS8V5B9pZolVaTpeRAj62qvPf01t7MeRzgw8f1fvHLK6eq/00xmIFpHB4N80I8p\n"\
"e9yp32U6kHX8nes/wVubaT8Uz5TXEKOyBGz7/Ea0oQRj/JQeptQhDqLe7dwSx0ik\n"\
"o8GK/IEgfo0nZoaG2MEpB9UYs9N0V1kelirqn54B5L3/B0NY26Y+NaL+LqmswY81\n"\
"oVVNnzS7FfWuEwCrxamPGOdkt5j85tfBsLxYl7c/LDsWb5yxp8f5yF2pRGGx9t/q\n"\
"tfSwbSfGkzaJ/s1zNAXzYs26X2VjCphAG5aXqCJwuvDrzoo6kQrhkF2BO123\n"\
"-----END CERTIFICATE-----"; 

const char* privateKey = "-----BEGIN RSA PRIVATE KEY-----\n"\
"MIIEpAIBAAKCAQEA0tT2SWPTMBBO+Aw5IO4pDEvb38C5QVAVwb2GKLRBDUN2vN1p\n"\
"UzpddGuSdKNRY8S++cJuoQg0FvhDi0RV8xKXDDrDhWXven0Cp0R1oirJIhRD14Q1\n"\
"JiXLYVSStyv+Dy5cNQM7EcAtEB3oJimTJssvc0BAL1RdGcQc9E0hbsfX1b5jUJCz\n"\
"0dvT9dQJbsYIfuxcCH/RC+QIPY4sgXY54ExYuJcSVk5RjkGnb8oamMhAeevmC321\n"\
"1QMVrGeA3wb+MuSvIPee4x1ldwshlyQiQwJK85ona9D5Q4GB0oa9IkQrhy8W9C2+\n"\
"qdWzMUFDWbZ+tO1yKO6xkOusm0KK9KfTAZLCAQIDAQABAoIBAAdHsUVxe/5h4cgw\n"\
"69wWPTaee/Yq+nql4EpzV+mZ59EkbjoxTwy/rDBz8neymAHg7lNDgnxvAxVz9ykc\n"\
"7/bXwn16nfvCPFW3cVsDQyiZX56coocTmp45md8/g11t2HT/vdFazjRbtXKkBbsj\n"\
"nmAtjfLl2hGIUTqtie5pp2ji37ISs+Fx1YnyQyzcxH9WNnCRioHR0/xcGVsV6S4/\n"\
"gdSXCdYe6GYvptNYXrOvO133mmZ4VsaXyAbSByQ9hdMxyw2NsFeOE4cvsLrsMu23\n"\
"vcB5CCd9qPbXKa0nn82ngM2kFPJdDzlx6zYf+JH7j18XFBi00NZHCKkSbZqlVnM3\n"\
"2Hw7iw0CgYEA66p64rmLpCvkbxvgIA41NXIK5lYTOiCIopfSXzeE71xDUBCCpwiN\n"\
"pmjUyGc7Snsi4uA5bekbhqFS8EjycpSVtRyaTHNogGMFngAf0OHJWeI3fEofxPrq\n"\
"aSjWaeu6xWqMSnn9gWM7zjHoizOUn/E97UbxGz6/PGgfkMZhoWenl1MCgYEA5QXv\n"\
"BF10X2ZkpUj519X2t8xhB2wphtMCHWiZxvb2bEisKcVH4AfGGTrG0jpDul7bbH7N\n"\
"bjPYZ4hYxXJ9RTdIFn8riblwWd263nAjtpoJBOsro37kZBliKLHpg50nQPTtu8p9\n"\
"isLKdZNMWUcAI1eMgJLRec0+H06+2f2Y68t2utsCgYEAx2wyJN3iXwCNg8BQ/o9H\n"\
"U3y34IaCKrQRK5XR/aUbpvmqNHxkMzi3tVQMi7FfqP1eXiJkNrAGDP1DE/aGw5Mc\n"\
"xG/52ivnCI3C2Rs90SLfomIdld2b2LKD3o0iM+u41wn8waFNdkok962gHPt/D4Gz\n"\
"Z7CaovWMGm/6Fibpww3V8eUCgYEAmb5I/RorlBtttxNtPXpSXIJkJJJ0siCWy907\n"\
"VjjVpk2G3aelH3y4fJlbfSdF7ifdj1D0YWWGgcIjLbE07TE2QHvwY01MYdeaqdaH\n"\
"8v4mzdl9ew0lDxmHLqB4jTUDc/S4XxeI/YzUUuemGkyyiAcut+VXoDeq22OGdVpr\n"\
"vObidn8CgYBILP+knX/xAPQBmRpFTGh39hNSB6AAqsQ6VWH8YBISu/190OCN4ToF\n"\
"7bS/nYM+2ic28wXVHkbvVzY5XHXT45IIea+sRaPnvEDxJ1h8Fh+F9ynz1J2YdJlm\n"\
"2fD4mRMk1yJoLDPYB/COxl/z9JBjBp2IVARwhFw2VxfJGNu6VbC0Fw==\n"\
"-----END RSA PRIVATE KEY-----";
// Initialize Wi-Fi and MQTT clients
// Initialize Wi-Fi and MQTT clients
WiFiClientSecure net;
PubSubClient client(net);

void connectToWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
}

void connectToAWS() {
  Serial.print("Connecting to AWS IoT Core...");
  while (!client.connected()) {
    if (client.connect("ESP32-DHT11")) {  // Unique client ID
      Serial.println("Connected to AWS IoT Core!");
      client.subscribe(SERVO_CONTROL_TOPIC); // Subscribe to servo control topic
    } else {
      Serial.print("Failed to connect. State: ");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

void setupNTP() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("NTP setup completed. Synchronizing time...");
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  } else {
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  }
}

time_t getTimestamp() {
  time_t now = time(nullptr); // Get the current time as Unix timestamp
  if (now == ((time_t)-1)) {
    Serial.println("Failed to obtain time");
    return 0; // Return 0 if time sync fails
  }
  return now; // Return Unix timestamp as a number
}

// MQTT callback to handle messages
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Received message on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(message);

  // Handle servo control
  if (String(topic) == SERVO_CONTROL_TOPIC && message == "rotate") {
    servo.write(90);
    delay(1000);
    servo.write(0);
    Serial.println("Servo rotated in response to MQTT message");
  }
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize Wi-Fi
  connectToWiFi();

  // Initialize NTP
  setupNTP();

  // Initialize MQTT
  net.setCACert(rootCA);
  net.setCertificate(deviceCert);
  net.setPrivateKey(privateKey);
  client.setServer(AWS_IOT_ENDPOINT, 8883);
  client.setCallback(callback);
  connectToAWS();

  // Initialize sensors
  dht.begin();
  pinMode(MQ6_PIN, INPUT);

  // Initialize Servo motor
  servo.setPeriodHertz(50);
  servo.attach(SERVO_PIN, 500, 2400);
  servo.write(0);
}

void loop() {
  // Ensure MQTT connection
  if (!client.connected()) {
    connectToAWS();
  }
  client.loop();

  // Read sensor data
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float gasLevel = analogRead(MQ6_PIN);
  gasLevel = (4096 - gasLevel) / 4096 * 100; // Convert to percentage

  // Get the current timestamp as a number
  time_t timestamp = getTimestamp();
  delay(5000);
  // Validate and publish data
  if (!isnan(temperature)) {
    String tempPayload = "{\"timestamp\": " + String(timestamp) + ", \"temperature\": " + String(temperature) + "}";
    client.publish(TEMPERATURE_TOPIC, tempPayload.c_str());
    Serial.println("Published Temperature: " + tempPayload);
  }
  timestamp = getTimestamp();
  delay(5000);

  if (!isnan(humidity)) {
    String humPayload = "{\"timestamp\": " + String(timestamp) + ", \"humidity\": " + String(humidity) + "}";
    client.publish(HUMIDITY_TOPIC, humPayload.c_str());
    Serial.println("Published Humidity: " + humPayload);
  }
  timestamp = getTimestamp();
  delay(5000);
  if (!isnan(gasLevel)) {
    String gasPayload = "{\"timestamp\": " + String(timestamp) + ", \"gasLevel\": " + String(gasLevel) + "}";
    client.publish(GAS_TOPIC, gasPayload.c_str());
    Serial.println("Published Gas Level: " + gasPayload);
  }

  // Servo control logic
  if (humidity > 85.0 || gasLevel > 60.0) {
    servo.write(90);
    delay(10000);
    servo.write(0);
    Serial.println("Servo rotated due to high humidity or gas concentration");
  }

  delay(5000); // Wait 5 seconds before next reading
}
