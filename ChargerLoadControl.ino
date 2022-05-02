// Charger Load Control
int Version1 = 1;
int Version2 = 0;
int Version3 = 0;

//----------------------------
//History
// 0.1.000 18.03.2022 Erste Version Test Isim PWM Zuordnung
// 0.2.000 20.03.2022 RS485 Implementierung
// 0.3.000 27.03.2022 Erste funktionsfähige Version
// 0.4.000 27.03.2022 kWh Zähler einlesen, neues Display Management
// 0.5.000 02.04.2022 Neue Funktion für PWM-to-mA, neue Eingangsbelegung wegen PWM Frequenz 970Hz auf Pin 6 statt 470Hz auf Pin3
// 0.6.000 03.04.2022 kWh-Zähler erweitert um Verbrauchszähler
// 1.0.000 03.04.2022 Erfolgreicher Test

// Display
//A5=SCL, A4=SDA
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4); // I2C Display auf Adresse 0x27 instanzieren

// RS485 Modbus zum ABB kWh-Zähler - Modbus RTU, 19200 baud, Parity: even, Stopbit:1
// D8=RxD-RO, D9=TxD-DI, D3=DE-RE
#include <SoftwareSerial.h>
#define SSerialTxControl 7  //RS485 Direction control
#define SSerialRX        8  //Serial Receive pin
#define SSerialTX        9  //Serial Transmit pin
#define RS485Transmit    HIGH
#define RS485Receive     LOW
SoftwareSerial RS485Serial(SSerialRX, SSerialTX);
int byteRS485Received;

//--- Eingaenge
#define BN_LEVEL_DN_PIN 2    // Pin 2 als Eingang: Ladestufen Runtertasten
#define BN_LEVEL_UP_PIN 3    // Pin 3 als Eingang: Ladestufen Hochtasten
#define BN_DISPLAY_PIN  5    // Pin 5 als Eingang: Display Seiten wechseln

//--- Ausgaenge
#define PWM_OUT_PIN 6 // Pin 6 für PWM Signal

// Init
bool Init = false;
int Cyclecount1 = 0;
int Cyclecount2 = 0;

// Komponenten von Isim
int Isim = 0;  // Simulierter Strom in 0,01A - wird an den Wallbox CT geschickt
int Imax = 2000; // Maximaler Hausstrom in 0,01A - über DIP Schalter in Wallbox einstellen: 20A
int Iset = 0;  // Vorgabe Ladestrom in 0,01A - Stufenweise 0,6,7,8,..,15,16
int Iwb = 0;   // Wallbox Strom in 0,01A

int LoadLevel = 0;  // Ladestufen 0-12: 0=0A,1=6A,2=7A,3=8A...,10=15A,11=16A,12=0 mA auf CT

// Historiemerker
int IsimHist = 1;
int IsetHist = 1;
int IwbHist = 1;
int LoadLevelHist = 1;

// PWM Signal
int PWMSignal = 0; // PWM Signal 0-255
int PWMSignalHist = 1;

// Taster
#define DEBOUNCETIME 200    // Entprellverzögerung
int BnLevelUp = 0;
int BnLevelUpHist = 0;
unsigned long BnLevelUpTime = 0;
int BnLevelDn = 0;
int BnLevelDnHist = 0;
unsigned long BnLevelDnTime = 0;
int BnDisplay = 0;
int BnDisplayHist = 0;
unsigned long BnDisplayTime = 0;

// Eingabe am seriellen Monitor
String inString = "";

// Display
char sBuffer[20];
int DisplayNr = 0;
bool DisplayNow = false;
unsigned long BlinkTime;
#define BLINKINTERVAL 800 // ms
bool BlinkPuls = false;
int BlinkOn = 1;

// Zykluszeit
unsigned long MillisHist;
unsigned int CycleTime;
unsigned int CycleTimeMax = 0;
unsigned int CycleTimeMin = 1000;
#define CYCLETIMERESETINTERVAL 5000 // ms
unsigned long CycleTimeMReset;

// PWM Wechselrichter
#define PWMPULSINTERVAL 130 // ms Abstand zwischen den Strom-Pulsen 
int PWMPuls = 1;
unsigned long PWMPulsTime = 0;

