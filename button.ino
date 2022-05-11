#ifdef UseButtons
void QueryButton () {
  // Taster abfragen - pos. Flanke mit Entprellung-----------------------------
  BnLevelUp = digitalRead(BN_LEVEL_UP_PIN);
  if ( (BnLevelUpHist == 0) & (BnLevelUp == 1) & (BnLevelUpTime < millis()) ) {
    BnLevelUpTime = millis() + DEBOUNCETIME;
    ChangeLevel(1);
  }
  BnLevelUpHist = BnLevelUp;

  BnLevelDn = digitalRead(BN_LEVEL_DN_PIN);
  if ( (BnLevelDnHist == 0) & (BnLevelDn == 1) & (BnLevelDnTime < millis()) ) {
    BnLevelDnTime = millis() + DEBOUNCETIME;
    ChangeLevel(-1);
  }
  BnLevelDnHist = BnLevelDn;

  BnDisplay = digitalRead(BN_DISPLAY_PIN);
  if ( (BnDisplayHist == 0) & (BnDisplay == 1) & (BnDisplayTime < millis()) ) {
    BnDisplayTime = millis() + DEBOUNCETIME;
    DisplayNr += 1;
    if (DisplayNr > 3) DisplayNr = 0;
    lcd.setCursor(0, 1); lcd.print("                    ");
    lcd.setCursor(0, 2); lcd.print("                    ");
    lcd.setCursor(0, 3); lcd.print("                    ");
    DisplayNow = true;
  }
  BnDisplayHist = BnDisplay;
}
#endif
