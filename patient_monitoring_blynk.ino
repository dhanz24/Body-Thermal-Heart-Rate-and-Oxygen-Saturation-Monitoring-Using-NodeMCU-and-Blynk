/*
  #  simple SpO2 plotter for MH-ET LIVE MAX30102 breakout board and ESP32 devkit-C
  Using Sparkfun MAX3010X library
  https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library
  ESP32_MAX30102_simple-SpO2_plotter.ino
  by coniferconifer Copyright 2020
  LICENSED under Apache License 2.0
  Version 1.0
  Shows SpO2 and the user's heart beat on Arduino's serial plotter.
  No display hardware is required.
  This program should not be used for medical purposes.
  I wrote this to learn how SpO2 can be measured and pay tributes for the inventors.
  Pulse oximetry was developed in 1972, by Takuo Aoyagi and Michio Kishi,
  bioengineers, at Nihon Kohden in Japan.
  https://ethw.org/Takuo_Aoyagi
  Since MH-ET LIVE MAX30102 breakout board seems outputting IR and RED swapped.
  red = particleSensor.getFIFOIR();
  ir = particleSensor.getFIFORed();
  is used in my code. If you have Sparkfun's MAX30105 breakout board , try to 
  use #define MAX30105
  ## Tips:
  SpO2 is calculated as R=((square root means or Red/Red average )/((square root means of IR)/IR average))
  SpO2 = -23.3 * (R - 0.4) + 100;
  // taken from a graph in https://ww1.microchip.com/downloads/jp/AppNotes/00001525B_JP.pdf
  // https://ww1.microchip.com/downloads/en/Appnotes/00001525B.pdf
  ## Instructions:
  0) Install Sparkfun's MAX3010X library
  1) Load code onto ESP32 with MH-ET LIVE MAX30102 board
  2) Put MAX30102 board in plastic bag and insulates from your finger.
     and attach sensor to your finger tip
  3) Run this program by pressing reset botton on ESP32
  4) Wait for 3 seconds and Open Arduino IDE Tools->'Serial Plotter'
     Make sure the drop down is set to 115200 baud
  5) Search the best position and presure for the sensor by watching
     the blips on Arduino's serial plotter
     I recommend to place LED under the backside of nail and wrap you
     finger and the sensor by rubber band softly.
  6) Checkout the SpO2 and blips by seeing serial Plotter
     100%,95%,90%,85% SpO2 lines are always drawn on the plotter
  ## Hardware Connections (Breakoutboard to ESP32 Arduino):
  -VIN = 3.3V
  -GND = GND
  -SDA = 21 (or SDA)
  -SCL = 22 (or SCL)
  -INT = Not connected
  this script also works on Arduino nao
  ## Hardware Connections (Breakoutboard to Arduino nano): experimental
  -VIN = 3.3V 
  -GND = GND
  -SDA = A4 (or SDA)
  -SCL = A5 (or SCL)
  -INT = Not connected
  
  ## Trouble Shooting:
  Make sure to solder jumper on 3V3 side.
  if you forget this, I2C does not work and can not find MAX30102.
  says "MAX30102 was not found. Please check wiring/power."
*/

// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID   "TMPLAI53fpKs"
#define BLYNK_DEVICE_NAME   "patient monitoring"
#define BLYNK_AUTH_TOKEN    "Z0xkKGCW_4MjM2EMa9ulpfXwblgwEKDL"

// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include "MAX30105.h" //sparkfun MAX3010X library
MAX30105 particleSensor;
//#define MAX30105 //if you have Sparkfun's MAX30105 breakout board , try #define MAX30105 

int ThermistorPin = A0;  // sensor LM35DZ
float tempc;
float svoltage;

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "pona";
char pass[] = "dhanz2423";

BlynkTimer timer;

#define USEFIFO