// Modbus Daten abhören
byte readData[25];//Array für die empfangenen Bytes
bool DataReceived = false;
// Strom/Leistung aus Modbus Daten extrahieren:  Request 8 Byte + Response 17 Byte
int CurrentL1 = 0; // Gemessener Phasenstrom L1 in 0.01 A
int CurrentL2 = 0; // Gemessener Phasenstrom L2 in 0.01 A
int CurrentL3 = 0; // Gemessener Phasenstrom L3 in 0.01 A
int CurrentL1Hist,CurrentL2Hist,CurrentL3Hist = 1;
int PowerL1,PowerL2,PowerL3;
bool CarIsLoading = false;
bool LoadStartPuls = false;

// kWh-Zähler aus Modbus Daten extrahieren:  Request 8 Byte + Response 17 Byte
unsigned long kWhCounter = 0; // Energiezähler in 0.01 kWh
unsigned long kWhCounterHist = 1;
unsigned long kWhCounterStart = 0; // Energiezähler beim Start der Ladung in 0.01 kWh
unsigned long kWhCounterStartHist = 1;
unsigned long kWhLoaded = 0;
//-------------------------------------
//####################

void setup() {
  // Serieller Monitor
  Serial.begin( 9600 );    // Start the serial port
  lcd.init(); //Im Setup wird der LCD gestartet 
  lcd.backlight(); //Hintergrundbeleuchtung einschalten
  
  // Serielle RS485
  pinMode(SSerialTxControl, OUTPUT);
  digitalWrite(SSerialTxControl, RS485Receive);  // Init Transceiver   
  RS485Serial.begin(19200);   // set the data rate, Start the software serial port
  RS485Serial.setTimeout(100);
  
  //--- Eingaenge
  pinMode(BN_LEVEL_UP_PIN,INPUT);
  digitalWrite(BN_LEVEL_UP_PIN, HIGH); // Pullup-Widerstand einschalten
  pinMode(BN_LEVEL_DN_PIN,INPUT);
  digitalWrite(BN_LEVEL_DN_PIN, HIGH); // Pullup-Widerstand einschalten
  pinMode(BN_DISPLAY_PIN,INPUT);
  digitalWrite(BN_DISPLAY_PIN, HIGH); // Pullup-Widerstand einschalten
  
  //--- Ausgaenge
  pinMode(PWM_OUT_PIN, OUTPUT);

  Serial.println(F("Charger Load Control starting"));

  // Display Ausgabe Zeile 1 Überschrift
  lcd.setCursor(0, 0);lcd.print(F("CHARGER LOAD CONTROL")); 
}

