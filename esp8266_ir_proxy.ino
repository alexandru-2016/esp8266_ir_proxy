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
}

void send_ir_code(uint64_t code, char* msg) {
  delay(100);
  turnOkLedOff();
  Serial.println(msg);
  irsend.sendNEC(code, 32);
}

void loop() {  
  if (irrecv.decode(&results)) {
    serialPrintUint64(results.value, 16);
    Serial.println();

    switch (results.value) {
      case 0xE0E0D02F:
        send_ir_code(0x807F08F7, "Volume Down");
        break;
      case 0xE0E0E01F:
        send_ir_code(0x807F8877, "Volume Up");
        break;
      case 0xE0E0F00F:
        send_ir_code(0x807F40BF, "Mute");
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
