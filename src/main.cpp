#include <Arduino.h>
#define SCK_PIN 18
#define STRAW_1_PIN 4
#define STRAW_2_PIN 2
#define STRAW_3_PIN 15
#define STRAW_4_PIN 14

int signalToState(int signal)
{
  if (signal > 12582912)
  {
    return 2;
  }
  else if (signal < 4194304)
  {
    return 0;
  }
  return 1;
}

long readPin(int pin)
{
  while (digitalRead(pin)) {}

  // read 24 bits
  long result = 0;
  for (int i = 0; i < 24; i++) {
    digitalWrite(SCK_PIN, HIGH);
    digitalWrite(SCK_PIN, LOW);
    result = result << 1;
    if (digitalRead(pin)) {
      result++;
    }
  }

  // get the 2s compliment
  result = result ^ 0x800000;
  return result;
}

void setup() {
  pinMode(STRAW_1_PIN, INPUT);   // Connect HX710 OUTs to Arduino pins 4, 5, 6, 7
  pinMode(STRAW_2_PIN, INPUT); 
  pinMode(STRAW_3_PIN, INPUT); 
  pinMode(STRAW_4_PIN, INPUT); 
  pinMode(SCK_PIN, OUTPUT);  // Connect HX710 SCK to Arduino pin 3
  Serial.begin(9600);
}

void loop() {
  long result1 = readPin(STRAW_1_PIN);
  long result2 = readPin(STRAW_2_PIN);
  long result3 = readPin(STRAW_3_PIN);
  long result4 = readPin(STRAW_4_PIN);

  // pulse the clock line 3 times to start the next pressure reading
  for (char i = 0; i < 3; i++) {
    digitalWrite(SCK_PIN, HIGH);
    digitalWrite(SCK_PIN, LOW);
  }

  // display pressure
  //Serial.println(result);
  Serial.println(signalToState(result1));
  Serial.println(signalToState(result2));
  Serial.println(signalToState(result3));
  Serial.println(signalToState(result4));
  Serial.println("ENDENDENDEND");
  delay(1000);
}