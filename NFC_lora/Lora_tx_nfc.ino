// Rhomb.io LoRa TX Totem
// -*- mode: C++ -*-
// Este sketch escanea el UUID de un TAG NFC. Emite un pitido al detectar el TAG.
// Envia por LoRa la info. Espera a recibir un ACK del server. 
// Enciende un LED cuando le llega el ACK para confirmar la recepción. 

// Incluyendo librerías para LoRa
#include <SPI.h>
#include <RH_RF95.h>

// Incluyendo librerías para NFC 
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

// Definiendo los pins necesarios para el LoRa 
#define RFM95_CS 53 // Duino Mega
#define RFM95_RST 9
#define RFM95_INT 3

// Estableciendo la frecuencia a 433 Mhz. El receptor debe tener la misma. 
#define RF95_FREQ 433.0
 
// Declarando un objeto del driver LoRa
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Declarando objetos del driver NFC
PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);

// Declarando la GPIO del Buzzer
const int buzzer = 37; //en placa deberia ser la 3

void setup() 
{
  //pins para el LED RGB
  pinMode(33, OUTPUT);
  pinMode(36, OUTPUT);
 // pinMode(36, OUTPUT);


  
 // pinMode(34, OUTPUT); // Pin para el LED a modo salida
  pinMode(RFM95_RST, OUTPUT); // Pin para el reset del LoRa
  digitalWrite(RFM95_RST, HIGH); // Ponemos a High para que no resetee
 
  while (!Serial);
  Serial.begin(9600);
  delay(100);
 
  Serial.println("Rhomb.io LoRaNFC-TX. Prueba de concepto.");
 
  
  digitalWrite(RFM95_RST, LOW); // Se hace un reset inicial del LoRa
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

 // Se verifica un correcto arranque tras el reseteo
  while (!rf95.init()) {
    Serial.println("Fallo en la inicialización del Rhomb.io LoRa Slave Module");
  while (1);
  }
  Serial.println("Inicialización del Rhomb.io LoRa Slave Module OK!");
 
  // Se verifica un correcto establecimiento de la frecuencia
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("Fallo al establecer la frecuencia");
    while (1);
  }
  Serial.print("Frecuencia establecida (MHz): "); Serial.println(RF95_FREQ);

  // Se establece una potencia de transmisión a 23 db
  rf95.setTxPower(23, false);

// Se inicializan las funciones del lector NFC
  nfc.begin();

// Se verifica que el lector NFC está funcionando
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("No se detecta el lector NFC");
    while (1); // halt
  }
  
  // Si el lector NFC funciona bien, se muestra su versión del firmware
  Serial.println("Lector NFC detectado"); 
  //Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Versión firmware del lector. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // Establece el máximo de intentos de lectura de un TAG
  // Esto previene una espera continua para ver si puede leerse un TAG
  // es el comportamiento por defecto del PN532.
  nfc.setPassiveActivationRetries(0xFF);
  
  // Configurar el lector para que lea TAGS NFC
  nfc.SAMConfig();
    
  Serial.println("Esperando a que se pase un TAG NFC por el lector...");

}
 
int16_t packetnum = 0;  // Contador de paquetes
 
void loop()
{

  
  // Declarando variables locales
  boolean success; // Variable booleana para saber si se ha pasado un TAG
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer para almacenar el UID leido del Tag
  uint8_t uidLength;                        // Longitud del UID (4 bytes)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength); // Se espera a detectar un TAG y entonces esta variable booleana se pone a 1

// Si se lee un TAG
if (success) {
  
  // Gestion del buzzer
  tone(buzzer, 2000); // Se emite un pitido de 2KHz
  delay(100);        //  durante 1 segundo
  noTone(buzzer);     // Se para el pitido
  tone(buzzer, 1000); // Se emite un pitido de 1KHz
  delay(100);        //  durante 1 segundo
  noTone(buzzer);     // Se para el pitido
  // buzzer end
  
  Serial.println("");
  Serial.println("Se ha detectado un TAG NFC");
  // Send a message to rf95_server
  
  char radiopacket[20] = "Hello World #      "; // Estas 4 líneas son para debug. 
  itoa(packetnum++, radiopacket+13, 10);
  Serial.println("Enviando info por LoRa... "); 
  radiopacket[19] = 0;
  
  // Se hace el envío del UID del tag por medio de LoRa
  rf95.send((uint8_t *)uid, 20);
 
  Serial.println("Esperando a que finalize el envío..."); delay(10);
  rf95.waitPacketSent(); // Esperamos a que el paquete se haya enviado
  // Esperamos a recibir el ACK como prueba de que el server ha recibido el paquete
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
 
  Serial.println("Envío realizado OK. Esperando ACK..."); delay(10);
  if (rf95.waitAvailableTimeout(1000))
  { 
    // Aquí es donde se recibe el ACK y se almacena en la variable buf  
    if (rf95.recv(buf, &len) && strcmp(buf, "rhomb745") == 0) // Aquí se evalua que el ACK recibido sea igual al que envía el server.
   {
      Serial.print("ACK Recibido! ");
      Serial.println((char*)buf); // Se muestra por pantalla el contenido del ACK
      Serial.print("RSSI: "); // Se muestra por pantalla la potencia de señal RF
      Serial.println(rf95.lastRssi(), DEC);    

//LED RGB
  digitalWrite(36, HIGH);
  delay(1000);
  digitalWrite(36, LOW);
  delay(1000);
// end led  
    }
    else // En caso de no haber recibido el ACK, entramos en este bucle y mostramos los errores por pantalla
    {
      Serial.print("Fallo. El ACK recibido no es correcto. Debería ser rhomb745 y es: ");
      Serial.println((char*)buf);
      digitalWrite(33, HIGH);
      delay(1000);
      digitalWrite(33, LOW);
      delay(1000);
         }
  }
  else
  {
    Serial.println("No se ha recibido el ACK. Hay un server a la escucha?");
    digitalWrite(33, HIGH);
    delay(1000);
    digitalWrite(33, LOW);
    delay(1000);
  }
  delay(1000);
}

}
