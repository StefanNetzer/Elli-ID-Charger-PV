#ifdef UseDisplay
void Display( int DisplayNr) {
  //  Display 0 ----------------------------------------
  if (DisplayNr == 0) {
    // Display Ausgabe Zeile 2 links
    if ( (BlinkPuls == true) || DisplayNow == true) {
      if ( (CarIsLoading == true) && (BlinkOn == 1) ) {
        lcd.setCursor(0, 1); lcd.print(F("loading.."));
      } else {
        lcd.setCursor(0, 1); lcd.print(F("         "));
      }
    }

    // Display Ausgabe Zeile 3 links
    if ( (LoadLevel != LoadLevelHist) || DisplayNow == true ) {
      sprintf(sBuffer, "Level=%2d", LoadLevel);
      lcd.setCursor(0, 2); lcd.print(sBuffer);
      LoadLevelHist = LoadLevel;
    }

    // Display Ausgabe Zeile 2 rechts
    if ( (Iset != IsetHist) || DisplayNow == true ) {
      sprintf(sBuffer, "Iset=%2d.%02dA", Iset / 100, abs(Iset % 100));
      lcd.setCursor(9, 1); lcd.print(sBuffer);
      IsetHist = Iset;
    }
    // Display Ausgabe Zeile 3 rechts
    if ( (Iwb != IwbHist) || DisplayNow == true ) {
      sprintf(sBuffer, "Iwb =%2d.%02dA", Iwb / 100, abs(Iwb % 100));
      lcd.setCursor(9, 2); lcd.print(sBuffer);
      IwbHist = Iwb;
    }
    // Display Ausgabe Zeile 4 rechts
    if ( (Isim != IsimHist) || DisplayNow == true ) {
      sprintf(sBuffer, "Isim=%2d.%02dA", Isim / 100, abs(Isim % 100));
      lcd.setCursor(9, 3); lcd.print(sBuffer);
      IsimHist = Isim;
    }
    DisplayNow = false;
  }

  // Display 1 - Ströme ----------------------------------------
  if (DisplayNr == 1) {
    // Display Ausgabe Zeile 2
    if ( (CurrentL1 != CurrentL1Hist) || DisplayNow == true ) {
      sprintf(sBuffer, "L1=%2d.%002dA", CurrentL1 / 100, abs(CurrentL1 % 100));
      lcd.setCursor(0, 1); lcd.print(sBuffer);
      sprintf(sBuffer, "P1=%4dW", PowerL1);
      lcd.setCursor(11, 1); lcd.print(sBuffer);
      CurrentL1Hist = CurrentL1;
    }
    // Display Ausgabe Zeile 3
    if ( (CurrentL2 != CurrentL2Hist) || DisplayNow == true ) {
      sprintf(sBuffer, "L2=%2d.%002dA", CurrentL2 / 100, abs(CurrentL2 % 100));
      lcd.setCursor(0, 2); lcd.print(sBuffer);
      sprintf(sBuffer, "P2=%4dW", PowerL2);
      lcd.setCursor(11, 2); lcd.print(sBuffer);
      CurrentL2Hist = CurrentL2;
    }
    // Display Ausgabe Zeile 4
    if ( (CurrentL3 != CurrentL3Hist) || DisplayNow == true ) {
      sprintf(sBuffer, "L3=%2d.%002dA", CurrentL3 / 100, abs(CurrentL3 % 100));
      lcd.setCursor(0, 3); lcd.print(sBuffer);
      sprintf(sBuffer, "P3=%4dW", PowerL3);
      lcd.setCursor(11, 3); lcd.print(sBuffer);
      CurrentL3Hist = CurrentL3;
    }
    DisplayNow = false;
  }
  // Display 2 - kWh-Zähler ----------------------------------------
  if (DisplayNr == 2) {
    // Display Ausgabe Zeile 2
    if ( (kWhCounterStart != kWhCounterStartHist) || DisplayNow == true ) {
      lcd.setCursor(0, 1);
      lcd.print("StartCnt=");
      lcd.print(kWhCounterStart / 100);
      lcd.print(".");
      lcd.print(abs(kWhCounterStart % 100));
      lcd.print("kWh");
    }
    // Display Ausgabe Zeile 3
    if ( (kWhCounter != kWhCounterHist) || DisplayNow == true ) {
      lcd.setCursor(0, 2);
      lcd.print("Counter =");
      lcd.print(kWhCounter / 100);
      lcd.print(".");
      lcd.print(abs(kWhCounter % 100));
      lcd.print("kWh");
      // Display Ausgabe Zeile 4
      lcd.setCursor(0, 3);
      lcd.print("Loaded  =");
      lcd.print(kWhLoaded / 100);
      lcd.print(".");
      lcd.print(abs(kWhLoaded % 100));
      lcd.print("kWh ");
    }
    DisplayNow = false;
  }
  // Display 3 - Systemdaten ----------------------------------------
  if (DisplayNr == 3) {
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

}
#endif
