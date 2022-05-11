#ifdef UseSerialInput
void SerialInput () {
  //---------------------------------------------------------------------------
  // Serieller Monitor: Eingabe von + oder - für Level hoch und runter, Zahl für Ladestrom 6 -> 6A
  while (Serial.available() > 0) {
    int inChar = Serial.read();
    if (inChar == '+') ChangeLevel(1);
    if (inChar == '-') ChangeLevel(-1);
    if (isDigit(inChar)) inString += (char)inChar;
    // Eingabe mit Enter abschließen
    if (inChar == '\n') {
      if (inString.length() > 0) {

      }
      Serial.print("Iset:");
      Serial.println(Amperage[1]);
      Serial.print("Load Level:");
      Serial.println(LoadLevel);
      // Am Ende den Speicher leeren
      inString = "";
    }
  }
}
#endif
