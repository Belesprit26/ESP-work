/**
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt/Firebase-ESP-Client
 * 
 * Copyright (c) 2022 mobizt
 *
*/

//This example shows how to set the server value (timestamp) to document field, update and dellete the document. This operation required Email/password, custom or OAUth2.0 authentication.

//#if defined(ESP32)
//#include <WiFi.h>
//#elif defined(ESP8266)
#include <ESP8266WiFi.h>
//#endif

#include <Adafruit_Sensor.h>
#include <Hash.h>
#include <time.h>

#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include <addons/TokenHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Buti"
#define WIFI_PASSWORD "buti12345"

/* 2. Define the API Key */
#define API_KEY "AIzaSyCS0DTQhwu5ty8b38Gmmpr-ynVOl01lxOs"
#define DATABASE_URL "https://gsthethird-1f244-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define DATABASE_SECRET "LtqAFx64eMGTjnOPTGiebVqgWyvR0emvpf2QMoOz"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "gsthethird-1f244"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "butizwide@gmail.com"
#define USER_PASSWORD "123456"
#define USER_PATH "users"
#define UID "OTNN3RGqxCVTeDOsWcBz7fHsIG23"


//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long dataMillis = 0;
unsigned long dataMillis2 = 0;

float temp = 11.11;
float humi = 0.0;
bool Switch_1 = 0;


int outputpin= A0;
const int indicator = 2;
const int relay1 = 5;
const int relay2 = 4;
int timer2 = 0;
int timer4 = 0;
int timer13 = 0;
int timer15 = 0;
int timer19 = 0;

int timezone = 2 * 3600;
int dst = 0;

void setup()
{

    pinMode(indicator,OUTPUT);
    pinMode(relay1,OUTPUT);
    pinMode(relay2,OUTPUT);
    
    Serial.begin(115200);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
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

    config.database_url = DATABASE_URL;

    fbdo.setResponseSize(4096);
    String base_path = "users/WBSR30TEa7NaHy0ZwF8FuHvYCr13";

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
    config.max_token_generation_retry = 5;

    #if defined(ESP8266)
    // In ESP8266 required for BearSSL rx/tx buffer for large data handle, increase Rx size as needed.
    fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */, 2048 /* Tx buffer size in bytes from 512 - 16384 */);
    #endif

    fbdo.setResponseSize(2048);

    Firebase.begin(&config, &auth);
    
    Firebase.reconnectWiFi(true);

    String var = "$userId";
    String val = "($userId === auth.uid && auth.token.premium_account === true && auth.token.admin === true)";
    Firebase.RTDB.setReadWriteRules(&fbdo, base_path, var, val, val, DATABASE_SECRET);
   


    configTime(timezone, dst, "pool.ntp.org","time.nist.gov");
    Serial.println("\nWaiting for Internet time");

    while(!time(nullptr)){
    Serial.print("*");
    }
    Serial.println("\nTime response....OK"); 

    time_t now = time(nullptr);
    struct tm* p_tm = localtime(&now);

}

///////// LOOP LEVEL

void loop()
{
       
    if (Firebase.ready() && (millis() - dataMillis > 5000 || dataMillis == 0)) 
    { 
      dataMillis = millis();
      
     /* time_t now = time(nullptr);
      struct tm* p_tm = localtime(&now);
      Serial.print(p_tm->tm_hour);
      Serial.print(":");
      Serial.println(p_tm->tm_min); */
    
      uploadTemp();

      switchState();

      alarmTwo();

      alarmFour();

      alarmOne();

      alarmThree();
      
      alarmSeven();
      
    }
      
}

