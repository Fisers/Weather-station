// Kalibrācija
int virzienaBiggestValue;
// Vēja ātrumam
#define MainPeriod 450
long previousMillis = 0;
volatile unsigned long duration=0;
volatile unsigned int pulsecount=0;
volatile unsigned long previousMicros=0;
///////////////////////

void calibration()
{
  Serial.println("Calibration started!");
  while(analogRead(A0)+10 >= virzienaBiggestValue)
  {
    virzienaBiggestValue = analogRead(A0);
    Serial.println(virzienaBiggestValue);
  }
  Serial.println("Calibrated!");
}

void setup() {
  Serial.begin(9600);
  calibration();
  attachInterrupt(digitalPinToInterrupt(2), windFreqDuration, RISING);
}

String graadiUzVirzienu(float grdi)
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
  
  // Vēja virziena noteikšana //
  
  int value = analogRead(A0);
  float graadi = ((float)value / (float)virzienaBiggestValue) * 360.0;
  
  // Vēja ātruma noteikšana //
  
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
    Serial.print("Virziens");
    Serial.print(" ");
    Serial.print("Grādi");
    Serial.print(" ");
    Serial.println("Ātrums");
    Serial.print(graadiUzVirzienu(graadi));
    Serial.print("    ");
    Serial.print(graadi);
    Serial.print(" ");
    Serial.print(windSpeed);
    Serial.println("m/s");
    interrupts();
  }
  
  delay(250);
}

void windFreqDuration() // interrupt handler
{
  duration += micros() - previousMicros;
  previousMicros = micros();
  pulsecount++;
}
