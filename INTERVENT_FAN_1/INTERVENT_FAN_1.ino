
/**
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: k_suwatchai@hotmail.com
 *
 * Github: https://github.com/mobizt/Firebase-ESP-Client
 *
 * Copyright (c) 2023 mobizt
 *
 */

/** This example will show how to authenticate as a user with Email and password.
 *
 * You need to enable Email/Password provider.
 * In Firebase console, select Authentication, select Sign-in method tab,
 * under the Sign-in providers list, enable Email/Password provider.
 *
 * From this example, the user will be granted to access the specific location that matches
 * the user uid.
 *
 * This example will modify the database rules to set up the security rule which need to
 * guard the unauthorized access with the user Email.
 */
#include <DHT.h>
#include <Arduino.h>
#include <Adafruit_Sensor.h>
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

//Timer Variables

bool hot;
bool cool;
bool energy;
bool SAB_1;
bool SAB_2;
float humi = 404.111;
float temp = 404.111;
int outputpin = A0;
unsigned long dataMillis = 0; 

#define INDICATOR 2
#define RELAY1 4
#define RELAY2 12
#define DHTPIN 5     // Digital pin connected to the DHT sensor

// Uncomment the type of sensor in use:
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21 

DHT dht(DHTPIN, DHTTYPE);

void setup()
{
    pinMode(INDICATOR,OUTPUT);
    pinMode(RELAY1,OUTPUT);
    pinMode(RELAY2,OUTPUT);
    
    Serial.begin(115200);
    dht.begin();
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
/////////////////INTERRUPTS//////////////////////////////////////////////////////////////////////////
 
///////////////////////////////////////////////////////////////////////////////////////////
}

void loop()
{
// Firebase.ready() should be called repeatedly to handle authentication tasks. 
    
      if (millis() - dataMillis > 5000 && Firebase.ready())
       {

///////////////////////////////FIREBASE FIRESTORE CODE FOR DHT////////////////////////////////////////////////////////////////
          //READ TEMPERATURE
          float newT = dht.readTemperature();
          if(isnan(newT)) {
              Serial.println("Failed to read from DHT sensor!");
            }else{
              temp = newT;
              Serial.println(temp);
            }
/////////////////////////////////////////////////////////////////////////////////////
          // Read Humidity
          float newH = dht.readHumidity();
          if(isnan(newH)) {
            Serial.println("Failed to read from DHT sensor!");
            }else{
              humi = newH;
              Serial.println(humi);
            }
//////////////////////////////////////////////////////////////////////////////////////////////     
          Serial.print("Commit a document (set server value, update document)... ");
          std::vector<struct fb_esp_firestore_document_write_t> writes;
          struct fb_esp_firestore_document_write_t transform_write;
          fb_esp_firestore_document_write_type_update,
          transform_write.type = fb_esp_firestore_document_write_type_transform;
          //////////////////////////////
          struct fb_esp_firestore_document_write_t update_write;
          update_write.type = fb_esp_firestore_document_write_type_update;
          //Set the document content to write (transform)
          FirebaseJson content;
          String documentPath = "intervent/3JYHBMBtkRgpezba819SXdSxGEj1" ;
          content.set("fields/Temperature/doubleValue", String(temp).c_str());
          content.set("fields/Humidity/doubleValue", String(humi).c_str());
          //content.set("fields/status/booleanValue", count % 2 == 0);         
          //Set the update document content
          update_write.update_document_content = content.raw();
          //Set the update document path
          update_write.update_document_path = documentPath.c_str();
          update_write.update_masks = "Temperature,Humidity";

          update_write.current_document.exists = "true";
          writes.push_back(update_write);
          
          if (Firebase.Firestore.commitDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, writes /* dynamic array of fb_esp_firestore_document_write_t */, "" /* transaction */))
              Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
          else
              Serial.println(fbdo.errorReason());
///////////////////////////////END////////////////////////////////////////////////////////////////////////

          Firebase.ready();
        
          dataMillis = millis();
          String path = "/Intervent/";
          path += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
          path += "/SAB_1";
          Serial.printf("Get bool... %s\n", Firebase.RTDB.getBool(&fbdo, path) ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());

      if (fbdo.dataType() == "boolean")
          SAB_1 = fbdo.boolData();
          Serial.println(SAB_1 && "Firebase");
        
          Setstate();

          ////////////////////////////////TEMP DATA////////////////////////////////////////////////////////////////////////////
///////////COOL///////////////////
          Firebase.ready();
        
          String path1 = "/Intervent/";
          path1 += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
          path1 += "/cool";
          Serial.printf("Get COOL state... %s\n", Firebase.RTDB.getBool(&fbdo, path1) ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());

      if (fbdo.dataType() == "boolean")
          cool = fbdo.boolData();
          Serial.println(cool && "Firebase");

////////////HOT///////////////////
          String path2 = "/Intervent/";
          path2 += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
          path2 += "/hot";
          Serial.printf("Get HOT state... %s\n", Firebase.RTDB.getBool(&fbdo, path2) ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());

      if (fbdo.dataType() == "boolean")
          hot = fbdo.boolData();
          Serial.println(hot && "Firebase");
          
//////////ENERGY EFFICIENT///////////////
          String path3 = "/Intervent/";
          path3 += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
          path3 += "/energy";
          Serial.printf("Get ENERGY state... %s\n", Firebase.RTDB.getBool(&fbdo, path3) ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());

      if (fbdo.dataType() == "boolean")
          energy = fbdo.boolData();
          Serial.println(energy && "Firebase");
       }

      Firebase.ready();
      Handlefan(); 
      Serial.printf("Temperature: ");
      Serial.println(temp);
      Serial.printf("Humidity: "); 
      Serial.println(humi); 


     if(cool == 1){
        tempControl1();
        }
        
     if(hot == 1){
        tempControl3();
        }

     if(energy == 1){
        tempControl2();
        }       
}



