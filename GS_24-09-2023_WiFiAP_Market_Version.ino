/* https://github.com/chriscook8/esp-arduino-apboot/blob/master/ESP-wifiboot.ino 
 Github: https://github.com/mobizt/Firebase-ESP-Client
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <time.h>

//-----------------------------Firebase Stuff -------------------------------
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>


/* 1. Define the WiFi credentials */
const char* ssid = "Buti";
const char* passphrase = "buti12345";

/* 2. Define the API Key */
#define API_KEY "AIzaSyCS0DTQhwu5ty8b38Gmmpr-ynVOl01lxOs"
#define DATABASE_URL "https://gsthethird-1f244-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define DATABASE_SECRET "LtqAFx64eMGTjnOPTGiebVqgWyvR0emvpf2QMoOz"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "gsthethird-1f244"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
const char* USER_EMAIL = "";
const char* USER_PASSWORD = "";
const char* USER_PATH = "users/";
const char* UID = "";
String UIDs = "";

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

//--------------------------------------------------------------

//Variables
int i = 0;
int statusCode;

unsigned long dataMillis = 0;
unsigned long dataMillis2 = 0;

float temp = 11.11;
float humi = 0.0;
bool Switch_1 = 0;

int outputpin= A0;
const int indicator = 2;
const int relay = 4;
int timer2 = 0;
int timer4 = 0;
int timer13 = 0;
int timer15 = 0;
int timer17 = 0;

int timezone = 2 * 3600;
int dst = 0;

String st;
String content;

//Function Decalration
bool testWifi(void);
void launchWeb(void);
void setupAP(void);
void uploadTemp(void);
void switchState(void);
void alarmTwo(void);
void alarmFour(void);
void alarmOne(void);
void alarmThree(void);
void alarmFive(void);