void uploadTemp()
  {
    time_t now = time(nullptr);
              struct tm* p_tm = localtime(&now);
              Serial.print(p_tm->tm_hour);
              Serial.print(":");
              Serial.println(p_tm->tm_min);

              
    if (Firebase.ready() && (millis() - dataMillis2 > 15000 || dataMillis2 == 0))
        {
          dataMillis2 = millis();

              /*time_t now = time(nullptr);
              struct tm* p_tm = localtime(&now);
              Serial.print(p_tm->tm_hour);
              Serial.print(":");
              Serial.println(p_tm->tm_min);
              */
      
              int rawvoltage= analogRead(outputpin);
              float millivolts= (rawvoltage/1039.0) * 2900;
              float temp= millivolts/10;
              Serial.print(temp);
              Serial.println(" degrees Celsius, ");
             
              
              Serial.print("Commit a document (set server value, update document)... ");
      
              //The dyamic array of write object fb_esp_firestore_document_write_t.
              std::vector<struct fb_esp_firestore_document_write_t> writes;
      
              //A write object that will be written to the document.
              struct fb_esp_firestore_document_write_t transform_write;
      
              //Set the write object write operation type.
              fb_esp_firestore_document_write_type_update,
              //fb_esp_firestore_document_write_type_delete,
              //fb_esp_firestore_document_write_type_transform
              transform_write.type = fb_esp_firestore_document_write_type_transform;
      
      
              //////////////////////////////
              //Add another write for update
      
              
              //A write object that will be written to the document.
              struct fb_esp_firestore_document_write_t update_write;
              //Set the write object write operation type.
              //fb_esp_firestore_document_write_type_update,
              //fb_esp_firestore_document_write_type_delete,
              //fb_esp_firestore_document_write_type_transform
              update_write.type = fb_esp_firestore_document_write_type_update;
              //Set the document content to write (transform)
              FirebaseJson content;
              String documentPath = "users/OTNN3RGqxCVTeDOsWcBz7fHsIG23" ;
              content.set("fields/Temperature/doubleValue", String(temp).c_str());
              //content.set("fields/Humidity_2/doubleValue", String(humi).c_str());
              //content.set("fields/status/booleanValue", count % 2 == 0);
              
              //Set the update document content
              update_write.update_document_content = content.raw();
              //Set the update document path
              update_write.update_document_path = documentPath.c_str();
              //Set the document mask field paths that will be updated
              //Use comma to separate between the field paths
              update_write.update_masks = "Temperature";
              //Set the precondition write on the document.
              //The write will fail if this is set and not met by the target document.
              //Th properties for update_write.current_document should set only one from exists or update_time
              update_write.current_document.exists = "true";
              //update_write.current_document.update_time = "2021-05-02T15:01:23Z";
              //Add a write object to a write array.
              writes.push_back(update_write);
              
      
              if (Firebase.Firestore.commitDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, writes /* dynamic array of fb_esp_firestore_document_write_t */, "" /* transaction */))
                  Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
              else
                  Serial.println(fbdo.errorReason());
        
          }            
    }




//GeyserSwitch on Button

void switchState(){

 String path3 = "/GeyserSwitch/";
        path3 += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
        path3 += "/Switch_1";
        Serial.printf("Get switch state... %s\n", Firebase.RTDB.getBool(&fbdo, path3) ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());

        if (fbdo.dataType() == "boolean")
            Switch_1 = fbdo.boolData();
            Serial.println(Switch_1 && "Firebase");

         // Stand alone GS state code
        if ((Switch_1 == 1)){
            digitalWrite(indicator, LOW); //ON, Low is ON...
            digitalWrite(relay1, LOW);
            digitalWrite(relay2,LOW);
          }

        if ((Switch_1 == 0)){
            digitalWrite(indicator, HIGH); //OFF, High is Off...
            digitalWrite(relay1, HIGH);
            digitalWrite(relay2,HIGH);
          }
  
}

