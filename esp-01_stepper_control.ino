/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>


#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

int stepsPerAngle;
int angle=0;
int uSecondDelay= 800;
/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "Nayatel-Smart 1"
#define WLAN_PASS       "temp123sms"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "programmer5"
#define AIO_KEY         "16d6fbd3ac9944e4823cdae46e5348e6"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe stepDelay = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/stepDelay");
Adafruit_MQTT_Subscribe stepperAngle = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/stepperAngle");
Adafruit_MQTT_Subscribe Direction = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/Direction");
Adafruit_MQTT_Publish photocell = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/espIP");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();
  StaticJsonDocument<200> stepper;

void setup() {
  Serial.begin(115200);

  delay(10);

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  stepper["angle"] = "90";
  stepper["dir"] = "clockwise";
  stepper["usDelay"] = 800;

// Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&stepDelay);
  mqtt.subscribe(&Direction);
  mqtt.subscribe(&stepperAngle);
  
 }

uint32_t x=0;

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &Direction) {
       if (strcmp((char *)Direction.lastread, "clockwise") == 0) {
          stepper["dir"] = "clockwise";
      }
      else
      {
          stepper["dir"] = "anticlockwise";       
      }
      
    }
    if (subscription == &stepperAngle) {
      stepper["angle"]=atoi((char*)stepperAngle.lastread);      
      serializeJson(stepper, Serial);
      Serial.println();
    }

    if (subscription == &stepDelay) {
      stepper["usDelay"] = atoi((char*)stepDelay.lastread); 
     
    }

  }
  x++;
  if(x%10==0)
    photocell.publish(x++);
  
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

//  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