void setup()
{
  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass);  //memulai program blynk
  
  Serial.println("Initializing...");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30102 was not found. Please check wiring/power/solder jumper at MH-ET LIVE MAX30102 board. ");
    while (1);
  }

  //Setup to sense a nice looking saw tooth on the plotter
  byte ledBrightness = 0x7F; //Options: 0=Off to 255=50mA
  byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  //Options: 1 = IR only, 2 = Red + IR on MH-ET LIVE MAX30102 board
  int sampleRate = 200; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 16384; //Options: 2048, 4096, 8192, 16384
  // Set up the wanted parameters
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
 timer.setInterval(500, sendUptime);
}

double avered = 0; double aveir = 0;
double sumirrms = 0;
double sumredrms = 0;
int i = 0;
int Num = 100;//calculate SpO2 by this sampling interval

double ESpO2 = 95.0;//initial value of estimated SpO2
double FSpO2 = 0.7; //filter factor for estimated SpO2
double frate = 0.95; //low pass filter for IR/red LED value to eliminate AC component
#define TIMETOBOOT 3000 // wait for this time(msec) to output SpO2
#define SCALE 88.0 //adjust to display heart beat and SpO2 in the same scale
#define SAMPLING 5 //if you want to see heart beat more precisely , set SAMPLING to 1
#define FINGER_ON 30000 // if red signal is lower than this , it indicates your finger is not on the sensor
#define MINIMUM_SPO2 80.0

// Heart Rate Monitor by interval of zero crossing at falling edge
// max 180bpm - min 45bpm
#define FINGER_ON 50000 // if ir signal is lower than this , it indicates your finger is not on the sensor
#define LED_PERIOD 100 // light up LED for this period in msec when zero crossing is found for filtered IR signal
#define MAX_BPS 180
#define MIN_BPS 45
double HRM_estimator( double fir , double aveir)
{
  static double fbpmrate = 0.95; // low pass filter coefficient for HRM in bpm
  static uint32_t crosstime = 0; //falling edge , zero crossing time in msec
  static uint32_t crosstime_prev = 0;//previous falling edge , zero crossing time in msec
  static double bpm = 60.0;
  static double ebpm = 60.0;
  static double eir = 0.0; //estimated lowpass filtered IR signal to find falling edge without notch
  static double firrate = 0.85; //IR filter coefficient to remove notch , should be smaller than fRate
  static double eir_prev = 0.0;


  // Heart Rate Monitor by finding falling edge
  eir = eir * firrate + fir * (1.0 - firrate); //estimated IR : low pass filtered IR signal
  if ( ((eir - aveir) * (eir_prev - aveir) < 0 ) && ((eir - aveir) < 0.0)) { //find zero cross at falling edge
    crosstime = millis();//system time in msec of falling edge
    //Serial.print(crosstime); Serial.print(","); Serial.println(crosstime_prev);
    if ( ((crosstime - crosstime_prev ) > (60 * 1000 / MAX_BPS)) && ((crosstime - crosstime_prev ) < (60 * 1000 / MIN_BPS)) ) {
      bpm = 60.0 * 1000.0 / (double)(crosstime - crosstime_prev) ; //get bpm
      //   Serial.println("crossed");
      ebpm = ebpm * fbpmrate + (1.0 - fbpmrate) * bpm;//estimated bpm by low pass filtered
#ifdef LED_SOUND_INDICATOR
      if (aveIr > FINGER_ON) {
        digitalWrite(LEDPORT, HIGH);
        tone(BLIPSOUND - (100.0 - eSpO2) * 10.0); //when SpO2=80% BLIPSOUND drops 200Hz to indicate anormaly
      }
#endif
    } else {
      //Serial.println("faild to find falling edge");
    }
    crosstime_prev = crosstime;
  }
  eir_prev = eir;
#ifdef LED_SOUND_INDICATOR
  if (millis() > (crosstime + LED_PERIOD)) {
    if ( aveIr > FINGER_ON ) {
      digitalWrite(LEDPORT, LOW);
      noTone();
    }
  }
#endif
  return (ebpm);
}

double Ebpm;//estimated Heart Rate (bpm)