//####################
void loop() {

// Blinksignale ----------------------------------------
BlinkPuls = false;
if (millis() > BlinkTime){
  BlinkTime = millis() + BLINKINTERVAL;
  BlinkPuls = true;
  BlinkOn = -BlinkOn;
}

// Zykluszeit messen ----------------------------------------
CycleTime = millis() - MillisHist;
MillisHist = millis();
if (CycleTime > CycleTimeMax){
  CycleTimeMax = CycleTime;
}
if ( (CycleTime < CycleTimeMin) && (CycleTime > 0) ){
  CycleTimeMin = CycleTime;
}
if (millis() > CycleTimeMReset){
  CycleTimeMReset = millis() + CYCLETIMERESETINTERVAL;
  CycleTimeMax = 0;
  CycleTimeMin = 1000;
}

// Display 0 ----------------------------------------
if (DisplayNr == 0){
  // Display Ausgabe Zeile 2 links
  if ( (BlinkPuls == true) || DisplayNow == true) {
    if ( (CarIsLoading == true) && (BlinkOn == 1) ) {
      lcd.setCursor(0, 1);lcd.print(F("loading.."));
    }else{
      lcd.setCursor(0, 1);lcd.print(F("         "));
    }
  }

  // Display Ausgabe Zeile 3 links
  if ( (LoadLevel != LoadLevelHist) || DisplayNow == true ) {
    sprintf(sBuffer,"Level=%2d",LoadLevel);
    lcd.setCursor(0, 2);lcd.print(sBuffer);
    LoadLevelHist=LoadLevel;
  }

  // Display Ausgabe Zeile 2 rechts
  if ( (Iset != IsetHist) || DisplayNow == true ) {
    sprintf(sBuffer,"Iset=%2d.%02dA",Iset/100,abs(Iset%100));
    lcd.setCursor(9, 1);lcd.print(sBuffer);
    IsetHist=Iset;
  }
  // Display Ausgabe Zeile 3 rechts
  if ( (Iwb != IwbHist) || DisplayNow == true ) {
    sprintf(sBuffer,"Iwb =%2d.%02dA",Iwb/100,abs(Iwb%100));
    lcd.setCursor(9, 2);lcd.print(sBuffer);
    IwbHist=Iwb;
  }
  // Display Ausgabe Zeile 4 rechts
  if ( (Isim != IsimHist) || DisplayNow == true ) {
    sprintf(sBuffer,"Isim=%2d.%02dA",Isim/100,abs(Isim%100));
    lcd.setCursor(9, 3);lcd.print(sBuffer);
    IsimHist=Isim;
  }
  DisplayNow = false;
}

// Display 1 - Ströme ----------------------------------------
if (DisplayNr == 1){
  // Display Ausgabe Zeile 2
  if ( (CurrentL1 != CurrentL1Hist) || DisplayNow == true ) {
    sprintf(sBuffer,"L1=%2d.%002dA",CurrentL1/100,abs(CurrentL1%100));
    lcd.setCursor(0, 1);lcd.print(sBuffer);
    sprintf(sBuffer,"P1=%4dW",PowerL1);
    lcd.setCursor(11, 1);lcd.print(sBuffer);
    CurrentL1Hist=CurrentL1;
  }
  // Display Ausgabe Zeile 3
  if ( (CurrentL2 != CurrentL2Hist) || DisplayNow == true ) {
    sprintf(sBuffer,"L2=%2d.%002dA",CurrentL2/100,abs(CurrentL2%100));
    lcd.setCursor(0, 2);lcd.print(sBuffer);
    sprintf(sBuffer,"P2=%4dW",PowerL2);
    lcd.setCursor(11, 2);lcd.print(sBuffer);
    CurrentL2Hist=CurrentL2;
  }
  // Display Ausgabe Zeile 4
  if ( (CurrentL3 != CurrentL3Hist) || DisplayNow == true ) {
    sprintf(sBuffer,"L3=%2d.%002dA",CurrentL3/100,abs(CurrentL3%100));
    lcd.setCursor(0, 3);lcd.print(sBuffer);
    sprintf(sBuffer,"P3=%4dW",PowerL3);
    lcd.setCursor(11, 3);lcd.print(sBuffer);
    CurrentL3Hist=CurrentL3;
  }
  DisplayNow = false;
}
// Display 2 - kWh-Zähler ----------------------------------------
if (DisplayNr == 2){
  // Display Ausgabe Zeile 2
  if ( (kWhCounterStart != kWhCounterStartHist) || DisplayNow == true ) {
    lcd.setCursor(0, 1);
    lcd.print("StartCnt=");
    lcd.print(kWhCounterStart/100);
    lcd.print(".");
    lcd.print(abs(kWhCounterStart%100));
    lcd.print("kWh");
  }
  // Display Ausgabe Zeile 3
  if ( (kWhCounter != kWhCounterHist) || DisplayNow == true ) {
    lcd.setCursor(0, 2);
    lcd.print("Counter =");
    lcd.print(kWhCounter/100);
    lcd.print(".");
    lcd.print(abs(kWhCounter%100));
    lcd.print("kWh");
    // Display Ausgabe Zeile 4
    lcd.setCursor(0, 3);
    lcd.print("Loaded  =");
    lcd.print(kWhLoaded/100);
    lcd.print(".");
    lcd.print(abs(kWhLoaded%100));
    lcd.print("kWh ");
  }
  DisplayNow = false;
}
// Display 3 - Systemdaten ----------------------------------------
if (DisplayNr == 3){
  // Display Ausgabe Zeile 2/3
  if ( DisplayNow == true ) {
    lcd.setCursor(0, 1);
    lcd.print("CycleMin=");
    lcd.print(CycleTimeMin);
    lcd.print("ms ");
    lcd.setCursor(0, 2);
    lcd.print("CycleMax=");
    lcd.print(CycleTimeMax);
    lcd.print("ms ");
  }
  // Display Ausgabe Zeile 4
  if ( (PWMSignal != PWMSignalHist) || DisplayNow == true ) {
    lcd.setCursor(0, 3);
    lcd.print("PWM     =");
    lcd.print(PWMSignal);
    lcd.print("  ");
    PWMSignalHist = PWMSignal;

    lcd.setCursor(14, 3);
    lcd.print("V");
    lcd.print(Version1);
    lcd.print(".");
    lcd.print(Version2);
    lcd.print(".");
    lcd.print(Version3);
  }
  DisplayNow = false;
}
//---------------------------------------------------------------------------

// Taster abfragen - pos. Flanke mit Entprellung-----------------------------
BnLevelUp = digitalRead(BN_LEVEL_UP_PIN);
if ( (BnLevelUpHist == 0) & (BnLevelUp==1) & (BnLevelUpTime < millis()) ){
  BnLevelUpTime = millis() + DEBOUNCETIME;
  ChangeLevel(1);
}
BnLevelUpHist = BnLevelUp;

BnLevelDn = digitalRead(BN_LEVEL_DN_PIN);
if ( (BnLevelDnHist == 0) & (BnLevelDn==1) & (BnLevelDnTime < millis()) ){
  BnLevelDnTime = millis() + DEBOUNCETIME;
  ChangeLevel(-1);
}
BnLevelDnHist = BnLevelDn;

BnDisplay = digitalRead(BN_DISPLAY_PIN);
if ( (BnDisplayHist == 0) & (BnDisplay==1) & (BnDisplayTime < millis()) ){
  BnDisplayTime = millis() + DEBOUNCETIME;
  DisplayNr +=1;
  if (DisplayNr > 3) DisplayNr=0;
  lcd.setCursor(0,1);lcd.print("                    ");
  lcd.setCursor(0,2);lcd.print("                    ");
  lcd.setCursor(0,3);lcd.print("                    ");
  DisplayNow = true;
}
BnDisplayHist = BnDisplay;

// RS485 - MB Daten abhören ----------------------
memset(readData,0,sizeof(readData)); // Speicher leeren
if (RS485Serial.available()) {
  // Request=8 Bytes, Response=17 Bytes, in 25 Bytes ist beides drin
  // Hier wartet er, bis 25 Bytes im Buffer sind, durch den Timeout bricht er nach 150ms ab
  RS485Serial.readBytes(readData,25); // Daten lesen, Buffer leeren
  DataReceived = true;
}

// Daten auswerten: Strom L1,L2,L3 - wird immer all 500ms abgefragt
// Auf den Request folgt der Response, deshalb die Request Daten prüfen
if (DataReceived == true){
  if ( readData[0] ==  1 && 
       readData[1] ==  3 && 
       readData[2] == 91 &&
       readData[3] == 12 && 
       readData[4] ==  0 && 
       readData[5] ==  6 &&
       readData[6] == 22 &&
       readData[7] ==239 ){
        // Request erkannt, dann kommen 3 Header-Bytes
        if ( readData[8] == 1 &&
             readData[9] == 3 &&
             readData[10]== 12 ){
              // Daten zuordnen
              CurrentL1 = readData[11]*256*256*256 + readData[12]*256*256 + readData[13]*256 + readData[14];
              CurrentL2 = readData[15]*256*256*256 + readData[16]*256*256 + readData[17]*256 + readData[18];
              CurrentL3 = readData[19]*256*256*256 + readData[20]*256*256 + readData[21]*256 + readData[22];
              //Checksumme ist in readData[23] und readData[24]
              //Serial.print("I1=");          
              //Serial.println(CurrentL1);
              PowerL1 = CurrentL1 / 100 * 230;           
              PowerL2 = CurrentL2 / 100 * 230;           
              PowerL3 = CurrentL3 / 100 * 230;
              // Wenn der Strom größer als 3A ist, dann lädt das Auto
              if (CurrentL1 > 300){
                 CarIsLoading = true;
              }
              // Wenn der Strom kleiner als 1A ist, dann lädt das Auto nicht mehr (Erhaltungsladung sind 0,5A)
              if (CurrentL1 < 100){
                 CarIsLoading = false;
                 LoadStartPuls = false;         
              }
        }
        DataReceived = false;
  }
}

// Daten auswerten: kWh-Zähler - wird beim Ladevorgang alle 10s abgefragt
// Auf den Request folgt der Response, deshalb die Request Daten prüfen
if (DataReceived == true){
  if (readData[0] ==  1 && 
      readData[1] ==  3 && 
      readData[2] == 80 &&
      readData[3] ==  0 && 
      readData[4] ==  0 && 
      readData[5] ==  4 &&
      readData[6] == 85 &&
      readData[7] ==  9 ){
        // Request erkannt, dann kommen 3 Header-Bytes
        if ( readData[8] == 1 &&
             readData[9] == 3 &&
             readData[10]== 8 ){
              // Daten zuordnen - readData[11-14] passen nicht in long Variable
              kWhCounter = readData[15]*256*256*256 +
                           readData[16]*256*256 +
                           readData[17]*256 +
                           readData[18];
              //Checksumme ist in readData[19] und readData[20]
              unsigned long longData1 = readData[15];
              unsigned long longData2 = readData[16];
              unsigned long longData3 = readData[17];
              unsigned long longData4 = readData[18];
              kWhCounter = (longData1 << 24) + (longData2 << 16) + (longData3 << 8) + longData4; 
              //Serial.print("kWh=");
              //Serial.println(kWhCounter);
              if ( (LoadStartPuls == false) && (kWhCounter > 0) ){
                LoadStartPuls = true;
                kWhCounterStart = kWhCounter;          
              }
              kWhLoaded = kWhCounter - kWhCounterStart;
        }
        DataReceived = false;
  }
}
  
// Iwb berechnen ----------------------
Iwb = CurrentL1;

// Isim berechnen ----------------------
if (LoadLevel<12) Isim = Imax - Iset + Iwb;
else              Isim = 0;

// PWM-Isim Zuordnung
// Funktion auf Basis Eckdaten: 20A->200
//PWMSignal = Isim * 10 / 94;
//PWMSignal = Isim / 10 + 10;
PWMSignal = Isim / 10;

// PWM-Signal begrenzen 0-255
if (PWMSignal < 0){
  PWMSignal=0;
}
if (PWMSignal > 255){
  PWMSignal = 255;
}

// Pulsgeber für PWM Signal, alle 100ms togglet PWMPuls 1/-1
if ( millis() > PWMPulsTime ){
  PWMPulsTime = millis() + PWMPULSINTERVAL;
  PWMPuls = - PWMPuls; // Bei jedem Durchlauf wechselt die 1 das Vorzeichen
}

// PWM Signal ausgeben - gepulst ----------------------
if (PWMPuls == 1) analogWrite(PWM_OUT_PIN, PWMSignal);
else              analogWrite(PWM_OUT_PIN, 0);

//---------------------------------------------------------------------------
// Serieller Monitor: Eingabe von + oder - für Level hoch und runter, Zahl für Ladestrom 6 -> 6A
  while (Serial.available() > 0) {
    int inChar = Serial.read();
    if (inChar == '+') ChangeLevel(1);
    if (inChar == '-') ChangeLevel(-1);
    if (isDigit(inChar)) inString += (char)inChar;
    // Eingabe mit Enter abschließen
    if (inChar == '\n') {
      if (inString.length()>0){
        switch (inString.toInt()){
          case 0: Iset = 0; LoadLevel = 0; 
          break;
          case 6: Iset = 600; LoadLevel = 1;  
          break;
          case 7: Iset = 700; LoadLevel = 2;  
          break;
          case 8: Iset = 800; LoadLevel = 3;  
          break;
          case 9: Iset = 900;  LoadLevel = 4; 
          break;
          case 10: Iset = 1000; LoadLevel = 5;  
          break;
          case 11: Iset = 1100; LoadLevel = 6;  
          break;
          case 12: Iset = 1200; LoadLevel = 7;  
          break;
          case 136: Iset = 1300; LoadLevel = 8;  
          break;
          case 14: Iset = 1400; LoadLevel = 9;  
          break;
          case 15: Iset = 1500; LoadLevel = 10;  
          break;
          case 16: Iset = 1600; LoadLevel = 11;  
          break;
        }
      }
      Serial.print("Iset:");
      Serial.println(Iset);
      Serial.print("Load Level:");
      Serial.println(LoadLevel);
      // Am Ende den Speicher leeren
      inString = "";
    }
  }
//---------------------------------------------------------------------------  

// END MAIN LOOP
}
//===========================================================================
void ChangeLevel(int LevelStep)
{
  if ( (LoadLevel > 0)  & (LevelStep < 0) ) LoadLevel -=1;
  if ( (LoadLevel < 12) & (LevelStep > 0) ) LoadLevel +=1;
  if      (LoadLevel == 0)  Iset = 0;
  else if (LoadLevel == 12) Iset = 1600;
  else                      Iset = LoadLevel*100 + 500;
}
