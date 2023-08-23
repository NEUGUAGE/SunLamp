#include <heltec.h> //import heltec's library. Can be changed using different modules
#include  <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <string>
#include <iostream>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define PIN        34
#define NUMPIXELS  60
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
// the setup routine runs once when starts up
void setup(){
  Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, 470E6 /**/);
  pixels.begin();
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  delay(100);
  Serial.println("\nSetup done");
  delay(100);
  WiFi.disconnect();
  delay(100);
}
///////////////////Wifi Part///////////////////
void initWiFi() {
  //Enter Your SSID and password//
  const char* ssid = "YOUR WIFI NAME";
  const char* password = "YOUR WIFI PASSWWORD";
  ///////////////////////////////
  WiFi.begin(ssid, password);
  Serial.println("$Connecting to WiFi Right Now$");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting...");
    delay(10000);
    Serial.println("Please Wait...");
  }
  if (WiFi.status()==WL_CONNECTED){
    Serial.println("Successfully Connected to ");
    Serial.print(ssid);
     Serial.print(" IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println("RRSI: ");
    Serial.print(WiFi.RSSI());
  }
}

// the loop routine runs over and over again forever
void loop() {
  if (WiFi.status() != WL_CONNECTED){
  Serial.print("Wifi Status: Disconnected");
  Serial.print("\nScan Start");
  int n = WiFi.scanNetworks();
  if (n == 0) {
      Serial.println("no networks found");
      Serial.println("Scan done");
  } 
  else {
    Serial.print(n);
    Serial.println("\nnetworks found");
    delay(100);
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each ne`twork found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      delay(10);
    }
    Serial.println("Scan done");
    delay(100);
    initWiFi();
  }
  }
  delay(500);
  ///////////////////End of Wifi Part///////////////////

  /////////////////////Get information from the sun rise//////////////////////
  if (WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    String serverPath_Sun;
    String response_Sun = "{}";
    String serverName_Sun;
    String sunrise;
    String sunset;
    serverName_Sun = "https://api.sunrisesunset.io/json";//Can be replaced with other website
    serverPath_Sun = serverName_Sun+ "?lat=38.907192&lng=-119.036873&timezone=PST&date=today"; //This specify the query string
    http.begin(serverPath_Sun);
    int httpResponseCode = http.GET();
    //Serial.println("\nResponse From the Server: ");
    //Serial.println(httpResponseCode);//Get the state
    response_Sun=http.getString();//Get the detailed response
    sunrise=(response_Sun.substring(23,27));
    sunset=(response_Sun.substring(45,49));
    //Serial.println(sunrise);
    //Serial.println(sunset);
    http.end();

/////////////////////End of getting information from the sun rise///////////////////

/////////////////////Get information from the time///////////////////
    String serverName_Time;
    String serverPath_Time;
    String response_Time;
    serverName_Time = "https://www.timeapi.io/api/Time/current/coordinate";//This API can be replaced
    serverPath_Time = serverName_Time+"?latitude=34&longitude=-119";
    http.begin(serverPath_Time);
    int response_Time_Code=http.GET();
    response_Time=http.getString();
    int Time_position=0;
    //This part is used to find specific part of the API, Can be replaced
    for (int findtime=140;findtime<160;findtime=findtime+1){
      String a=response_Time.substring(findtime,findtime+1);
      String b=response_Time.substring(findtime+1,findtime+2);
      if (a=="t"&&b=="i"){
        Time_position=findtime;        
      } 
    }
    String time=response_Time.substring(Time_position+7,Time_position+12);
    //Serial.println(response_Time.substring(Time_position+7,Time_position+12));
    http.end();

/////////////////////End of time getting part///////////////////

////////////////////Start of the ouput tranferring part//////////////
String sunrise_hour=sunrise.substring(0,1);
String sunrise_minutes=sunrise.substring(2,4);
String sunset_hour=sunset.substring(0,1);
String sunset_minutes=sunrise.substring(2,4);
String time_hour;
if(time.substring(0,1)=="0"){
  time_hour=time.substring(1,2);
}
else{
  time_hour=time.substring(0,2);
}
String time_minutes=time.substring(3,5);
int time_hour_int=time_hour.toInt();
int time_minutes_int=time_minutes.toInt();
int sunrise_hour_int=sunrise_hour.toInt();
int sunrise_minutes_int=sunrise_minutes.toInt();
int sunset_hour_int=sunset_hour.toInt();
int sunset_minutes_int=sunset_minutes.toInt();
int sunset_hour_int_24=sunset_hour_int+12;
int time_difference=0;
int intensity;
//wake up two hours after sunrise
if (time_hour_int<12&time_hour_int>sunrise_hour_int+2){
intensity=60*(time_hour_int-sunrise_hour_int)+360;
}
else if(time_hour_int>12&time_hour_int<=sunset_hour_int_24){
intensity=60*(sunset_hour_int_24-time_hour_int)+360;
}
else if(time_hour_int<23&time_hour_int>sunset_hour_int_24){
intensity=60*(sunset_hour_int_24-time_hour_int)+360;
}
else{
intensity=0;
}
//////////////////Light Part//////////////////////////////
Serial.println(time);
Serial.println(time_hour);
Serial.println(sunrise_hour);
Serial.println(sunset_hour_int_24);
Serial.println(time_hour_int);
Serial.println(intensity);//range from 800-0
int Brightness;
if (intensity!=0){
Brightness=(intensity+1)/5+80;
}
else{
Brightness=0;  
}
pixels.setBrightness(Brightness);
for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
    // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(255, 80+(1+intensity)/8, (intensity+1)/12+i));
    //pixels.setPixelColor(i, pixels.Color(255, 128, 0);
    pixels.show();   // Send the updated pixel colors to the hardware.
// Pause before next pass through loop
}
delay(300*1000);
}
}















