void outputMQTT ( char outTopic[], byte outPayload[] ) {
#ifdef UseMQTT
  client.publish( outTopic, outPayload );
#endif
}

void output ( char outTopic[], char outPayload[] ) {
  Serial.print( outTopic );
  Serial.println (outPayload);
#ifdef UseMQTT
  outputMQTT ( outTopic, outPayload);
#endif
}

//Usage:
//char msgtopic[] = "PowerL1";
//char msgPayload[]= "";
//sprintf(msgPayload,"%02d", PowerL1);
//output ( msgtopic , msgPayload );


void BlinkSignal (void) {
  // Blinksignale ----------------------------------------
  BlinkPuls = false;
  if (millis() > BlinkTime) {
    BlinkTime = millis() + BLINKINTERVAL;
    BlinkPuls = true;
    BlinkOn = -BlinkOn;
    Serial.print("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    MQTTSend();
  }
}

void MQTTSend () {

  //Display 0
  if (RS485Available) output ( "RS485Available " , "true" );
  else output ( "RS485Available " , "false" );
  if (CarIsLoading) output ( "CarIsLoading " , "true" );
  else output ( "CarIsLoading " , "false" );
  sprintf(sBuffer, "%2d", LoadLevel);
  sprintf(sTopic, "LoadLevel ");
  output ( sTopic , sBuffer );
  for ( int i = 0; i <= 2; ++i ) {
    sprintf(sBuffer, "%2d.%02d A", Amperage[i] / 100, abs(Amperage[i] % 100));
    sprintf(sTopic, "%s", AmperageT[i] );
    output ( sTopic , sBuffer );
  }
  for (int i = 0; i <= 2; ++i) AmpHist[i] = Amperage[i];


  //Display 1
  for ( int i = 0; i <= 2; ++i ) {
    sprintf(sBuffer, "%2d.%002d A", Current[i] / 100, abs(Current[i] % 100));
    sprintf(sTopic, "%s", CurrentT[i] );
    output ( sTopic , sBuffer );
  }

  //Display 2
  // char *kWhT[] = {"kWhCounter", "kwHCounterStart", "kWhLoaded" };
  for ( int i = 0; i <= 2; ++i ) {
    sprintf(sBuffer, "%2d.%002d", kWh[i] / 100, abs(kWh[i] % 100));
    sprintf(sTopic, "%s", kWhT[i] );
    output ( sTopic , sBuffer );
  }

  // char *kWhHistT[] = {"kWhCounterHist", "kwHCounterStartHist" };
  for ( int i = 0; i <= 1; ++i ) {
    sprintf(sBuffer, "%2d.%002d", kWhHist[i] / 100, abs(kWhHist[i] % 100));
    sprintf(sTopic, "%s", kWhHistT[i] );
    output ( sTopic , sBuffer );
  }
  for (int i = 0; i <= 2; ++i) CurrentHist[i] = Current[i];

  //Display 3
  sprintf(sBuffer, "%2d", PWMSignal);
  sprintf(sTopic, "PWM ");
  output ( sTopic , sBuffer );
  sprintf(sBuffer, "%10d", CycleTimeMin);
  sprintf(sTopic, "CycleMin ");
  output ( sTopic , sBuffer );
  sprintf(sBuffer, "%10d", CycleTimeMax);
  sprintf(sTopic, "CycleMax ");
  output ( sTopic , sBuffer );


  PWMSignalHist = PWMSignal;
}
