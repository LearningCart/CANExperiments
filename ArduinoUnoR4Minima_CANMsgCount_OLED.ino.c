/*
  CAN Read Write test for Ardunio R4 Mininma

  Receive and read CAN Bus messages., 
  Send count back after a second., 

*/
/*
  Note:
  Since, Arduino R4 Minima has built in CAN, we have just connected 
  CAN Transceiver chip TJA1050 from SmartElex and some clone.
*/
/**************************************************************************************
 * INCLUDE
 **************************************************************************************/
#include <Wire.h>
#include <Arduino_CAN.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/**************************************************************************************
 * SETUP/LOOP
 **************************************************************************************/
/**
	CANTX: D4,
  CANRX: D5
*/




#define DISPLAY_WIDTH 128 // OLED display width, in pixels
#define DISPLAY_HEIGHT 64 // OLED display height, in pixels


unsigned long previousMillis = 0;
const unsigned long heartbeat_interval = 3000;
static uint32_t msg_cnt = 0;
static uint32_t const CAN_MSG_ID = 0x444;
static bool RespondForIncommingMsg = false;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1);

void display_init() {

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;)
    {
      Serial.print(".");
      Serial.flush();
      delay(1000);
    }
  }

  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("CAN Node 2");
  display.display();
  delay(3000);
  display.clearDisplay();
}
void show_count (uint8_t count)
{
  display.clearDisplay();
  display.setCursor(0, 8);
  display.print("CAN: ");
  display.println(count, DEC);
  display.display();
}
void setup() {
  int errcnt = 0;
  previousMillis = 0;

  Serial.begin(115200);
  // while (!Serial) {
  //   delay(100);
  // }
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println("Jatin Gandhi: Arduino R4 Minima OLED Message Node");

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
  display_init();
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
    } else if (msg.data_length < 1)
    {
      // Message is short., blink built in LED
      msgIndicate();
      return;  
    } else {
      show_count(msg.data[0]);
      Serial.print("Can Msg Count: ");
      Serial.println(msg.data[0],DEC);
      msgIndicate();
    }
  }

  // unsigned long currentMillis = millis();

  // if (currentMillis - previousMillis >= heartbeat_interval) {
  //   previousMillis = currentMillis;
  //   Serial.println("Sending CAN HB To Node: " + String(CAN_ID));
  //   sendCANHBMsg();
  // }
}