void sendUptime()
{
  Blynk.virtualWrite(V3, tempc);  //mengirimkan data suhu ke blynk
  Blynk.virtualWrite(V4, Ebpm);  //mengirimkan data bpm ke blynk
  Blynk.virtualWrite(V5, ESpO2);  //mengirimkan data spo ke blynk
}

void loop()
{
  Blynk.run();
  timer.run();
  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!


  uint32_t ir, red , green;
  double fred, fir;
  double SpO2 = 0; //raw SpO2 before low pass filtered
  

#ifdef USEFIFO
  particleSensor.check(); //Check the sensor, read up to 3 samples

  while (particleSensor.available()) {//do we have new data
#ifdef MAX30105
   red = particleSensor.getFIFORed(); //Sparkfun's MAX30105
    ir = particleSensor.getFIFOIR();  //Sparkfun's MAX30105
#else
    red = particleSensor.getFIFOIR(); //why getFOFOIR output Red data by MAX30102 on MH-ET LIVE breakout board
    ir = particleSensor.getFIFORed(); //why getFIFORed output IR data by MAX30102 on MH-ET LIVE breakout board
#endif
    i++;
    fred = (double)red;
    fir = (double)ir;
    avered = avered * frate + (double)red * (1.0 - frate);//average red level by low pass filter
    aveir = aveir * frate + (double)ir * (1.0 - frate); //average IR level by low pass filter
    sumredrms += (fred - avered) * (fred - avered); //square sum of alternate component of red level
    sumirrms += (fir - aveir) * (fir - aveir);//square sum of alternate component of IR level
   
    Ebpm = HRM_estimator(fir, aveir); //Ebpm is estimated BPM

 //Pembacaan sensor suhu LM35 DZ  
 float xval = analogRead(ThermistorPin);  //output adc dalam format decimal
 svoltage = (xval/1024.0)*3300;  // rumus menghitung suhu dari arus, 3300 yaitu 3.3V
 tempc = svoltage/10;  //factor datasheet 10mV / derajat C

    if ((i % SAMPLING) == 0) {//slow down graph plotting speed for arduino Serial plotter by thin out
      if ( millis() > TIMETOBOOT) {
        float ir_forGraph = (2.0 * fir - aveir) / aveir * SCALE;
        float red_forGraph = (2.0 * fred - avered) / avered * SCALE;
        //trancation for Serial plotter's autoscaling
        if ( ir_forGraph > 100.0) ir_forGraph = 100.0;
        if ( ir_forGraph < 80.0) ir_forGraph = 80.0;
        if ( red_forGraph > 100.0 ) red_forGraph = 100.0;
        if ( red_forGraph < 80.0 ) red_forGraph = 80.0;
        //        Serial.print(red); Serial.print(","); Serial.print(ir);Serial.print(".");
        if (ir < FINGER_ON) ESpO2 = MINIMUM_SPO2; //indicator for finger detached
        
        //Serial.print(bpm);// raw Heart Rate Monitor in bpm
        //Serial.print(",");
        Serial.print(Ebpm);// estimated Heart Rate Monitor in bpm
        Serial.print(",");
        

       Serial.print("Suhu: "); 
       Serial.print(tempc);
       Serial.print("C ");  


        Serial.print(ir_forGraph); // to display pulse wave at the same time with SpO2 data
        Serial.print(","); Serial.print(red_forGraph); // to display pulse wave at the same time with SpO2 data
        Serial.print(",");
        Serial.print(ESpO2); //low pass filtered SpO2
        Serial.print(","); Serial.print(85.0); //reference SpO2 line
        Serial.print(","); Serial.print(90.0); //warning SpO2 line
        Serial.print(","); Serial.print(95.0); //safe SpO2 line
        Serial.print(","); Serial.println(100.0); //max SpO2 line
      }
    }
    if ((i % Num) == 0) {
      double R = (sqrt(sumredrms) / avered) / (sqrt(sumirrms) / aveir);
      // Serial.println(R);
      SpO2 = -23.3 * (R - 0.4) + 100; //http://ww1.microchip.com/downloads/jp/AppNotes/00001525B_JP.pdf
      ESpO2 = FSpO2 * ESpO2 + (1.0 - FSpO2) * SpO2;//low pass filter
      //  Serial.print(SpO2);Serial.print(",");Serial.println(ESpO2);
      sumredrms = 0.0; sumirrms = 0.0; i = 0;
      break;
    }
    particleSensor.nextSample(); //We're finished with this sample so move to next sample
    //Serial.println(SpO2);
  }
#else

  while (1) {//do we have new data
#ifdef MAX30105
   red = particleSensor.getRed();  //Sparkfun's MAX30105
    ir = particleSensor.getIR();  //Sparkfun's MAX30105
#else
    red = particleSensor.getIR(); //why getFOFOIR outputs Red data by MAX30102 on MH-ET LIVE breakout board
    ir = particleSensor.getRed(); //why getFIFORed outputs IR data by MAX30102 on MH-ET LIVE breakout board
#endif
    i++;
    fred = (double)red;
    fir = (double)ir;
    avered = avered * frate + (double)red * (1.0 - frate);//average red level by low pass filter
    aveir = aveir * frate + (double)ir * (1.0 - frate); //average IR level by low pass filter
    sumredrms += (fred - avered) * (fred - avered); //square sum of alternate component of red level
    sumirrms += (fir - aveir) * (fir - aveir);//square sum of alternate component of IR level
    if ((i % SAMPLING) == 0) {//slow down graph plotting speed for arduino IDE toos menu by thin out
      //#if 0
      if ( millis() > TIMETOBOOT) {
        float ir_forGraph = (2.0 * fir - aveir) / aveir * SCALE;
        float red_forGraph = (2.0 * fred - avered) / avered * SCALE;
        //trancation for Serial plotter's autoscaling
        if ( ir_forGraph > 100.0) ir_forGraph = 100.0;
        if ( ir_forGraph < 80.0) ir_forGraph = 80.0;
        if ( red_forGraph > 100.0 ) red_forGraph = 100.0;
        if ( red_forGraph < 80.0 ) red_forGraph = 80.0;
        //        Serial.print(red); Serial.print(","); Serial.print(ir);Serial.print(".");
        if (ir < FINGER_ON) ESpO2 = MINIMUM_SPO2; //indicator for finger detached
        
        //Serial.print(bpm);// raw Heart Rate Monitor in bpm
        //Serial.print(",");
        Serial.print(Ebpm);// estimated Heart Rate Monitor in bpm
        Serial.print(",");
        
        Serial.print((2.0 * fir - aveir) / aveir * SCALE); // to display pulse wave at the same time with SpO2 data
        Serial.print(","); Serial.print((2.0 * fred - avered) / avered * SCALE); // to display pulse wave at the same time with SpO2 data
        Serial.print(","); Serial.print(ESpO2); //low pass filtered SpO2
        Serial.print(","); Serial.print(85.0); //
        Serial.print(","); Serial.print(90.0); //warning SpO2 line
        Serial.print(","); Serial.print(95.0); //safe SpO2 line
        Serial.print(","); Serial.println(100.0); //max SpO2 line
        //#endif
      }
    }
    if ((i % Num) == 0) {
      double R = (sqrt(sumredrms) / avered) / (sqrt(sumirrms) / aveir);
      // Serial.println(R);
      SpO2 = -23.3 * (R - 0.4) + 100; //http://ww1.microchip.com/downloads/jp/AppNotes/00001525B_JP.pdf
      ESpO2 = FSpO2 * ESpO2 + (1.0 - FSpO2) * SpO2;
      //  Serial.print(SpO2);Serial.print(",");Serial.println(ESpO2);
      sumredrms = 0.0; sumirrms = 0.0; i = 0;
      break;
    }
    particleSensor.nextSample(); //We're finished with this sample so move to next sample
    //Serial.println(SpO2);
  }
#endif
}
