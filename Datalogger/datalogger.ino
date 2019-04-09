

#include <SD.h>
#include <SoftwareSerial.h>
#define LED_PIN 13
#define SERIAl_BAUDS 9600

#define SD_PIN 10  //descomentar para usar con Duino Uno Master Module
//#define SD_PIN 53 // descomentar para usar con Duino Mega 2560 Master Module

#if defined(__AVR_ATmega328P__)
  // Si usamos el 328P definimos los SerialX para evitar errores de undefined
  SoftwareSerial Serial1(12, 13);
  SoftwareSerial Serial2(14, 15);
  SoftwareSerial Serial3(16, 17);
#endif

// lo que devuelve SD.open() lo metemos aquí
File logfile;

// en caso de false nunca entrará en loop
// en el setup se pone a true cuando la sd está lista
bool sdReady = false;

// indice del archivo actual, cada vez que se entra al loop se verifica el tamaño
// del archivo y cuando supera X valor se aumenta este indice para generar nuevo
// fichero
int fileIndex = 0;

// se utiliza en el loop que verifica si los archivos existen
int maxFileIndex = 999;

// tamaño máximo de los ficheros de texto. Cuando se llega al
// máximo de un fechero se crea uno nuevo aumentando fileIndex
int maxFileSize = 1000000;

// las funciones readSerial hacen eso mismo, pero llaman primero a getFileName()
// para obtener el nombre del archio donde deben guardar
void readSerial1();
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  void readSerial2();
  void readSerial3();
  void readSerial4();
#endif

// Como parametro se le pasa un nombre que hace referencia a un puerto Serial,
// por ejemplo S1 para Serial1 o S2 para Serial2, luego utiliza el indice fileIndex
// y con esto genera un nombre de archivo tipo S1-001.txt.
// Con el nombre del archivo se verifica que exista en la sd, si no se crea y
// se da por bueno, si existe se verifica su tamaño, si está por debajo de
// maxFileSize se da por bueno, si no se incrementa fileIndex y se vuelve a
// realizar el proceso, así hasta encontrar un archivo libre o llegar al indice
// 1000
char getLogfile(char *);
void setupSerials();

void setup (void) {
  setupSerials();

  Serial.println(F("START"));

  pinMode(LED_PIN, OUTPUT);
  pinMode(SD_PIN, OUTPUT);

  if (SD.begin(SD_PIN)) {
    Serial.println(F("sd Ready"));
    sdReady = true;
  } else {
    Serial.println(F("Fail SD"));
  }
}

void loop (void) {
  if (sdReady) {
    digitalWrite(LED_PIN, LOW);

    // Si estamos con 328P solo leeremos serial1
    #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
      readSerial1();
    #endif

    // con los MEGA leemos los 4<
    #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
      readSerial1();
      readSerial2();
      readSerial3();
      readSerial4();
    #endif

    // Se podrían añadir definiciones para leonardo
    // if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)

  } else {
    // Algo va mal...
    while (1) {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
  }

  // relax...
  delay(10);
}

void setupSerials () {
  Serial.begin(SERIAl_BAUDS);
  Serial1.begin(SERIAl_BAUDS);
  Serial2.begin(SERIAl_BAUDS);
  Serial3.begin(SERIAl_BAUDS);
  delay(1000);
}

char getLogfile (char * serialName) {
  int x = 1;
  char currentFileName[20];

  // bucle infinito
  while (x > 0) {
    if (fileIndex >= 1000) {
      x = 0;
      strcpy(currentFileName, "0");
    } else {
      // creamos el nombre del fichero, ojo a %03d que relenna el número
      // para obtener 3 valores, ejem 1 → 001, 20 → 020
      sprintf(currentFileName, "%s-%03d.txt", serialName, fileIndex);
      Serial.println(currentFileName);

      if (!SD.exists(currentFileName)) {
        // si el archivo NO existe lo damos por bueno
        Serial.println("NO EXISTE");
        x = 0; // encontrado, salimos del while
        logfile = SD.open(currentFileName, O_CREAT | O_WRITE | O_APPEND);
        if (!logfile) {
          Serial.println("ERR LOGFILE");
        }

      } else {
        // si existe verificamos su tamaño y si excede incrementamos indice
        // para crear nuevo archivo
        logfile = SD.open(currentFileName, O_WRITE | O_APPEND);
        unsigned long size = logfile.size();
        if (size > maxFileSize) {
          // excede, creamos nuevo
          fileIndex++;
          Serial.println("EXISTE EXCEDE");
        } else {
          // devolvemos este archivo
          Serial.println("EXISTE NO EXCEDE");
          x = 0;
        }
      }
    }
    delay(1);
  }
}

void readSerial (HardwareSerial Serial, char * fileName) {
  if (!Serial) return;
  if (!Serial.available()) return;

  getLogfile(fileName);

  if (logfile && Serial.available()) {
    while (Serial.available()) {
      char x = Serial.read();
      logfile.write(x);
    }
    logfile.close();
  }
}

void readSerial1 () {
  if (!Serial) return;
  if (!Serial.available()) return;

  getLogfile((char *) "S1");

  if (logfile && Serial.available()) {
    while (Serial.available()) {
      char x = Serial.read();
      logfile.write(x);
    }
    logfile.close();
  }
}

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
void readSerial2 () {
  if (!Serial1) return;
  if (!Serial1.available()) return;

  getLogfile((char *) "S2");

  if (logfile) {
    logfile.write(Serial1.read());
    logfile.close();
  }
}

void readSerial3 () {
  if (!Serial2) return;
  if (!Serial2.available()) return;

  getLogfile((char *) "S3");

  if (logfile) {
    logfile.write(Serial2.read());
    logfile.close();
  }
}

void readSerial4 () {
  if (!Serial3) return;
  if (!Serial3.available()) return;

  getLogfile((char *) "S4");

  if (logfile) {
    logfile.write(Serial3.read());
    logfile.close();
  }
}
#endif
