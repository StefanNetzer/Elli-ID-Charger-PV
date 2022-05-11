// Charger Load Control

//----------------------------
//History
// 0.1.000 18.03.2022 Erste Version Test Isim PWM Zuordnung
// 0.2.000 20.03.2022 RS485 Implementierung
// 0.3.000 27.03.2022 Erste funktionsfähige Version
// 0.4.000 27.03.2022 kWh Zähler einlesen, neues Display Management
// 0.5.000 02.04.2022 Neue Funktion für PWM-to-mA, neue Eingangsbelegung wegen PWM Frequenz 970Hz auf Pin 6 statt 470Hz auf Pin3
// 0.6.000 03.04.2022 kWh-Zähler erweitert um Verbrauchszähler
// 1.0.000 03.04.2022 Erfolgreicher Test
// 1.0.002 06.05.2022 SNE Modularisierung Code, Compileoptionen und von LAN und MQTT
// 1.0.003 07.05.2022 SNE Vereinfachung der Ausgabe
// 1.0.004 10.05.2020 SNE Vereinfachung serial Input, rudimentäres RS485 Error Handling

#include "ChargerLoadControl.h" // Definitionen ausgelagert in Header Datei

// SETUP
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // LCD
#ifdef UseDisplay
  lcd.init(); //Im Setup wird der LCD gestartet
  lcd.backlight(); //Hintergrundbeleuchtung einschalten
#endif
  // Serielle RS485
  pinMode(SSerialTxControl, OUTPUT);
  digitalWrite(SSerialTxControl, RS485Receive);  // Init Transceiver
  RS485Serial.begin(19200);   // set the data rate, Start the software serial port
  RS485Serial.setTimeout(100);

  //--- Eingaenge
#ifdef UseButtons
  pinMode(BN_LEVEL_UP_PIN, INPUT);
  digitalWrite(BN_LEVEL_UP_PIN, HIGH); // Pullup-Widerstand einschalten
  pinMode(BN_LEVEL_DN_PIN, INPUT);
  digitalWrite(BN_LEVEL_DN_PIN, HIGH); // Pullup-Widerstand einschalten
  pinMode(BN_DISPLAY_PIN, INPUT);
  digitalWrite(BN_DISPLAY_PIN, HIGH); // Pullup-Widerstand einschalten
#endif
  //--- Ausgaenge
  pinMode(PWM_OUT_PIN, OUTPUT);

  Serial.println(F("Charger Load Control starting"));
#ifdef UseMQTT
  // You can use Ethernet.init(pin) to configure the CS pin
  Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  Serial.println("Ethernet WebServer Example");

  // start the Ethernet connection and the server:
  Ethernet.begin(mac);

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  Serial.println(Ethernet.localIP());
  //---
  client.setServer(server, 1883);
  client.setCallback(callback);
#endif

  // Display Ausgabe Zeile 1 Überschrift
#ifdef UseDisplay
  lcd.setCursor(0, 0); lcd.print(F("CHARGER LOAD CONTROL"));
#endif
}


void loop() {
#ifdef UseMQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
#endif
  BlinkSignal();
#ifdef UseDisplay
  Display ( DisplayNr );
#endif
  CycleTimeCheck ();
#ifdef UseButtons
  QueryButton();
#endif
#ifdef UseSerialInput
  SerialInput();
#endif
  QueryRS485();
  CalcIsim ();
}
