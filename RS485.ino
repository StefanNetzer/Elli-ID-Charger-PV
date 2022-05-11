void QueryRS485 () {
  // RS485 - MB Daten abhören ----------------------
  memset(readData, 0, sizeof(readData)); // Speicher leeren
  if (RS485Serial.available()) {
    // Request=8 Bytes, Response=17 Bytes, in 25 Bytes ist beides drin
    // Hier wartet er, bis 25 Bytes im Buffer sind, durch den Timeout bricht er nach 150ms ab
    RS485Serial.readBytes(readData, 25); // Daten lesen, Buffer leeren
    DataReceived = true;
    RS485Available = true;
  }
  else {
    RS485Available = false;
  }

  // Daten auswerten: Strom L1,L2,L3 - wird immer all 500ms abgefragt
  // Auf den Request folgt der Response, deshalb die Request Daten prüfen
  if (DataReceived == true) {
    if ( readData[0] ==  1 &&
         readData[1] ==  3 &&
         readData[2] == 91 &&
         readData[3] == 12 &&
         readData[4] ==  0 &&
         readData[5] ==  6 &&
         readData[6] == 22 &&
         readData[7] == 239 ) {
      // Request erkannt, dann kommen 3 Header-Bytes
      if ( readData[8] == 1 &&
           readData[9] == 3 &&
           readData[10] == 12 ) {
        // Daten zuordnen
        Current[0] = readData[11] * 256 * 256 * 256 + readData[12] * 256 * 256 + readData[13] * 256 + readData[14];
        Current[1] = readData[15] * 256 * 256 * 256 + readData[16] * 256 * 256 + readData[17] * 256 + readData[18];
        Current[2] = readData[19] * 256 * 256 * 256 + readData[20] * 256 * 256 + readData[21] * 256 + readData[22];
        //Checksumme ist in readData[23] und readData[24]
        //Serial.print("I1=");
        //Serial.println(CurrentL1);
        for (int i = 0; i <= 2; ++i ) Power[i] = Current[i] / 100 * 230;
        //PowerL1 = Current[0] / 100 * 230;
        //PowerL2 = Current[1] / 100 * 230;
        //PowerL3 = Current[2] / 100 * 230;
        // Wenn der Strom größer als 3A ist, dann lädt das Auto
        if (Current[0] > 300) {
          CarIsLoading = true;
        }
        // Wenn der Strom kleiner als 1A ist, dann lädt das Auto nicht mehr (Erhaltungsladung sind 0,5A)
        if (Current[0] < 100) {
          CarIsLoading = false;
          LoadStartPuls = false;
        }
      }
      DataReceived = false;
    }
  }

  // Daten auswerten: kWh-Zähler - wird beim Ladevorgang alle 10s abgefragt
  // Auf den Request folgt der Response, deshalb die Request Daten prüfen
  if (DataReceived == true) {
    if (readData[0] ==  1 &&
        readData[1] ==  3 &&
        readData[2] == 80 &&
        readData[3] ==  0 &&
        readData[4] ==  0 &&
        readData[5] ==  4 &&
        readData[6] == 85 &&
        readData[7] ==  9 ) {
      // Request erkannt, dann kommen 3 Header-Bytes
      if ( readData[8] == 1 &&
           readData[9] == 3 &&
           readData[10] == 8 ) {
        // Daten zuordnen - readData[11-14] passen nicht in long Variable
        kWh[0] = readData[15] * 256 * 256 * 256 +  // kWhCounter
                 readData[16] * 256 * 256 +
                 readData[17] * 256 +
                 readData[18];
        //Checksumme ist in readData[19] und readData[20]
        unsigned long longData1 = readData[15];
        unsigned long longData2 = readData[16];
        unsigned long longData3 = readData[17];
        unsigned long longData4 = readData[18];
        kWh[0] = (longData1 << 24) + (longData2 << 16) + (longData3 << 8) + longData4; //kWhCounter
        //Serial.print("kWh=");
        //Serial.println(kWhCounter);
        if ( (LoadStartPuls == false) && (kWh[0] > 0) ) {
          LoadStartPuls = true;
          kWh[1] = kWh[0]; //kWhCounterStart = kWhCounter
        }
        kWh[2] = kWh[0] - kWh[1]; //kWhLoaded = kWhCounter - kWhCounterStart
      }
      DataReceived = false;
    }
  }
}