//Establishing Local server at port 80 whenever required
ESP8266WebServer server(80);
void setup()
{
  pinMode(indicator,OUTPUT);
  pinMode(relay,OUTPUT);
  Serial.begin(115200); //Initialising if(DEBUG)Serial Monitor
  Serial.println();
  Serial.println("Disconnecting current wifi connection");
  WiFi.disconnect();
  EEPROM.begin(512); //Initialasing EEPROM
  delay(10);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println();
  Serial.println();
  Serial.println("Startup");
  //---------------------------------------- Read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");
  String esid;
  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println();
  //------------------------------------------------
  Serial.println("Reading EEPROM pass");
  String epass = "";
  for (int i = 32; i < 64; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);
  Serial.println();
  //-------------------------------------------------
  Serial.println("Reading EEPROM email");
  String eemail = "";
   for (int i = 64; i < 96; ++i)
  {
    eemail += char(EEPROM.read(i));
    USER_EMAIL = eemail.c_str();
  }
  Serial.print("Email: ");
  Serial.println(eemail);
  Serial.println();
  //---------------------------------------------------
  Serial.println("Reading EEPROM User Password");
  String epass2 = "";
  for (int i = 96; i < 128; ++i)
  {
    epass2 += char(EEPROM.read(i));
    USER_PASSWORD = epass2.c_str();
  }
  Serial.print("User Password: ");
  Serial.println(epass2);
  Serial.println();
  //-----------------------------------------------------
  Serial.println("Reading EEPROM UID");
  String euid = "";
  for (int i = 128; i < 160; ++i)
  {
    euid += char(EEPROM.read(i));
    UIDs = euid;
  }
  Serial.print("UID: ");
  Serial.println(euid);
  Serial.println();
  
  WiFi.begin(esid.c_str(), epass.c_str());
  if (testWifi())
  {
    Serial.println("Succesfully Connected!!!");
    
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    config.database_url = DATABASE_URL;

    fbdo.setResponseSize(4096);
    String base_path = String() + "users/" + UIDs;

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
    return;
  }
  else
  {
    Serial.println("Turning the HotSpot On");
    launchWeb();
    setupAP();// Setup HotSpot
  }
  Serial.println();
  Serial.println("Waiting.");
  while ((WiFi.status() != WL_CONNECTED))
  {
    Serial.print(".");
    delay(100);
    server.handleClient();
  }

  
}
void loop() {
  if ((WiFi.status() == WL_CONNECTED))
  {
    if (Firebase.ready() && (millis() - dataMillis > 5000 || dataMillis == 0)) 
    { 
      dataMillis = millis();
    
      uploadTemp();

      switchState();

      alarmTwo();

      alarmFour();

      alarmOne();

      alarmThree();
      
      alarmFive();
      
    }
  }
  else
  {
  }
}
//--------------------------------GeyserSwitch Stuff----------------------------------
void uploadTemp()
  {
        time_t now = time(nullptr);
        struct tm* p_tm = localtime(&now);
        Serial.print(p_tm->tm_hour);
        Serial.print(":");
        Serial.println(p_tm->tm_min);
        

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
        String documentPath = String() + "users/" + UIDs;
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
            digitalWrite(relay, LOW);
          }

        if ((Switch_1 == 0)){
            digitalWrite(indicator, HIGH); //OFF, High is Off...
            digitalWrite(relay, HIGH);
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
         //digitalWrite(relay1,LOW);
         //digitalWrite(relay2,LOW);
         timer2 = 1;
         Switch_1 == 1;
         Serial.printf("Switch_1 to firebase... %s\n", Firebase.RTDB.setBool(&fbdo, path3, true) ? "ok" : fbdo.errorReason().c_str());
         }
         
     if ((timer2 == 1) && (p_tm->tm_hour == 3) && (p_tm->tm_min == 58))
     {
         digitalWrite(indicator, HIGH);
         //digitalWrite(relay1,HIGH);
         //digitalWrite(relay2,HIGH);
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
         //digitalWrite(relay1,LOW);
         //digitalWrite(relay2,LOW);
         timer4 = 1;
         Switch_1 == 1;
         Serial.printf("Switch_1 to firebase... %s\n", Firebase.RTDB.setBool(&fbdo, path3, true) ? "ok" : fbdo.errorReason().c_str());
         }
         
     if ((timer4 == 1) && (p_tm->tm_hour == 5) && (p_tm->tm_min == 58))
     {
         digitalWrite(indicator, HIGH);
         //digitalWrite(relay1,HIGH);
         //digitalWrite(relay2,HIGH);
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
         //digitalWrite(relay1,LOW);
         //digitalWrite(relay2,LOW);
         timer13 = 1;
         Switch_1 == 1;
         Serial.printf("Switch_1 to firebase... %s\n", Firebase.RTDB.setBool(&fbdo, path3, true) ? "ok" : fbdo.errorReason().c_str());
         }
         
     if ((timer13 == 1) && (p_tm->tm_hour == 14) && (p_tm->tm_min == 58))
     {
         digitalWrite(indicator, HIGH);
         //digitalWrite(relay1,HIGH);
         //digitalWrite(relay2,HIGH);
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
         //digitalWrite(relay1,LOW);
         //digitalWrite(relay2,LOW);
         timer15 = 1;
         Switch_1 == 1;
         Serial.printf("Switch_1 to firebase... %s\n", Firebase.RTDB.setBool(&fbdo, path3, true) ? "ok" : fbdo.errorReason().c_str());
         }
         
     if ((timer15 == 1) && (p_tm->tm_hour == 16) && (p_tm->tm_min == 58))
     {
         digitalWrite(indicator, HIGH);
         //digitalWrite(relay1,HIGH);
         //digitalWrite(relay2,HIGH);
         timer15 = 1;
         Switch_1 == 0;
         Serial.printf("Switch_1 to firebase... %s\n", Firebase.RTDB.setBool(&fbdo, path3, false) ? "ok" : fbdo.errorReason().c_str());
         } 
       // 3PM alarm milli done
 }
       

void alarmFive(){

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
        timer17 = fbdo.boolData();
        Serial.print("Timer_19 state is = ");
        Serial.println(timer17);

         // If Alarm15 is Live

     if ((timer17 == 1) && (p_tm->tm_hour == 17) && (p_tm->tm_min == 0))
     {    
         digitalWrite(indicator,LOW);
         //digitalWrite(relay1,LOW);
         //digitalWrite(relay2,LOW);
         timer17 = 1;
         Switch_1 == 1;
         Serial.printf("Switch_1 to firebase... %s\n", Firebase.RTDB.setBool(&fbdo, path3, true) ? "ok" : fbdo.errorReason().c_str());
         }
         
     if ((timer17 == 1) && (p_tm->tm_hour == 18) && (p_tm->tm_min == 58))
     {
         digitalWrite(indicator, HIGH);
         //digitalWrite(relay1,HIGH);
         //digitalWrite(relay2,HIGH);
         timer17 = 1;
         Switch_1 == 0;
         Serial.printf("Switch_1 to firebase... %s\n", Firebase.RTDB.setBool(&fbdo, path3, false) ? "ok" : fbdo.errorReason().c_str());
         } 
       // 3PM alarm milli done
 }

