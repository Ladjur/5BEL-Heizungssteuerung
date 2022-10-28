/****************void ReadTemp wurde aus Zeitgründen von Aris Mandolini(Siraman25) kopiert****************/
#include <Arduino.h>
#include <math.h>
#define VREAD A0 // Definition vom Input-PIN des NTC im Spannungsteiler
#define OUTPUT_TRANS 4  // Definition vom Input-PIN für den Transistor

const bool MEASURING_U2_LOWSIDE = true; // Wenn true -> Lowside, false -> Highside
//Diese const werden in der funktion ReadTemp verwendet
const int ADC_VREF = 5;     // Referenzspannung des ADCs
const int ADC_STEPS = 1024; // Referenzschritte des ADCs
const int RESISTANCE = 26260; // Anderer Widerstand in Ohm
//Diese const werden in der funktion loop verwendet verwendet
const int TIMEDELAY = 1000; // Zeit in ms zwischen Messungen

int voltageSteps = 0;         // Gemesste ADC-Schritte
float voltage = 0;            // Gemesste Spannung
uint64_t timestampMillis = 0; // Timestamp für Messungen
float R2 = 0;                 // Berechneter Widerstand R2
float R1 = 0;                 // Berechneter Widerstand R1

const float R_N = 10000;       // Nennwiederstand (Datasheet)
const float B = 3435;          // Materialkonstante (Datasheet)
const float T_N = 25 + 273.15; // Nenntemperatur in KELVIN! (Datasheet)
float temperature = 0;         // Berechnete Temperatur

int TEMP_IDEAL = 30;
int TEMP_DELTA = 5;

String HeizData = "";         //Einlesung des Benutzers
int Mode = 2; // 1 = manuell 2 = auto
String mitRead = "";          //Zwischenspeicher ob Betrieb in Manuell oder Auto ist
int mitReadint = 0;           //Umwandeln von HeizData von String to int


/****const int ADC_VREF = 5; wird in ReadTemp verwendet***********/
/****const int ADC_STEPS = 1024; wird in ReadTemp verwendet*******/
/****const int RESISTANCE = 26260; wird in ReadTemp verwendet*****/
void ReadTemp()           //Berechnung der Temperatur
{
  if (Serial.available())
  {
    HeizData = Serial.readString();
    if (HeizData.charAt(0) == 'm')
    {
      Mode = 1;
      Serial.println("Mode Manu");
      if (temperature < TEMP_IDEAL + TEMP_DELTA)
      {
        digitalWrite(OUTPUT_TRANS, 1);
      }
    }
    else if (HeizData.charAt(0) == 'a')
    {
      Mode = 2;
      Serial.println("Mode Auto");
    }
    mitRead = HeizData.charAt(5);
    mitRead = mitRead + HeizData.charAt(6);
    TEMP_DELTA = mitRead.toInt();
    mitRead = HeizData.charAt(8);
    mitRead = mitRead + HeizData.charAt(9);
    TEMP_IDEAL = mitRead.toInt();
    Serial.println(TEMP_IDEAL);
    Serial.println(TEMP_DELTA);
  }
}
void SwitchAutoManu()     //Wechsel von Automatic auf Manuellenbetrieb
{
  voltageSteps = analogRead(VREAD);
  voltage = ((float)ADC_VREF / ADC_STEPS) * voltageSteps;
  if (MEASURING_U2_LOWSIDE)
  {
    // Serial.println(voltageSteps);
    // Serial.println(voltage);

    R2 = (voltage * RESISTANCE) / (ADC_VREF - voltage);
    Serial.print("Lowside enabled. R2: ");
    Serial.println(R2);

    temperature = (1 / ((1 / T_N) + ((1 / B) * log(R2 / R_N)))) - 273.15;
    Serial.print("Temp BC: ");
    Serial.print(temperature);

    temperature = (temperature + 5.013) / 1.213;

    Serial.print(" °C // Temp AC: ");
    Serial.print(temperature);
    Serial.println(" °C");
  }
  else
  {
    R1 = ((ADC_VREF - voltage) * RESISTANCE) / voltage;
    Serial.print("Highside enabled. R1: ");
    Serial.println(R1);

    temperature = (1 / ((1 / T_N) + ((1 / B) * log(R1 / R_N)))) - 273.15;

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
  }
}
void Automatic()          //Ein Ausschalten der Heizung bei Automatischen Betrieb
{
  if (Mode == 2)
  {
    if (temperature < TEMP_IDEAL - TEMP_DELTA)
    {
      digitalWrite(OUTPUT_TRANS, 1);
    }
    if (temperature > TEMP_IDEAL + TEMP_DELTA)
    {
      digitalWrite(OUTPUT_TRANS, 0);
    }
  }
}
void Manuell()            //Ein und Ausschalten der Heizung bei Manuellem Betrieb
{
  if (Mode == 1)
  {
    if (temperature > TEMP_IDEAL + TEMP_DELTA)
    {
      digitalWrite(OUTPUT_TRANS, 0);
    }
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(VREAD, INPUT);
  pinMode(OUTPUT_TRANS, OUTPUT);
}
//const int TIMEDELAY = 1000; wird im main loop verwendet
void loop()
{
  /// Umschaltung Manuell Auto
  SwitchAutoManu();
  ////Berechnung Temperatur
  if (millis() - timestampMillis > TIMEDELAY)
  {
    ReadTemp();
    Automatic();
    Manuell();
    timestampMillis = millis();
  }
}