void alarmTwo(){

        time_t now = time(nullptr);
        struct tm* p_tm = localtime(&now);

 String path3 = "/GeyserSwitch/";
        path3 += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
        path3 += "/Switch_1";
        Serial.printf("Get switch state... %s\n", Firebase.RTDB.getBool(&fbdo, path3) ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());

        
 String path2 = "/GeyserSwitch/";
        path2 += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
        path2 += "/Timer_1";
        Serial.printf("2AM timer... ", Firebase.RTDB.getBool(&fbdo, path2) ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());


        if (fbdo.dataType() == "boolean")
       // Serial.println(fbdo.boolData());
        timer2 = fbdo.boolData();
        Serial.print("Timer_1 state is = ");
        Serial.println(timer2);

         // If Alarm15 is Live

     if ((timer2 == 1) && (p_tm->tm_hour == 2) && (p_tm->tm_min == 0))
     {    
         digitalWrite(indicator,LOW);
         digitalWrite(relay1,LOW);
         digitalWrite(relay2,LOW);
         timer2 = 1;
         Switch_1 == 1;
         Serial.printf("Switch_1 to firebase... %s\n", Firebase.RTDB.setBool(&fbdo, path3, true) ? "ok" : fbdo.errorReason().c_str());
         }
         
     if ((timer2 == 1) && (p_tm->tm_hour == 3) && (p_tm->tm_min == 58))
     {
         digitalWrite(indicator, HIGH);
         digitalWrite(relay1,HIGH);
         digitalWrite(relay2,HIGH);
         timer2 = 1;
         Switch_1 == 0;
         Serial.printf("Switch_1 to firebase... %s\n", Firebase.RTDB.setBool(&fbdo, path3, false) ? "ok" : fbdo.errorReason().c_str());
         } 
       // 3PM alarm milli done
 }

void alarmFour(){

        time_t now = time(nullptr);
        struct tm* p_tm = localtime(&now);

 String path3 = "/GeyserSwitch/";
        path3 += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
        path3 += "/Switch_1";
        Serial.printf("Get switch state... %s\n", Firebase.RTDB.getBool(&fbdo, path3) ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());

        
 String path4 = "/GeyserSwitch/";
        path4 += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
        path4 += "/Timer_4";
        Serial.printf("4AM timer... ", Firebase.RTDB.getBool(&fbdo, path4) ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());


        if (fbdo.dataType() == "boolean")
       // Serial.println(fbdo.boolData());
        timer4 = fbdo.boolData();
        Serial.print("Timer_4 state is = ");
        Serial.println(timer4);

         // If Alarm15 is Live

     if ((timer4 == 1) && (p_tm->tm_hour == 4) && (p_tm->tm_min == 0))
     {    
         digitalWrite(indicator,LOW);
         digitalWrite(relay1,LOW);
         digitalWrite(relay2,LOW);
         timer4 = 1;
         Switch_1 == 1;
         Serial.printf("Switch_1 to firebase... %s\n", Firebase.RTDB.setBool(&fbdo, path3, true) ? "ok" : fbdo.errorReason().c_str());
         }
         
     if ((timer4 == 1) && (p_tm->tm_hour == 5) && (p_tm->tm_min == 58))
     {
         digitalWrite(indicator, HIGH);
         digitalWrite(relay1,HIGH);
         digitalWrite(relay2,HIGH);
         timer4 = 1;
         Switch_1 == 0;
         Serial.printf("Switch_1 to firebase... %s\n", Firebase.RTDB.setBool(&fbdo, path3, false) ? "ok" : fbdo.errorReason().c_str());
         } 
       // 3PM alarm milli done
 }

void alarmOne(){

        time_t now = time(nullptr);
        struct tm* p_tm = localtime(&now);

 String path3 = "/GeyserSwitch/";
        path3 += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
        path3 += "/Switch_1";
        Serial.printf("Get switch state... %s\n", Firebase.RTDB.getBool(&fbdo, path3) ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());

        
 String path13 = "/GeyserSwitch/";
        path13 += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
        path13 += "/Timer_13";
        Serial.printf("1PM timer... ", Firebase.RTDB.getBool(&fbdo, path13) ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());


        if (fbdo.dataType() == "boolean")
       // Serial.println(fbdo.boolData());
        timer13 = fbdo.boolData();
        Serial.print("Timer_13 state is = ");
        Serial.println(timer13);

         // If Alarm15 is Live

     if ((timer13 == 1) && (p_tm->tm_hour == 13) && (p_tm->tm_min == 0))
     {    
         digitalWrite(indicator,LOW);
         digitalWrite(relay1,LOW);
         digitalWrite(relay2,LOW);
         timer13 = 1;
         Switch_1 == 1;
         Serial.printf("Switch_1 to firebase... %s\n", Firebase.RTDB.setBool(&fbdo, path3, true) ? "ok" : fbdo.errorReason().c_str());
         }
         
     if ((timer13 == 1) && (p_tm->tm_hour == 14) && (p_tm->tm_min == 58))
     {
         digitalWrite(indicator, HIGH);
         digitalWrite(relay1,HIGH);
         digitalWrite(relay2,HIGH);
         timer13 = 1;
         Switch_1 == 0;
         Serial.printf("Switch_1 to firebase... %s\n", Firebase.RTDB.setBool(&fbdo, path3, false) ? "ok" : fbdo.errorReason().c_str());
         } 
       // 3PM alarm milli done
 }

