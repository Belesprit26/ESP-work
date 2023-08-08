// DHT Temperature & Humidity Sensor
// Unified Sensor Library Example
// Written by Tony DiCola for Adafruit Industries
// Released under an MIT license.

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>
// Provide the token generation process info.
#include <addons/TokenHelper.h>
// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Buti"
#define WIFI_PASSWORD "buti12345"


/* 3. Define the user Email and password that already registerd or added in your project */
#define USER_EMAIL "martin@gmail.com"
#define USER_PASSWORD "123456"
#define USER_PATH "intervent"
#define UID "3JYHBMBtkRgpezba819SXdSxGEj1"

#define FIREBASE_PROJECT_ID "gsthethird-1f244"
#define API_KEY "AIzaSyCS0DTQhwu5ty8b38Gmmpr-ynVOl01lxOs"
#define DATABASE_SECRET "LtqAFx64eMGTjnOPTGiebVqgWyvR0emvpf2QMoOz"
#define DATABASE_URL "https://gsthethird-1f244-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 6. Define the Firebase Data object */
FirebaseData fbdo;
/* 7. Define the FirebaseAuth data for authentication data */
FirebaseAuth auth;
/* 8. Define the FirebaseConfig data for config data */
FirebaseConfig config;

float temp = 404.111;
float humi = 404.111;
unsigned long dataMillis = 0;
String down = "Something is wrong";
String up = "Something is wrong";
String perfect = "Temperature is perfect and Fans are on Standby";

#define DHTPIN 5     // Digital pin connected to the DHT sensor 
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.
#define INDICATOR 2
#define RELAY1 4
#define RELAY2 12

// Uncomment the type of sensor in use:
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

void setup() {

  pinMode(INDICATOR,OUTPUT);
  pinMode(RELAY1,OUTPUT);
  pinMode(RELAY2,OUTPUT);
  
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
      Serial.print(".");
      delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  
  // Initialize device.
  dht.begin();
  Serial.println(F("DHTxx Unified Sensor Example"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;

////////////WIFI CLIENT WORK////////////////////
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL */
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  String base_path = "/Intervent/";
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  /* Initialize the library with the Firebase authen and config */
  #if defined(ESP8266)
  // In ESP8266 required for BearSSL rx/tx buffer for large data handle, increase Rx size as needed.
  fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
  #endif

  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);
  String var = "$userId";
  String val = "($userId === auth.uid && auth.token.premium_account === true && auth.token.admin === true)";
  Firebase.RTDB.setReadWriteRules(&fbdo, base_path, var, val, val, DATABASE_SECRET);

}

void loop() { 
  Firebase.ready();
  // Delay between measurements.
  delay(delayMS);
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
    temp = event.temperature;
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    humi = event.relative_humidity;
  }
  
  Serial.print(F("This is temp now: "));
  Serial.println(temp);
  Serial.print(F("This is humi now: "));
  Serial.println(humi);

          if(temp > 23.00){
            digitalWrite(INDICATOR, LOW); //ON, Low is ON...
            digitalWrite(RELAY1,HIGH);
            digitalWrite(RELAY2,HIGH);
            up = "System is running";
            Serial.println(up);
         }else if(temp < 21.00){
            digitalWrite(INDICATOR, HIGH); //OFF, High is Off...
            digitalWrite(RELAY1,LOW);
            digitalWrite(RELAY2,LOW);
            down = "System is Off and on Standby";
            Serial.println(down);
         }else{
            Serial.println(perfect);
         } 
        

  if (millis() - dataMillis > 7000 && Firebase.ready())
       { 
         Firebase.ready();
         
         Serial.print("Commit a document (set server value, update document)... ");

 ///////////////////TEMPERATURE & HUMIDITY PART/////////////////////////////////////////////////////       

        std::vector<struct fb_esp_firestore_document_write_t> writes;
        struct fb_esp_firestore_document_write_t transform_write;
        fb_esp_firestore_document_write_type_update,transform_write.type = fb_esp_firestore_document_write_type_transform;
        //////////////////////////////
       //A write object that will be written to the document.
        struct fb_esp_firestore_document_write_t update_write;
        update_write.type = fb_esp_firestore_document_write_type_update;

        //Set the document content to write (transform)

        FirebaseJson content;
        String documentPath = "intervent/3JYHBMBtkRgpezba819SXdSxGEj1";

        content.set("fields/Temperature/doubleValue", String(temp).c_str());
        content.set("fields/Humidity/doubleValue", String(humi).c_str());
        
        if(temp > 23.00){
            content.set("fields/State1/stringValue", String(up).c_str());
        }else if(temp < 21.00){
            content.set("fields/State1/stringValue", String(down).c_str());
        }else{
            content.set("fields/State1/stringValue", String(perfect).c_str());
        }

        //Set the update document content
        update_write.update_document_content = content.raw();

        //Set the update document path
        update_write.update_document_path = documentPath.c_str();

        //Set the document mask field paths that will be updated
        //Use comma to separate between the field paths
        update_write.update_masks = "Temperature,Humidity,State1";


        //Set the precondition write on the document.
        //The write will fail if this is set and not met by the target document.
        //Th properties for update_write.current_document should set only one from exists or update_time
        update_write.current_document.exists = "true";
        


        //Add a write object to a write array.
        writes.push_back(update_write);

        if (Firebase.Firestore.commitDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, writes /* dynamic array of fb_esp_firestore_document_write_t */, "" /* transaction */))
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
        else
            Serial.println(fbdo.errorReason());  

       }
      
}
