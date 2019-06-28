#include <EEPROM.h>

// Calibration
int LargestDirectionValue;
// Wind speed
#define MainPeriod 450
long previousMillis = 0;
volatile unsigned long duration=0;
volatile unsigned int pulsecount=0;
volatile unsigned long previousMicros=0;
///////////////////////

void calibration()
{
  Serial.println("Calibration started!");
  while(analogRead(A0)+10 >= LargestDirectionValue)
  {
    if(LargestDirectionValue < analogRead(A0))
    {
      LargestDirectionValue = analogRead(A0);
    }
    delayMicroseconds(250);
  }
  EEPROM.write(0, LargestDirectionValue & 0xFF);
  EEPROM.write(1, LargestDirectionValue >> 8);
  Serial.println("Calibrated!");
}

void setup() {
  Serial.begin(9600);
  pinMode(7, INPUT);
  if(digitalRead(7) == HIGH)
  {
    calibration();
  }
  attachInterrupt(digitalPinToInterrupt(2), windFreqDuration, RISING);

  LargestDirectionValue = EEPROM.read(1);
  LargestDirectionValue <<= 8;
  LargestDirectionValue |= EEPROM.read(0);
}

String AngleToDirection(float grdi)
{
  if(grdi >= 348.75 or grdi <= 11.25)
  {
    return "North";
  }
  else if(grdi >= 78.75 and grdi <= 101.25)
  {
    return "East";
  }
  else if(grdi >= 168.75 and grdi <= 191.25)
  {
    return "South";
  }
  else if(grdi >= 258.75 and grdi <= 281.25)
  {
    return "West";
  }
  else
  {
    return "";
  }
}

void loop() {
  
  // Wind direction measure //
  
  int value = analogRead(A0);
  float angle = ((float)value / (float)LargestDirectionValue) * 360.0;
  
  // Wind speed measure //
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= MainPeriod) 
  {
    noInterrupts();
    previousMillis = currentMillis;   
    unsigned long _duration = duration;
    unsigned long _pulsecount = pulsecount;
    duration = 0;
    pulsecount = 0;
    float Freq = 1000000 / float(_duration);
    Freq *= _pulsecount; // calculate F

    float windSpeed = 0.765 * Freq + 0.35;
    Serial.print("Direction");
    Serial.print(" ");
    Serial.print("Angle");
    Serial.print(" ");
    Serial.println("Speed");
    Serial.print(AngleToDirection(angle));
    Serial.print("    ");
    Serial.print(angle);
    Serial.print(" ");
    Serial.print(windSpeed);
    Serial.println("m/s");
    interrupts();
  }
  
  delay(250);
}

void windFreqDuration()
{
  duration += micros() - previousMicros;
  previousMicros = micros();
  pulsecount++;
}