//----------------------------------------------- Fuctions used for WiFi credentials saving and connecting to it which you do not need to change
bool testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}
void launchWeb()
{
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}
void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP("GeyserSwitchConnect", "");
  Serial.println("Initializing_softap_for_wifi credentials_modification");
  launchWeb();
  Serial.println("over");
}
void createWebServer()
{
  {
    server.on("/", []() {
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]) + '.' + String(ip[4]) + '.' + String(ip[5]) + '.' + String(ip[6]) + '.' + String(ip[7]) + '.' + String(ip[8]) + '.' + String(ip[9]);
      content = "<!DOCTYPE HTML>\r\n";
      content += "<html><body>";
      content += "<center>";
      content += "<h1>GeyserSwitchConnect</h1>";
      content += "<br>";
      content += "<h2>Welcome to the GeyserSwitch Config Panel</h2>";
      content += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      content += "<h3>Available Networks</h3>";
      content += ipStr;
      
      content += "<p>";
      content += st;
      content += "</p>";
      content += "<br>";
      content += "<h4>Please enter your WiFi router details & GeyserSwitch login details below</h4>";
      content += "<br>";
      content += "<h5>Your Home WiFi Details</h5>";
      content += "<form method='get' action='setting'><label> WiFi SSID: </label><input name='ssid' length=32><br><br>";
      content += "<label>WiFi Password: </label><input name='pass' length=64><br><br>";
      content += "<br>";
      content += "<h5>Your GS Login Details</h5>";
      content += "<label>User Email: </label><input name='email' length=64><br><br>"; 
      content += "<label>User Password: </label><input name='pass2' length=64><br><br>";
      content += "<label>User ID: </label><input name='uid' length=64><br><br><input type='submit'><br></form>";
      content += "<br><br>";
      content += "</center>";
      content += "</body>";
      content += "</html>";
      
      server.send(200, "text/html", content);
    });
    server.on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]) + '.' + String(ip[4]) + '.' + String(ip[5]) + '.' + String(ip[6]) + '.' + String(ip[7]) + '.' + String(ip[8]) + '.' + String(ip[9]);
      content = "<!DOCTYPE HTML>\r\n<html>go back";
      server.send(200, "text/html", content);
    });
    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      String uemail = server.arg("email");
      String upass = server.arg("pass2");
      String uuid = server.arg("uid");
      
      if (qsid.length() > 0 && qpass.length() > 0 && uemail.length() > 0 && upass.length() > 0 && uuid.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 160; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");
        Serial.println(uemail);
        Serial.println("");
        Serial.println(upass);
        Serial.println("");
        Serial.println(uuid);
        Serial.println("");
        //ssid
        Serial.println("writing eeprom ssid:");
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i]);
        }
        //pass
        Serial.println("writing eeprom pass:");
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i]);
        }
        //user email
        Serial.println("writing eeprom user email:");
        for (int i = 0; i < uemail.length(); ++i)
        {
          EEPROM.write(64 + i, uemail[i]);
          Serial.print("Wrote: ");
          Serial.println(uemail[i]);
        }
        //user password
        Serial.println("writing eeprom user password:");
        for (int i = 0; i < upass.length(); ++i)
        {
          EEPROM.write(96 + i, upass[i]);
          Serial.print("Wrote: ");
          Serial.println(upass[i]);
        }
        //user UID
        Serial.println("writing eeprom user uid:");
        for (int i = 0; i < uuid.length(); ++i)
        {
          EEPROM.write(128 + i, uuid[i]);
          Serial.print("Wrote: ");
          Serial.println(uuid[i]);
        }
        EEPROM.commit();
        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.reset();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);
    });
  }
}
