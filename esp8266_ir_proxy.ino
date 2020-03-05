#include <Arduino.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

uint16_t RECV_PIN = 13; // ESP8266 D7
IRrecv irrecv(RECV_PIN);

const uint16_t kIrLed = 4;  // ESP8266 D2 (Recommended by IRsend)
IRsend irsend(kIrLed);  // Set the GPIO to be used to sending the message.

const uint16_t OK_LED = 16; // ESP8266 D0

decode_results results;

bool ok_led_on = true;

int off_timeout = -1;

void turnOkLedOff() {
  digitalWrite(OK_LED, LOW);
  off_timeout = 0;
  ok_led_on = false;
}

void turnOkLedOn() {
  digitalWrite(OK_LED, HIGH);
  ok_led_on = true;
}

void setup() {
  Serial.begin(115200);
  irrecv.enableIRIn();  // Start the receiver
  irsend.begin();

  pinMode(OK_LED, OUTPUT);
  turnOkLedOn();
  Serial.println();
  Serial.println();
}

void send_ir_code(uint16_t buf[], uint16_t len, char* msg) {
  delay(50);
  turnOkLedOff();
  Serial.println(msg);
  irsend.sendRaw(buf, len, 38);
}

// normally the raw data for NEC should only be 67
// but the microlab speakers missbehave when receiving IR code from another remove of a near samsung TV
// the fix is to include an extra 4 elements which actually represent the repeat code
uint16_t rawDataDownExtra[71] = {9228, 4438,  632, 1614,  632, 508,  606, 536,  608, 504,  632, 506,  628, 486,  636, 506,  630, 510,  630, 484,  634, 1642,  626, 1616,  632, 1616,  658, 1618,  630, 1616,  632, 1616,  658, 1616,  606, 506,  636, 534,  598, 512,  602, 538,  630, 1614,  604, 510,  634, 534,  574, 534,  630, 1618,  628, 1618,  660, 1614,  630, 1618,  604, 536,  626, 1620,  628, 1616,  638, 1636,  628, 39862,  9178, 2196,  632};  // NEC 807F08F7
uint16_t rawDataUpExtra[71] = {9116, 4474,  568, 1676,  628, 510,  568, 570,  568, 540,  632, 508,  570, 570,  566, 542,  630, 508,  592, 572,  564, 1654,  566, 1676,  628, 1642,  570, 1702,  542, 1676,  628, 1642,  568, 1676,  594, 1654,  626, 508,  568, 570,  588, 520,  628, 1642,  590, 520,  628, 508,  568, 570,  566, 546,  626, 1668,  564, 1656,  590, 1652,  628, 512,  572, 1670,  630, 1642,  568, 1678,  588, 39930,  9098, 2234,  590};  // NEC 807F8877
uint16_t rawDataMuteExtra[71] = {9200, 4436,  626, 1618,  630, 510,  626, 514,  630, 508,  598, 538,  602, 482,  634, 506,  626, 510,  632, 482,  632, 1638,  628, 1616,  630, 1616,  658, 1612,  630, 1618,  628, 1616,  632, 1638,  630, 484,  656, 1614,  604, 536,  630, 480,  628, 508,  628, 482,  634, 504,  628, 514,  628, 1614,  628, 510,  604, 1642,  604, 1640,  632, 1640,  626, 1616,  630, 1616,  636, 1636,  624, 39856,  9160, 2194,  634};  // NEC 807F40BF

void loop() {
  if (irrecv.decode(&results)) {
    Serial.print("Received code: ");
    serialPrintUint64(results.value, 16);
    Serial.println();

    switch (results.value) {
      case 0xE0E0D02F:
        send_ir_code(rawDataDownExtra, 71, "Volume Down");
        break;
      case 0xE0E0E01F:
        send_ir_code(rawDataUpExtra, 71, "Volume Up");
        break;
      case 0xE0E0F00F:
        send_ir_code(rawDataMuteExtra, 71, "Mute");
        break;
      default:
        Serial.println("Ignore");
        break;
    }
    
    Serial.println();
    
    irrecv.resume();  // Receive the next value
  } else {
    // cannot turn led on immediatly after ir send, because it causes a lot of errors on next ir read
    // not sure why ...
    if (!ok_led_on) {
      turnOkLedOn();
    }
  }
}
