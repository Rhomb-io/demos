// Rhomb.io LoRa RX Totem
// -*- mode: C++ -*-
// Este sketch recibe por LoRa el UUID de un TAG NFC escaneado en un dispositivo emisor. 
// Envia por RS-485 la info. Envía un ACK al emisor. 

#include <SPI.h>
#include <RH_RF95.h>
#define RFM95_CS 53
#define RFM95_RST 9
#define RFM95_INT 3

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 433.0
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);
// Blinky on receipt
#define LED 13
void setup()
{
pinMode(37, OUTPUT);  // pin para RS485
pinMode(LED, OUTPUT);
pinMode(RFM95_RST, OUTPUT);
digitalWrite(RFM95_RST, HIGH);
while (!Serial);
Serial.begin(9600);
delay(100);
Serial.println("Rhomb.io LoRaNFC-RX. Prueba de concepto.");
// manual reset
digitalWrite(RFM95_RST, LOW);
delay(10);
digitalWrite(RFM95_RST, HIGH);
delay(10);
while (!rf95.init()) {
Serial.println("Fallo en la inicialización del Rhomb.io LoRa Slave Module");
while (1);
}
Serial.println("Inicialización del Rhomb.io LoRa Slave Module OK!");
// Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
if (!rf95.setFrequency(RF95_FREQ)) {
Serial.println("Fallo al establecer la frecuencia");
while (1);
}
Serial.print("Frecuencia establecida (MHz): "); Serial.println(RF95_FREQ);
// Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
// The default transmitter power is 13dBm, using PA_BOOST.
// If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
// you can set transmitter powers from 5 to 23 dBm:
rf95.setTxPower(23, false);
}
void loop()
{
if (rf95.available())
{
// Should be a message for us now
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t len = sizeof(buf);
if (rf95.recv(buf, &len))
{
digitalWrite(LED, HIGH);
Serial.println("");
digitalWrite(37, HIGH); //gipo RS485 high
delay(100);
 
RH_RF95::printBuffer("Se han recibido estos datos: ", buf, len);

delay(100);
digitalWrite(37, LOW);//gpio rs485 low

Serial.println("Se han enviado los datos por el bus RS485");
Serial.print("RSSI: ");
Serial.println(rf95.lastRssi(), DEC);
// Send a reply
uint8_t data[] = "rhomb745";
rf95.send(data, sizeof(data));
rf95.waitPacketSent();
Serial.println("Se ha enviado un ACK");
digitalWrite(LED, LOW);
}
else
{
Serial.println("Fallo en la recepción");
}
}
}

