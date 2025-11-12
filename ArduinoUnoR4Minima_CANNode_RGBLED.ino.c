/*
  CAN Read Write test for Ardunio R4 Mininma

  Receive and read CAN Bus messages., 
  Send count back after a second., 

*/

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <Arduino_CAN.h>
#include <FastLED.h>


/**************************************************************************************
 * SETUP/LOOP
 **************************************************************************************/
/**
	CANTX: D4,
  CANRX: D5
*/

#define LED_PIN     8
#define NUM_LEDS    1

CRGB leds[NUM_LEDS];


unsigned long previousMillis = 0;
const unsigned long heartbeat_interval = 3000;
static uint32_t msg_cnt = 0;
static uint32_t const CAN_MSG_ID = 0x333;
static bool RespondForIncommingMsg = false;


void setup_RGBLED() {

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

}

void setcolor_RGBLED(uint8_t data[3])
{
  // We have already checked the length., so not checking again., 
  leds[0] = CRGB ( data[0], data[1], data[2]);
  FastLED.show();
  delay(100);
}


void turnoff_RGBLED()
{
  // We have already checked the length., so not checking again., 
  leds[0] = CRGB ( 0, 0, 0);
  FastLED.show();
  delay(100);
}

void setup() {
  int errcnt = 0;
  previousMillis = 0;

  Serial.begin(115200);
  // while (!Serial) {
  //   delay(100);
  // }
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("Jatin Gandhi: Arduino R4 Minima RGB LED Node");

  if (!CAN.begin(CanBitRate::BR_500k)) {
    errcnt = 0;
    Serial.println("CAN.begin(...) failed.");
    for (;;) {
      errcnt++;
      if (errcnt >= 50) {
        Serial.println("CAN.begin(...) failed.");
        errcnt = 0;
      }

      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
  } else {
    Serial.println("CAN begin successful.,");
    Serial.println("Waiting for CAN msg., ");
  }
  setup_RGBLED();
  uint8_t data[3] = {50, 50, 50};
  setcolor_RGBLED(data); // Use default LED brightness., 
  delay(1500);
  turnoff_RGBLED();
}

void msgIndicate(void) {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(500);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(500);                       // wait for a second
}

void sendCANHBMsg(void) {
  /* Assemble a CAN message with the format of
   * 0xCA 0xFE 0x00 0x00 [4 byte message counter]
   */
  uint8_t const msg_data[] = { 'H', 'B', 0, 0, 0, 0, 0, 0 };
  CanMsg const msg(CanStandardId(CAN_MSG_ID), sizeof(msg_data), msg_data);

  /* Transmit the CAN message, capture and display an
   * error core in case of failure.
   */
  if (int const rc = CAN.write(msg); rc < 0) {
    Serial.print("CAN.write(...) failed with error code ");
    Serial.println(rc);
#ifdef STALL_ON_ERROR
    for (;;) {
      delay(100);
    }
#endif
  }
  msgIndicate();
}
void sendCANMsgCount(void) {
  /* Assemble a CAN message with the format of
   * 0xCA 0xFE 0x00 0x00 [4 byte message counter]
   */
  uint8_t const msg_data[] = { 0xCA, 0xFE, 0, 0, 0, 0, 0, 0 };
  memcpy((void *)(msg_data + 4), &msg_cnt, sizeof(msg_cnt));
  CanMsg const msg(CanStandardId(CAN_MSG_ID), sizeof(msg_data), msg_data);

  /* Transmit the CAN message, capture and display an
   * error core in case of failure.
   */
  if (unsigned int const rc = CAN.write(msg); rc < 0) {
    Serial.print("CAN.write(...) failed with error code ");
    Serial.print("0x");
    Serial.println(rc, HEX);
#ifdef STALL_ON_ERROR
    for (;;) {
      delay(100);
    }
#endif
  }

  /* Increase the message counter. */
  msg_cnt++;
}



void loop() {
  CanMsg msg;
  msg.id = 0x000;
  msg.data_length = 0;

  if (CAN.available()) {
    // Get message., 
    msg = CAN.read();
    
    if (CAN_MSG_ID != msg.id) {
      return;  // Messages is not for us.,
    } else if (msg.data_length < 3)
    {
      // Message is short., blink built in LED
      msgIndicate();
      return;  
    } else {
      Serial.print("Got RGB LED Message (R,G,B): (");
      Serial.print(msg.data[0], HEX);
      Serial.print(", ");
      Serial.print(msg.data[1], HEX);
      Serial.print(", ");
      Serial.print(msg.data[2], HEX);
      Serial.println(")");
      // We got RGB 
      setcolor_RGBLED(msg.data);
    }
  }

  // unsigned long currentMillis = millis();

  // if (currentMillis - previousMillis >= heartbeat_interval) {
  //   previousMillis = currentMillis;
  //   Serial.println("Sending CAN HB To Node: " + String(CAN_ID));
  //   sendCANHBMsg();
  // }
}
