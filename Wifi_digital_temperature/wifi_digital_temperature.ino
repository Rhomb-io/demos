/*
  Rhomb.io WiFi Temperature Sensing

  Copyright (c) Tecnofingers S.L
  All rights reserved.

  @author Rhomb.io
  @version 4.0.0

  MIT License

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

// Sonda digital
#include <OneWire.h>
#include <DallasTemperature.h>
//#define ONE_WIRE_BUS A3 //Rhomb.io Duino UNO - Conectar a 1-WIRE
//#define ONE_WIRE_BUS 21 //Rhomb.io ESP32 - Conectar a SDA
//#define ONE_WIRE_BUS 2 //Rhomb.io ESP8266 - Conectar a SDA
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.

#define LEDPIN 5 // DEIMOS IO2 - ESP8266

//Here you can define default values, if you want.
char intervalo_tiempo[40];
char offset_temperatura[6];
char id_dispositivo[40];
char iot_server[40];
char iot_port[40];



//If this pin is LOW during boot process, Wifi configurations are reset and the device will reboot in AP mode. IO1 at Deimos Board.
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
  Serial.println("Startup OK");
  pinMode(LEDPIN, OUTPUT);

  sensors.begin();  // Start up the temperature sensor library

  pinMode(rpin, INPUT_PULLUP);

  //Uncomment if you want to format SPI Filesystem
//  SPIFFS.format();

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
          strcpy(id_dispositivo, json["id_dispositivo"]);
          strcpy(iot_server, json["iot_server"]);
          strcpy(iot_port, json["iot_port"]);

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

  WiFiManagerParameter custom_intervalo_tiempo("intervalo", "Time interval (sec.) Ex.: 5 (max. 4200)", intervalo_tiempo, 40);
  WiFiManagerParameter custom_offset_temperatura("offset", "Temperature offset (celsius degrees) Ex.: 1.32, Ex.: -1.25, Ex: 0", offset_temperatura, 6);
  WiFiManagerParameter custom_id_dispositivo("id", "Device ID (number) Ex.: 837289382", id_dispositivo, 40);
  WiFiManagerParameter custom_iot_server("server", "Server IP Ex.: 36.84.25.999", iot_server, 40);
  WiFiManagerParameter custom_iot_port("port", "Server port Ex.: 2522", iot_port, 40);


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
  wifiManager.addParameter(&custom_id_dispositivo);
  wifiManager.addParameter(&custom_iot_server);
  wifiManager.addParameter(&custom_iot_port);

  //Uncomment this line if you want to reset the settings
  //  wifiManager.resetSettings();

  //Another way (easyest) to reset parameters and access to configuration portal is to put IO1 to GND at boot time.
  if (digitalRead(rpin) == LOW) {
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
  strcpy(id_dispositivo, custom_id_dispositivo.getValue());
  strcpy(iot_server, custom_iot_server.getValue());
  strcpy(iot_port, custom_iot_port.getValue());

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["intervalo_tiempo"] = intervalo_tiempo;
    json["offset_temperatura"] = offset_temperatura;
    json["id_dispositivo"] = id_dispositivo;
    json["iot_server"] = iot_server;
    json["iot_port"] = iot_port;

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

void loop() {

  double temp;
  double temp_ok;

  //Casting char arrays to int and double values
  int custom_intervalo = atoi(intervalo_tiempo);
  double custom_offset = atof(offset_temperatura);
  int custom_id = atoi(id_dispositivo);
  int custom_port = atoi(iot_port);



  //Serial printing actual parameters
  Serial.println(" ");
  Serial.print("Actual configuration: interval(sec.)/offset(ºC)/ID(number): ");
  Serial.print(custom_intervalo);
  Serial.print("/");
  Serial.print(offset_temperatura);
  Serial.print("/");
  Serial.println(id_dispositivo);
  //Temperature measuring related code

  sensors.requestTemperatures(); // Send the command to get temperature readings


  temp = sensors.getTempCByIndex(0);
  temp_ok = temp + custom_offset;

  Serial.print("Temperature is: ");
  Serial.println(temp_ok); // Why "byIndex"?

  // END TEMPERATURE MEASURING CODE




  //Forming the TCPcmd
  String StrUno = "|s|temp:";
  String StrDos = id_dispositivo + StrUno + temp_ok ;

  //IoT Server IP and Port
  const uint16_t port = custom_port;
  const char * host = iot_server; // ip or dns

  Serial.print("TCP CMD: ");
  Serial.println(StrDos);
  Serial.println("Connecting to IoT Server...");

  // Use WiFiClient class to create TCP connections
  WiFiClient client;

  if (!client.connect(host, port)) {
    Serial.print("connection failed, ");
    Serial.println("waiting 5 seconds before retry...");
    delay(5000);
    return;
  }

  // This will send the request to the server
  client.print(StrDos);

  //read back one line from server
  String line = client.readStringUntil('\r');
  client.println(line);

  Serial.println("Values Sended OK!");
  client.stop();

  digitalWrite(LEDPIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(250);                       // wait for a second
  digitalWrite(LEDPIN, LOW);  
  delay(250); 
  
  digitalWrite(LEDPIN, HIGH);
  delay(250);
  digitalWrite(LEDPIN, LOW);

  Serial.print("Closing TCP connection... ");
  client.stop();
  Serial.println(" OK!");

  Serial.print("Going to sleep now. Wake period: ");
  Serial.print(custom_intervalo);
  Serial.println(" seconds...");
  Serial.println(" ");

  ESP.deepSleep(custom_intervalo * 1000000); // 20e6 is 20 microseconds

}
