int Version1 = 1; //Display 3
int Version2 = 0; //Display 3
int Version3 = 3; //Display 3

// prepare for different MCUs
#define UNO
#undef MEGA
#undef ESP32
// define Modules to be used
//#define UseDisplay
//#define UseButtons
#define UseSerialInput
//#define UseMQTT


// Display
//A5=SCL, A4=SDA

#ifdef UseDISPLAY
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4); // I2C Display auf Adresse 0x27 instanzieren
#endif

// RS485 Modbus zum ABB kWh-Zähler - Modbus RTU, 19200 baud, Parity: even, Stopbit:1
// D8=RxD-RO, D9=TxD-DI, D3=DE-RE
#include <SoftwareSerial.h>
#ifdef UNO
#define SSerialTxControl 3  //RS485 Direction control
#define SSerialRX        8  //Serial Receive pin
#define SSerialTX        9  //Serial Transmit pin
#endif

#define RS485Transmit    HIGH
#define RS485Receive     LOW
SoftwareSerial RS485Serial(SSerialRX, SSerialTX);
int byteRS485Received;
bool RS485Available;

#ifdef UseMQTT
// MQTT and LAN
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
//IPAddress ip(192, 168, 188, 30);
IPAddress server(192, 168, 178, 54);
//MQTT
EthernetClient ethClient;
PubSubClient client(ethClient);
#endif //UseMQ

//--- Eingaenge
#ifdef UseButtons
#define BN_LEVEL_DN_PIN 2    // Pin 2 als Eingang: Ladestufen Runtertasten
#define BN_LEVEL_UP_PIN 3    // Pin 3 als Eingang: Ladestufen Hochtasten
#define BN_DISPLAY_PIN  5    // Pin 5 als Eingang: Display Seiten wechseln

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

#endif

//--- Ausgaenge
#ifdef UNO
#define PWM_OUT_PIN 6 // Pin 6 für PWM Signal
#endif

// Init
bool Init = false;
int Cyclecount1 = 0;
int Cyclecount2 = 0;

// Komponenten von Isim
//int Isim = 0;    //Display 0 // Simulierter Strom in 0,01A - wird an den Wallbox CT geschickt
//int Iset = 0;    //Display 0 // Vorgabe Ladestrom in 0,01A - Stufenweise 0,6,7,8,..,15,16
//int Iwb = 0;     //Display 0 // Wallbox Strom in 0,01A
//int Imax = 2000; //Display 0 // Maximaler Hausstrom in 0,01A - über DIP Schalter in Wallbox einstellen: 20A
char *AmperageT[] { "Isim ", "Iset ", "Iwb ", "Imax " };
int Amperage[] { 0, 0, 0, 1600 };
                    
int LoadLevel = 0; //Display 0 // Ladestufen 0-12: 0=0A,1=6A,2=7A,3=8A...,10=15A,11=16A,12=0 mA auf CT
int LevelOffSet = 400;

// Historiemerker
//int IsimHist = 1;
//int IsetHist = 1;
//int IwbHist = 1;
//int LoadLevelHist = 1; 
char *AmpHistT[]= { "IsimHist ", "IsetHist ", "IwbHist ", "LoadLevelHist " };  
int AmpHist[] = { 1, 1, 1, 1 };

// PWM Signal
int PWMSignal = 0; // Display 3 // PWM Signal 0-255
int PWMSignalHist = 1;



// Eingabe am seriellen Monitor
#ifdef UseSerialInput
String inString = "";
#endif

// Display
char sBuffer[20];
char sTopic[20];
int DisplayNr = 0;
bool DisplayNow = false;
unsigned long BlinkTime;
#define BLINKINTERVAL 10000 // ms
bool BlinkPuls = false;
int BlinkOn = 1;

// Zykluszeit
unsigned long MillisHist;
unsigned int CycleTime;
unsigned int CycleTimeMax = 0;    //Display 3
unsigned int CycleTimeMin = 1000; //Display 3
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
//int CurrentL1 = 0; // Display 1 // Gemessener Phasenstrom L1 in 0.01 A
//int CurrentL2 = 0; // Display 1 // Gemessener Phasenstrom L2 in 0.01 A
//int CurrentL3 = 0; // Display 1 // Gemessener Phasenstrom L3 in 0.01 A
//int CurrentL1Hist, CurrentL2Hist, CurrentL3Hist = 1;
//int PowerL1=0, PowerL2=0, PowerL3=0;
bool CarIsLoading = false; //Display 0
//changing to arrays
char *CurrentT[] = { "L1","L2","L3"};
int Current[] = { 0,0,0 }; 
int CurrentHist[] = { 1, 1, 1 }; 
int Power[] = { 0,0,0 };
bool LoadStartPuls = false;

// kWh-Zähler aus Modbus Daten extrahieren:  Request 8 Byte + Response 17 Byte
//unsigned long kWhCounter = 0;      // Display 2 // Energiezähler in 0.01 kWh
//unsigned long kWhCounterStart = 0; // Display 2// Energiezähler beim Start der Ladung in 0.01 kWh
//unsigned long kWhLoaded = 0;       //Display 2
char *kWhT[] = {"kWhCounter ", "kwHCounterStart ", "kWhLoaded " };
unsigned long kWh[] = { 0, 0, 0 };
//unsigned long kWhCounterStartHist = 1;
//unsigned long kWhCounterHist = 1;
char *kWhHistT[] = {"kWhCounterHist", "kwHCounterStartHist" };
unsigned long kWhHist[] = { 0, 0 };

//-------------------------------------
//####################
