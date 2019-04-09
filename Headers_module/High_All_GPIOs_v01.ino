//sketch para poner a nivel alto todas las GPIOs del microcontrolador de arduino que toque.
//NOTA: Antes de nada comentar los define que sean de microcontroladores distintos al que se esta usando.
#define DUINO_MEGA
//#define DUINO_UNO   // valido para S100 Duino UNO
//#define S100_UNO
//#define S100_LEONARDO

#ifdef DUINO_MEGA
  #define i_Max 100
  #define A_Max A15
#endif
#ifdef DUINO_UNO
  #define i_Max 60
  #define A_Max A7
#endif
#ifdef S100_LEONARDO
  #define i_Max 60
  #define A_Max A11
#endif

int i;

void setup() {
  for(i=0;i<i_Max;i++)
  {
    pinMode(i, OUTPUT);    
  }
   for(i=A0;i<A_Max;i++)
  {
    pinMode(i, OUTPUT);    
  }
}

void loop() {
  
  for(i=0;i<i_Max;i++)  {
     digitalWrite(i, HIGH);   // turn the LED on (HIGH is the voltage level)
  }
  for(i=A0;i<A_Max;i++)  {
     digitalWrite(i, HIGH);   // turn the LED on (HIGH is the voltage level)
  }
}
