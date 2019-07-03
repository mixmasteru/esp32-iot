// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include "secrets.h"
#include "DHT.h"
#include <WiFi.h>
#include <AWS_IOT.h>

#define DHTPIN 2     // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

AWS_IOT iot;   // AWS_IOT instance

char HOST_ADDRESS[]="aq5mzcukulkpf-ats.iot.eu-central-1.amazonaws.com";
char CLIENT_ID[]= "iot_all";
char TOPIC_NAME[]= "garten/ghaus";
int status = WL_IDLE_STATUS;
int tick=0,msgCount=0,msgReceived = 0;
char payload[512];
char rcvdPayload[512];


void setup() {
  Serial.begin(9600);
  Serial.println(F("DHTxx test!"));
  delay(2000);

      while (status != WL_CONNECTED)
      {
          Serial.print("Attempting to connect to SSID: ");
          Serial.println(WIFI_SSID);
          // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
          status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

          // wait 5 seconds for connection:
          delay(5000);
      }

      Serial.println("Connected to wifi");

      if(iot.connect(HOST_ADDRESS,CLIENT_ID)== 0) // Connect to AWS using Host Address and Cliend ID
      {
          Serial.println("Connected to AWS");
          delay(1000);
      }
      else
      {
          Serial.println("AWS connection failed, Check the HOST Address");
          while(1);
      }

      delay(2000);

  dht.begin(); //Initialize the DHT11 sensor
}

void loop() {
  // Wait a few seconds between measurements.
  delay(2000);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));

  sprintf(payload,"Humidity:%f  Temperature:%f'C",h,t); // Create the payload for publishing

  if(iot.publish(TOPIC_NAME,payload) == 0)   // Publish the message(Temp and humidity)
  {
      Serial.print("Publish Message:");
      Serial.println(payload);
  }
  else
  {
      Serial.println("Publish failed");
  }
  // publish the temp and humidity every 5 seconds.
  vTaskDelay(5000 / portTICK_RATE_MS);
}
