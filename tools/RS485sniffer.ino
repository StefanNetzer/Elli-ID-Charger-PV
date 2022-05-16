// adapted from http://jheyman.github.io/blog/pages/RS485Sniffer/
// requires a Mega to have more than 1 serial IO


#include <MsTimer2.h>

#define MESSAGE_GAP_TIMEOUT_IN_MS 5
#define LED_TOGGLE_MSGS 50

char RecvBuffer[256];

unsigned int recvIndex = 0;
unsigned long msgIndex = 0;
unsigned long receiveTime = 0;

char receiveTimeString[32];
char msgIndexString[32];

const byte LED_PIN = 13;

void setup()
{

  Serial.begin(115200);
  Serial.println ( "RS485 Sniffer");
  // RS485 Modbus zum ABB kWh-Zähler - Modbus RTU, 19200 baud, Parity: even, Stopbit:1
  Serial3.begin(19200, SERIAL_8E1);  //

  pinMode(LED_PIN, OUTPUT);

  // set an end-of-message detection timeout of 5ms
  MsTimer2::set(MESSAGE_GAP_TIMEOUT_IN_MS, onTimer);
}

bool dumpData = false;
bool timerStarted = false;

// If this timer expires, this means no additional character was received for a while: notify main loop
void onTimer() {
  dumpData = true;
}

void loop()
{
  char received;

  if (Serial3.available() > 0) {
    received = Serial3.read();
    RecvBuffer[recvIndex++] = received;
  }

  // first time we get an empty buffer after receiving stuff:
  // this could be the end of the message, (re)start the end-of-message detection timer.
  if (recvIndex > 0) {

    if (!timerStarted) {
      MsTimer2::start();
      timerStarted = true;
    }
  }

  // If the timer expired and positioned this var, we should now dump the received message
  if (dumpData) {
    receiveTime = micros() - MESSAGE_GAP_TIMEOUT_IN_MS * 1000;

    // reinitialize vars for next detection/dump
    dumpData = false;
    MsTimer2::stop();
    timerStarted = false;

    msgIndex++ ;
    if (msgIndex > 999) msgIndex = 0;

    // build and print message

    sprintf(receiveTimeString, "%015lu:", receiveTime);
    Serial.write(receiveTimeString, strlen(receiveTimeString));
    Serial.write(receiveTimeString, strlen(receiveTimeString));

    sprintf(msgIndexString, "%04lu:", msgIndex);
    Serial.write(msgIndexString, strlen(msgIndexString));

    Serial.write(RecvBuffer, recvIndex);
    Serial.println();

    // reset index for next message
    recvIndex = 0;
  }
}