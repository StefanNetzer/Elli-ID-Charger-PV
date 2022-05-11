void CalcIsim () {
  // Iwb berechnen ----------------------
  Amperage[2]  = Current[0];

  // Isim berechnen ----------------------
  if (LoadLevel < 12) Amperage[0]  = Amperage[3]  - Amperage[1]  + Amperage[2] ; // Isim = Imax - Iset + Iwb;
  else              Amperage[0]  = 0;

  // PWM-Isim Zuordnung
  // Funktion auf Basis Eckdaten: 20A->200
  // Funktion auf Basis Eckdaten: 16A->160
  //PWMSignal = Amperage[0]  * 10 / 94;
  //PWMSignal = Amperage[0]  / 10 + 10;
  PWMSignal = Amperage[0]  / 10; // stimmt das noch ???

  // PWM-Signal begrenzen 0-255
  if (PWMSignal < 0) {
    PWMSignal = 0;
  }
  if (PWMSignal > 255) {
    PWMSignal = 255;
  }

  // Pulsgeber für PWM Signal, alle 100ms togglet PWMPuls 1/-1
  if ( millis() > PWMPulsTime ) {
    PWMPulsTime = millis() + PWMPULSINTERVAL;
    PWMPuls = - PWMPuls; // Bei jedem Durchlauf wechselt die 1 das Vorzeichen
  }

  // PWM Signal ausgeben - gepulst ----------------------
  if (PWMPuls == 1) analogWrite(PWM_OUT_PIN, PWMSignal);
  else              analogWrite(PWM_OUT_PIN, 0);
}

void ChangeLevel(int LevelStep)
{
  if ( (LoadLevel > 0)  & (LevelStep < 0) ) LoadLevel -= 1;
  if ( (LoadLevel < 12) & (LevelStep > 0) ) LoadLevel += 1;
  if      (LoadLevel == 0)  Amperage[1]  = 0; //Iset 
  else if (LoadLevel == 12) Amperage[1]  = 1600;  // muss das auf 1200 reduziert werden
  else                      Amperage[1]  = LoadLevel * 100 + LevelOffSet;  // @500 = 600...1600
}

void CycleTimeCheck ( void )
{
  // Zykluszeit messen ----------------------------------------
  CycleTime = millis() - MillisHist;
  MillisHist = millis();
  if (CycleTime > CycleTimeMax) {
    CycleTimeMax = CycleTime;
  }
  if ( (CycleTime < CycleTimeMin) && (CycleTime > 0) ) {
    CycleTimeMin = CycleTime;
  }
  if (millis() > CycleTimeMReset) {
    CycleTimeMReset = millis() + CYCLETIMERESETINTERVAL;
    CycleTimeMax = 0;
    CycleTimeMin = 1000;
  }
}
