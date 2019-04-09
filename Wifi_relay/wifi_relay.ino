/**
 * Rhomb.io WiFi Relay 2ch 
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
/*
This example is written for a network using WPA encryption. For
WEP, change the Wifi.begin() call accordingly.

Based in the Wifi Web Server sketch 
created for arduino 25 Nov 2012
by Tom Igoe

Ported for sparkfun esp32 
31.01.2017 by Jan Hendrik Berlin
 */

#include <WiFi.h>

const char* ssid     = "your_ssid";      //Insert here your wifi SSID
const char* password = "your_wifi_pass";   //Insert here your wifi Pass
IPAddress prevClient(0, 0, 0, 0);
String header;


WiFiServer server(80);

void setup()
{
    Serial.begin(115200);
    Serial.println("Start");

// Configuring GPIO Output mode
    pinMode(26, OUTPUT);      // IO3
    pinMode(27, OUTPUT);      // IO4
    
// Setting to HIGH due to relay inverse logic
   digitalWrite(26, HIGH);               // IO3
   digitalWrite(27, HIGH);               // IO4

   delay(10);

// We start by connecting to a WiFi network
   Serial.println();
   Serial.println();
   Serial.print("Connecting to ");
   Serial.println(ssid);

   WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.begin();
}

int value = 0;

void loop(){
 WiFiClient client = server.available();   // Listen for incoming clients
 
  if (client) {
    Serial.println("New Client.");          // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c); 
        if (c == '\n') {                    // if the byte is a newline character
          if (currentLine.length() == 0) {      
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

//BUTTON 1
        if (currentLine.endsWith("GET /H1")) {                
          digitalWrite(26, LOW);                // GET /H1 turns relay 1 on
        }
        if (currentLine.endsWith("GET /L1")) {                
          digitalWrite(26, HIGH);               // GET /L1 turns relay 1 off
        }        
//BUTTON 2    
        if (currentLine.endsWith("GET /H2")) {                
          digitalWrite(27, LOW);                // GET /H2 turns relay 2 on
        }
        if (currentLine.endsWith("GET /L2")) {                
          digitalWrite(27, HIGH);               // GET /L2 turns relay 2 off
        }        
// BUTTON 3
        if (currentLine.endsWith("GET /HF")) {                
          digitalWrite(26, LOW);               // GET /HF makes relay 1 to blink
          delay(100);
          digitalWrite(26, HIGH);  
          delay(100);
          digitalWrite(26, LOW);  
          delay(100);
          digitalWrite(26, HIGH); 
          delay(100);
          digitalWrite(26, LOW);  
          delay(100);
          digitalWrite(26, HIGH);
        }
// BUTTON 4
        if (currentLine.endsWith("GET /HG")) {                
          digitalWrite(27, LOW);             // GET /HG makes relay 2 to blink   
          delay(100);
          digitalWrite(27, HIGH);  
          delay(100);
          digitalWrite(27, LOW);  
          delay(100);
          digitalWrite(27, HIGH); 
          delay(100);
          digitalWrite(27, LOW);  
          delay(100);
          digitalWrite(27, HIGH);
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

