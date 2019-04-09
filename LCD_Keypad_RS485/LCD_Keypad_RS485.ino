//Rhomb.io LCD Keypad Shield Flat Keyboard RS-485 Device.
//Programa que escribe en un display LCD lo que se escribe en un teclado numérico.
//El programa es capaz de escribir en el LCD lo que recibie por el bus RS-485.

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27,16,2);
const byte ROWS = 4; //Definir filas del teclado
const byte COLS = 3; //Definir columnas del teclado

char hexaKeys[ROWS][COLS] = { //Matriz del teclado
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3}; 
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); //Instanciar el objeto Keypad

int caracter =0; //Posicionar el cursor
int linea =1;

void setup() {
  Serial.begin(9600);
 // Inicializar el LCD
  lcd.init();
  
  //Encender la luz de fondo.
  lcd.backlight();
}

void loop() {
lcd.setCursor(0, 0);
lcd.print("Int. Parking");  //Mensaje de cabecera en linea 1
lcd.setCursor(caracter, linea);

char customKey = customKeypad.getKey();//Detectar que tecla se presiona
  
  if (customKey){ //Si presiono tecla la imprimo por pantalla y desplazo el cursor 1 caracter
    lcd.print(customKey);
    caracter ++;
    
if (customKey == 35){ //Si presiono almohadilla se borra la pantalla y se reposiciona el cursor
lcd.clear();
caracter = 0;
}

  }
 if (Serial.available()) //Si entran datos por RS-485 los imprimo por pantalla
  {
byte data = Serial.read();
lcd.print(char(data));
caracter ++;
  }

if (caracter ==16){ // Si la segunda linea de LCD se llena, borramos
//lcd.clear();
lcd.setCursor(0, 0);
lcd.print("Int. Parking");
caracter = 0;
linea = 1;
}
  }

