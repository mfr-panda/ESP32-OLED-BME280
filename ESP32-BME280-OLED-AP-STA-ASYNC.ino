#include <WiFi.h>
#include <WiFiAP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// includes necessaires au fonctionnement de l'OTA :
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

SSD1306  display(0x3c, 5, 4);  

#define SEALEVELPRESSURE_HPA (1032.00)

Adafruit_BME280 bme;  //i2c

float temperature, humidity, pressure, altitude;

String temperature1, humidity1, pressure1, altitude1, adresse, adresse_AP;

/*Router SSID & Password*/
char* ssid_1 = "SSID-1";                       // Identifiant WiFi
char* password_1 = "PASSWORD-1";                // Mot de passe WiFi

char* ssid_2 = "SSID-2";                       
char* password_2 = "PASSWORD-2";

char* ssid = "";
char* password= "";

char* ssid_ap = "sonde-iot-BME280;

boolean ssid_ok = false;

String AdrIP;
String AdrIP_AP;

AsyncWebServer server(80);

/* ****************************
 *  SCAN WIFI
 *  **************************/

void scanWifi() {
  int n = WiFi.scanNetworks();
  Serial.println("scan OK");  
  if (n == 0) {
    Serial.println("Pas de réseaux");    
  } else {
    Serial.print(n);
    Serial.println(" Réseaux trouvés");
    
    for (int i = 0; i < n; ++i) {
      // Print SSID pour chaque réseau
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.println(WiFi.RSSI(i));
            
      if(WiFi.SSID(i) == ssid_1){ 
      Serial.println(String(ssid_1) + " : Réseau Connu");
      ssid = ssid_1;
      password = password_1;
      ssid_ok=true;
      }
      if(WiFi.SSID(i) == ssid_2){ 
      Serial.println(String(ssid_2) + " : Réseau Connu");
      ssid = ssid_2;
      password = password_2;
      ssid_ok=true;
      }
    }
  }
}
 
void setup() {
  Serial.begin(115200);
  delay(100);
  int i = 0;
  
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
  
   // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  display.clear();  

  scanWifi();
  delay(100);
  WiFi.mode(WIFI_AP_STA);


    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "Creation AP");
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 26, "sonde-iot-fablac");
    display.display();
    delay(2000);
    WiFi.softAP(ssid_ap, NULL,7,0,5);
    Serial.println(WiFi.softAPIP());
    AdrIP_AP = WiFi.softAPIP().toString();
    display.clear();
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, ssid_ap);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 26, AdrIP_AP);
    display.display();
    delay(4000);
  
    if (ssid_ok == false){
      display.clear();
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 0, "Pas de reseaux");
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 12, "Utilisez AP :");
      display.setFont(ArialMT_Plain_16);
      display.drawString(0, 24, "sonde-iot-fablac");
      display.display();
      delay(3000);

    }
    
  
  if (ssid_ok == true){
    Serial.println("Connecting to ");
    Serial.println(ssid);
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 0, "Connexion à");
    display.setFont(ArialMT_Plain_16);
    display.drawString(0, 26, ssid);
    display.display();
    delay(100);
    //WiFi.mode(WIFI_STA); 
   
  //connexion au réseau wifi local
    WiFi.begin(ssid, password);
  
    while ((WiFi.status() != WL_CONNECTED) and (i<16)) {
      delay(500);
      i=i+1;      
      Serial.print(".");
      }
    if (i<16) {
      Serial.println("");
      Serial.println("WiFi connected..!");
      Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
      AdrIP = WiFi.localIP().toString();
      display.clear();
      display.setFont(ArialMT_Plain_16);
      display.drawString(0, 0, ssid);
      display.setFont(ArialMT_Plain_16);
      display.drawString(0, 26, AdrIP);
      display.display();
    }
    
    delay(3000);
  }

  
 
  Wire1.begin(21,22);
  bme.begin(0x76,&Wire1);

  server.on("/", HTTP_GET, handle_OnConnect);
  server.onNotFound(notFound);

  server.begin();
  Serial.println("HTTP server started");

  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, ssid);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 10, AdrIP);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 26, "HTTP server OK");
  display.display();
  delay(2000);

  ArduinoOTA.setHostname("Sonde-Temperature-FabLac"); // on donne une petit nom a notre module
  ArduinoOTA.setPassword("F4bL4c");
  ArduinoOTA.begin(); // initialisation de l'OTA

}

void displayBME(){
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);

  temperature1 = "Température : " + String(temperature) + " °C";
  humidity1 = "Humidité : " + String(humidity) + " %";
  pressure1 = "Pression : " + String(pressure) + " hPa";
  //altitude1 = "Altitude : " + String(altitude) + " m";
  adresse_AP = "IP AP :" + AdrIP_AP;
  adresse = " IP STA : " + AdrIP;

  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, temperature1);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 12, humidity1);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 24, pressure1);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 36, adresse_AP);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 48, adresse);
  display.display();
  
  }

void handle_OnConnect(AsyncWebServerRequest *request) {
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  request->send(200, "text/html", SendHTML(temperature,humidity,pressure,altitude)); 
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}


String SendHTML(float temperature,float humidity,float pressure,float altitude){
  String ptr = "<!doctype html><head><meta charset=\"UTF-8\">\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<meta http-equiv=\"refresh\" content=\"2\">\n";
  ptr +="<title>Sonde FabLac ESP32</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1>Sonde FabLac ESP32</h1>\n";
  ptr +="<p>Température: ";
  ptr +=temperature;
  ptr +="&deg;C</p>";
  ptr +="<p>Humidité: ";
  ptr +=humidity;
  ptr +="%</p>";
  ptr +="<p>Pression: ";
  ptr +=pressure;
  ptr +="hPa</p>";
  ptr +="<p>Altitude: ";
  ptr +=altitude;
  ptr +="m</p>";
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

void loop() {
   ArduinoOTA.handle();
  displayBME();
  delay(500);
}
