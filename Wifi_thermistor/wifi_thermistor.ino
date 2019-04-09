/*
* Rhomb.io WiFi Temperature Sensing
*
* Copyright (c) Tecnofingers S.L
* All rights reserved.
*
* @author Rhomb.io
* @version 1.0.0
*
* MIT License
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/


#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <math.h>

//Here you can define default values, if you want.
char intervalo_tiempo[40];
char offset_temperatura[6];

//If this pin is LOW during boot process, configurations are reset and the device will reboot in AP mode. IO1 at Deimos Board.
int rpin = 4;

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;

  //Deprecated, for reference use only
  //int periodo = 1000;
  //double offset = 6.5;
}


void setup() {
  Serial.begin(115200);
  Serial.println();
  pinMode(rpin, INPUT_PULLUP);

  //Uncomment if you want to format SPI Filesystem
  // SPIFFS.format();

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          Serial.println("\nparsed json");

          strcpy(intervalo_tiempo, json["intervalo_tiempo"]);
          strcpy(offset_temperatura, json["offset_temperatura"]);


        } else {
          Serial.println("failed to load json config");
        }
        configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read

  WiFiManagerParameter custom_intervalo_tiempo("intervalo", "intervalo tiempo", intervalo_tiempo, 40);
  WiFiManagerParameter custom_offset_temperatura("offset", "offset temperatura", offset_temperatura, 6);


  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  //Descomentar si se desa IP estática
  // wifiManager.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  //Custom parameters to get interval and offset values from config portal
  wifiManager.addParameter(&custom_intervalo_tiempo);
  wifiManager.addParameter(&custom_offset_temperatura);


  //Uncomment this line if you want to reset the settings
  // wifiManager.resetSettings();

  //Another way (easyest) to reset parameters and access to configuration portal is to put IO1 to GND at boot time.
  if(digitalRead(rpin)==LOW){
    Serial.println("Reseteando parametros");
    wifiManager.resetSettings();
    delay(1000);
    Serial.println("Reiniciando en modo AP");
    delay(1000);
    ESP.restart();
  }

  //set minimu quality of signal so it ignores AP's under that quality
  //defaults to 8%
  //wifiManager.setMinimumSignalQuality();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifiManager.setTimeout(120);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration

  //Config the AP name and password to access configuration portal
  if (!wifiManager.autoConnect("Rhomb.io_AP", "rhombio2018")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("Connected to WiFi!");

  //read updated parameters
  strcpy(intervalo_tiempo, custom_intervalo_tiempo.getValue());
  strcpy(offset_temperatura, custom_offset_temperatura.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["intervalo_tiempo"] = intervalo_tiempo;
    json["offset_temperatura"] = offset_temperatura;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  Serial.println("local ip");
  Serial.println(WiFi.localIP());
  delay(500);
}

//Temperature measuring function.
double Thermister(int RawADC) {
  double Temp;
  Temp = log(((40960000/RawADC) - 10020)); //Mega = 1023 y aqui pone 1024, ESP32 = 4095 y aqui pongo 4096  // 10020 es el valor de la resistencia
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp ))* Temp );
  Temp = Temp + 273.15;// Converierte de Kelvin a Celsius // CAMBIAR SI MIDE VALORES NEGATIVOS
  Temp = Temp - 500; // ESP8266 offset
  //Para convertir Celsius a Farenheith esriba en esta linea: Temp = (Temp * 9.0)/ 5.0 + 32.0;
  return Temp;
}


void loop() {

  //Casting char arrays to int and double values
  int custom_intervalo = atoi(intervalo_tiempo);
  double custom_offset = atof(offset_temperatura);

  //Serial printing actual parameters
  Serial.print("Actual configuration: interval(sec.)/offset(ºC): ");
  Serial.print(custom_intervalo);
  Serial.print("/");
  Serial.println(offset_temperatura);
  delay(1000);


  //Temperature measuring related code
  int val;
  double temp;//Variable de temperatura = temp
  double temp_ok;//Offset recalculation value
  val=analogRead(A0);//Lee el valor del pin analogo 0 y lo mantiene como val
  //temp=Thermister(val+100);//Realiza la conversión del valor analogo a grados Celsius
  temp=Thermister(val);
  temp_ok=temp+custom_offset;
  Serial.print("Actual temperature: ");
  Serial.print(temp_ok);
  Serial.println(" ºC");

  //Forming the TCPcmd
  String StrUno = "837289382|s|temp:";
  String StrDos = StrUno + temp_ok ;

  //IoT Server IP and Port
  const uint16_t port = 4444;
  const char * host = "45.76.37.219"; // ip or dns
  
  Serial.print("Connecting to IoT Server ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;

  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    Serial.println("wait 5 sec...");
    delay(5000);
    return;
  }

  // This will send the request to the server
  client.print(StrDos);
  Serial.print("TCP CMD: ");
  Serial.println(StrDos);
  //read back one line from server
  String line = client.readStringUntil('\r');
  client.println(line);

  Serial.println("Values Sended OK! Closing connection");
  client.stop();

  Serial.println("Waiting period...");
  delay(custom_intervalo*1000);

}
