/*

  Rhomb.io Long distance RFID TAG reader with GPRS and high gain antenna.

  Copyright (c) Tecnofingers S.L
  All rights reserved.

  @author Rhomb.io
  @version 1.6.0

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

  NOTES:
  - Use Rhomb.io Deimos PCB and Duino Zero master module
  - Insert the SIM868 Rhomb.io slave module at Deimos slave 1 pcb socket
  - Attach the RS485 module to UART_B Rx
*/

//Reset timer
#include <RBD_Timer.h>
RBD::Timer timer;
int Reset = 24; //IO6 PCB
int reset_time = 1*60*60*1000; //1 hour in ms

//UHF definitions
int incomingByte = 0;
byte tag[20];
char char_tag[100];
char char_tag_trimed[100];
int i = 0;

//GSM definitions
//String device_ID = "201910003"; //development
String device_ID = "868183033441986"; //production
String fixed_part = "|s|t:";

//Rhomb SIM868 module pinout
#define GSM_PWREN 2 //IO0 -- IO08
#define GSM_PWRKEY 5  //IO1 -- IO09
#define GPS_EN 6 // IO2 -- IO10
#define GSM_STATUS 7 //IO3 -- IO11

//Modem definition
#define TINY_GSM_MODEM_SIM808
#include <TinyGsmClient.h>

//Other definitions
int LED = 8; //PCB IO4

// Uncomment this if you want to see all AT commands
//#define DUMP_AT_COMMANDS

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon SerialUSB

// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "m2m.orange.es";
const char user[] = "";
const char pass[] = "";

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

TinyGsmClient client(modem);

void setup() {

  //Restart timer
  digitalWrite(Reset, HIGH);
  delay(200); 
  pinMode(Reset, OUTPUT);
  timer.setTimeout(reset_time);
  timer.restart();

  //Startup Serials
  Serial.begin(9600); //UART_B for receiving data from the antenna MCU
  SerialMon.begin(9600); //SerialUSB for debugging
  delay(10);
  SerialUSB.println("Puerto serie iniciado");

  //Startup SIMCOM
  pinMode(GSM_PWREN, OUTPUT);
  pinMode(GSM_PWRKEY, OUTPUT);
  pinMode(GPS_EN, OUTPUT);
  pinMode(GSM_STATUS, INPUT);

  digitalWrite(GSM_PWREN, HIGH);

  for (char i = 0; i < 5; i++) {
    bool gsm_status = digitalRead(GSM_STATUS);
    if (gsm_status == HIGH) {
      SerialUSB.println(F("GSM HIGH!!"));
      break;
    } else {
      SerialUSB.println(F("GSM LOW!"));
      digitalWrite(GSM_PWRKEY, HIGH);
      delay(1500);
      digitalWrite(GSM_PWRKEY, LOW);
      delay(1500);
    }
  }

  // Set GSM module baud rate
  SerialAT.begin(4800); //UART_A for master and slave internal communication
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println(F("Initializing modem"));
  modem.restart();

  String modemInfo = modem.getModemInfo();
  SerialMon.print(F("Modem: "));
  SerialMon.println(modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");

  SerialMon.print(F("Waiting for network"));
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }

  SerialMon.println(" OK - Connected to GSM network!"); // At this point you have GSM connectivity

  //One second blink
  digitalWrite(LED, HIGH);
  delay(1000);
  digitalWrite(LED, LOW);

  SerialMon.print(F("Connecting to "));
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }

  SerialMon.println(" OK - Connected to APN via GPRS!"); // At this point you have GPRS connectivity
  SerialMon.println("Ready to scan a TAG");

  //One second blink
  digitalWrite(LED, HIGH);
  delay(1000);
  digitalWrite(LED, LOW);
}

void loop() {
  //Restart timer
  if(timer.onRestart()) {
    SerialUSB.print(reset_time);
    SerialUSB.println(" miliseconds passed, restarting...");
    digitalWrite(Reset, LOW);
  }
  
  while (Serial.available () > 0) //Read from serial and store in array
  {
    incomingByte = Serial.read ();
    tag[i] =  incomingByte, HEX;
    i++;
  }
  if (i >= 20) { // If array is complete we have scanned a TAG
    SerialUSB.println(" ");
    SerialUSB.print("TAG detected: ");
    array_to_string(tag, 19, char_tag); //Array to string conversion
    SerialUSB.println(char_tag); //Print the string
    i = 0;

    SerialUSB.print("TCP CMD: ");
    String tcpcmd = device_ID + fixed_part + char_tag; //Forming TCP command
    SerialUSB.println(tcpcmd); //Showing TCP command

    // Remote server configuration
    // const uint16_t port = 3136; //development
    const uint16_t port = 4444; //production
    const char * host = "45.76.37.219"; //ip or dns

    if (!client.connect(host, port)) {
      SerialMon.println("connection failed");
      SerialMon.println("Reconnecting...");
      delay(1000);
      if (!modem.gprsConnect(apn, user, pass)) {
        SerialMon.println(" Reconection fail, restarting...");
        digitalWrite(Reset, LOW);
      }
      else {
        SerialMon.println("Reconection success! Resending last TAG");
        client.print(tcpcmd); //Sending the data to server via TCP socket
        String line = client.readStringUntil('\r'); //Waiting for server response
        client.println(line);
        SerialMon.println("Values Sended OK! Closing connection");
        client.stop(); //Closing TCP socket

        //Fast 200 ms blink
        digitalWrite(LED, HIGH);
        delay(200);
        digitalWrite(LED, LOW);
        delay(200);
        digitalWrite(LED, HIGH);
        delay(200);
        digitalWrite(LED, LOW);
      }
      return;
    }

    client.print(tcpcmd); //Sending the data to server via TCP socket
    String line = client.readStringUntil('\r'); //Waiting for server response
    client.println(line);
    SerialMon.println("Values Sended OK! Closing connection");
    client.stop(); //Closing TCP socket

    //Fast 200 ms blink
    digitalWrite(LED, HIGH);
    delay(200);
    digitalWrite(LED, LOW);
    delay(200);
    digitalWrite(LED, HIGH);
    delay(200);
    digitalWrite(LED, LOW);
  }
} //End loop

//Function declaration for byte array to string conversion
void array_to_string(byte array[], unsigned int len, char buffer[])
{
  for (unsigned int i = 0; i < len; i++)
  {
    byte nib1 = (array[i] >> 4) & 0x0F;
    byte nib2 = (array[i] >> 0) & 0x0F;
    buffer[i * 2 + 0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
    buffer[i * 2 + 1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
  }
  buffer[len * 2] = '\0';
}