void Setstate(){
       Firebase.ready();
   // Stand alone GS state code
      if(SAB_1 == 1){
        digitalWrite(INDICATOR, LOW); //ON, Low is ON...
        digitalWrite(RELAY1,LOW);
      }
  
      if(SAB_1 == 0){
        digitalWrite(INDICATOR, HIGH); //OFF, High is Off...
        digitalWrite(RELAY1,HIGH);
        }
}

void Handlefan(){ 
      Firebase.ready(); 
///////////IF SAB_1/RELAY1 IS ON, wait 20secs and switch SAB_2/RELAY2 ON////////////////
     if(SAB_1 == 1) {
        delay(20000);
        SAB_2 == 1;
        digitalWrite(RELAY2,LOW);
       }
/////////////////////////END///////////////////////////////////////////////////////////////
      
///////////IF SAB_1/RELAY1 IS OFF, wait 20secs and switch SAB_2/RELAY2 OFF////////////////
     if(SAB_1 == 0) {
        delay(10000);
        SAB_2 == 0;
        digitalWrite(RELAY2,HIGH);
       }
}

/////////////////////////////////END///////////////////////////////////////////////////////////

///////////////////////////COOL/////////////////////////
void tempControl1(){
      if(temp > 25.00){
        SAB_1 == 1;
        Firebase.ready();
        String path = "/Intervent/";
        path += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
        path += "/SAB_1";
        Serial.printf("Temp too HIGH, setting fan to... %s\n", Firebase.RTDB.setBool(&fbdo, path, true) ? "ok" : fbdo.errorReason().c_str());
        }

      if(temp < 15.00){
        SAB_1 == 0;
        Firebase.ready();
        String path = "/Intervent/";
        path += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
        path += "/SAB_1";
        Serial.printf("Temp too LOW, setting fan to... %s\n", Firebase.RTDB.setBool(&fbdo, path, false) ? "ok" : fbdo.errorReason().c_str());
        }
  }

////////////////////////ENERGY EFFICIENT////////////////////////////
void tempControl2(){
      if(temp > 35.00){
        SAB_1 == 1;
        Firebase.ready();
        String path = "/Intervent/";
        path += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
        path += "/SAB_1";
        Serial.printf("Temp too HIGH, setting fan to... %s\n", Firebase.RTDB.setBool(&fbdo, path, true) ? "ok" : fbdo.errorReason().c_str());
        }

      if(temp < 18.00){
        SAB_1 == 0;
        Firebase.ready();
        String path = "/Intervent/";
        path += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
        path += "/SAB_1";
        Serial.printf("Temp too LOW, setting fan to... %s\n", Firebase.RTDB.setBool(&fbdo, path, false) ? "ok" : fbdo.errorReason().c_str());
        }
  }

////////////////////////HOT////////////////////////////////
void tempControl3(){
      if(temp > 34.00){
        SAB_1 == 1;
        Firebase.ready();
        String path = "/Intervent/";
        path += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
        path += "/SAB_1";
        Serial.printf("Temp too HIGH, setting fan to... %s\n", Firebase.RTDB.setBool(&fbdo, path, true) ? "ok" : fbdo.errorReason().c_str());
        }

      if(temp < 24.00){
        SAB_1 == 0;
        Firebase.ready();
        String path = "/Intervent/";
        path += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
        path += "/SAB_1";
        Serial.printf("Temp too LOW, setting fan to... %s\n", Firebase.RTDB.setBool(&fbdo, path, false) ? "ok" : fbdo.errorReason().c_str());
        }
  }
