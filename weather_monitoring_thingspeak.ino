#include <SFE_BMP180.h>  // Include BMP180 sensor library
#include <Wire.h>        // Include Wire library for I2C communication
#include <ESP8266WiFi.h> // Include ESP8266 WiFi library
#include "DHT.h"         // Include DHT sensor library

String apiKey = ""; // Thingspeak API Key
const char *ssid =  "";  // WiFi SSID
const char *pass =  "";      // WiFi password
const char* server = "api.thingspeak.com"; // Thingspeak server

DHT dht(D3, DHT11);   // DHT sensor on pin D3, using DHT11 type
SFE_BMP180 bmp;       // BMP180 sensor object
double T, P;          // Temperature (T) and Pressure (P) variables
char status;          // Status variable for BMP180 sensor
WiFiClient client;    // WiFi client for communication with Thingspeak

void setup() 
{
  Serial.begin(115200); // Start serial communication
  delay(10);
  bmp.begin();          // Initialize BMP180 sensor
  Wire.begin();         // Initialize I2C communication
  dht.begin();          // Initialize DHT sensor
  WiFi.begin(ssid, pass); // Connect to WiFi

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void loop() 
{
  // BMP180 sensor
  status =  bmp.startTemperature();  // Start BMP180 temperature measurement
  if (status != 0) 
  {
    delay(status);
    status = bmp.getTemperature(T);  // Get BMP180 temperature

    status = bmp.startPressure(3);   // Start BMP180 pressure measurement (oversampling 3 times)
    if (status != 0) 
    {
      delay(status);
      status = bmp.getPressure(P, T); // Get BMP180 pressure
      if (status != 0)
      {
        // Pressure obtained successfully
      }
    }
  }

  // DHT11 sensor
  float h = dht.readHumidity();    // Read humidity from DHT sensor
  float t = dht.readTemperature(); // Read temperature from DHT sensor

  if (isnan(h) || isnan(t)) 
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Rain sensor
  int r = analogRead(A0); // Read analog value from rain sensor
  r = map(r, 0, 1024, 0, 100); // Map analog value to a range of 0-100

  // Send data to Thingspeak
  if (client.connect(server, 80)) 
  {
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(t);
    postStr += "&field2=";
    postStr += String(h);
    postStr += "&field3=";
    postStr += String(P, 2);
    postStr += "&field4=";
    postStr += String(r);
    postStr += "\r\n\r\n\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n\n\n");
    client.print(postStr);

    // Print data to Serial Monitor
    Serial.print("Temperature: ");
    Serial.println(t);
    Serial.print("Humidity: ");
    Serial.println(h);
    Serial.print("Absolute Pressure: ");
    Serial.print(P, 2);
    Serial.println("mb");
    Serial.print("Rain: ");
    Serial.println(r);
  }
  client.stop(); // Disconnect from Thingspeak
  delay(1000);   // Wait for a second before sending data again
}