void alarmThree(){

        time_t now = time(nullptr);
        struct tm* p_tm = localtime(&now);

 String path3 = "/GeyserSwitch/";
        path3 += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
        path3 += "/Switch_1";
        Serial.printf("Get switch state... %s\n", Firebase.RTDB.getBool(&fbdo, path3) ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());

        
 String path15 = "/GeyserSwitch/";
        path15 += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
        path15 += "/Timer_15";
        Serial.printf("3PM timer... ", Firebase.RTDB.getBool(&fbdo, path15) ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());


        if (fbdo.dataType() == "boolean")
       // Serial.println(fbdo.boolData());
        timer15 = fbdo.boolData();
        Serial.print("Timer_15 state is = ");
        Serial.println(timer15);

         // If Alarm15 is Live

     if ((timer15 == 1) && (p_tm->tm_hour == 15) && (p_tm->tm_min == 0))
     {    
         digitalWrite(indicator,LOW);
         digitalWrite(relay1,LOW);
         digitalWrite(relay2,LOW);
         timer15 = 1;
         Switch_1 == 1;
         Serial.printf("Switch_1 to firebase... %s\n", Firebase.RTDB.setBool(&fbdo, path3, true) ? "ok" : fbdo.errorReason().c_str());
         }
         
     if ((timer15 == 1) && (p_tm->tm_hour == 16) && (p_tm->tm_min == 58))
     {
         digitalWrite(indicator, HIGH);
         digitalWrite(relay1,HIGH);
         digitalWrite(relay2,HIGH);
         timer15 = 1;
         Switch_1 == 0;
         Serial.printf("Switch_1 to firebase... %s\n", Firebase.RTDB.setBool(&fbdo, path3, false) ? "ok" : fbdo.errorReason().c_str());
         } 
       // 3PM alarm milli done
 }
       

void alarmSeven(){

        time_t now = time(nullptr);
        struct tm* p_tm = localtime(&now);

 String path3 = "/GeyserSwitch/";
        path3 += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
        path3 += "/Switch_1";
        Serial.printf("Get switch state... %s\n", Firebase.RTDB.getBool(&fbdo, path3) ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());

        
 String path19 = "/GeyserSwitch/";
        path19 += auth.token.uid.c_str(); //<- user uid of current user that sign in with Emal/Password
        path19 += "/Timer_19";
        Serial.printf("7PM timer... ", Firebase.RTDB.getBool(&fbdo, path19) ? fbdo.to<bool>() ? "true" : "false" : fbdo.errorReason().c_str());


        if (fbdo.dataType() == "boolean")
       // Serial.println(fbdo.boolData());
        timer19 = fbdo.boolData();
        Serial.print("Timer_19 state is = ");
        Serial.println(timer19);

         // If Alarm15 is Live

     if ((timer19 == 1) && (p_tm->tm_hour == 19) && (p_tm->tm_min == 0))
     {    
         digitalWrite(indicator,LOW);
         digitalWrite(relay1,LOW);
         digitalWrite(relay2,LOW);
         timer19 = 1;
         Switch_1 == 1;
         Serial.printf("Switch_1 to firebase... %s\n", Firebase.RTDB.setBool(&fbdo, path3, true) ? "ok" : fbdo.errorReason().c_str());
         }
         
     if ((timer19 == 1) && (p_tm->tm_hour == 20) && (p_tm->tm_min == 58))
     {
         digitalWrite(indicator, HIGH);
         digitalWrite(relay1,HIGH);
         digitalWrite(relay2,HIGH);
         timer19 = 1;
         Switch_1 == 0;
         Serial.printf("Switch_1 to firebase... %s\n", Firebase.RTDB.setBool(&fbdo, path3, false) ? "ok" : fbdo.errorReason().c_str());
         } 
       // 3PM alarm milli done
 }
