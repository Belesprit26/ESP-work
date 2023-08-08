#include <DHT.h>
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

//Variables
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

}

void loop()
{
     if (millis() - dataMillis > 7000)
       {
          //READ TEMPERATURE
          float newT = dht.readTemperature();
          if(isnan(newT)) {
          Serial.println("Failed to read from DHT sensor!");
          }else{
            temp = newT;
            Serial.println(temp);
          }
          // READ HUMIDITY
          float newH = dht.readHumidity();
          if(isnan(newH)) {
          Serial.println("Failed to read from DHT sensor!");
          }else{
            humi = newH;
            Serial.println(humi);
          }

          if(temp > 23.00){
            digitalWrite(INDICATOR, LOW); //ON, Low is ON...
            digitalWrite(RELAY1,LOW);
            digitalWrite(RELAY2,LOW);
               }
        
          if(temp < 21.00){
            digitalWrite(INDICATOR, HIGH); //OFF, High is Off...
            digitalWrite(RELAY1,HIGH);
            digitalWrite(RELAY2,HIGH);
               }
          
       }
       
}
