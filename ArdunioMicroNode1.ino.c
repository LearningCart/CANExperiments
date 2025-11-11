#include <SPI.h>          //Library for using SPI Communication 
#include <mcp2515.h>      //Library for using CAN Communication (https://github.com/autowp/arduino-mcp2515/)


#define CAN_MSG_ID (0x111)
#define LED_PIN  (LED_BUILTIN)

unsigned long previousMillis = 0;
const unsigned long heartbeat_interval = 5000;

struct can_frame canMsg;


MCP2515 mcp2515(17);


void SendCANMessge(can_frame candata)
{
    mcp2515.sendMessage(&candata);     //Sends the CAN message
}


void msgIndicate(void)
{
  for(int i = 0; i < 5; i++)
  {
    digitalWrite(LED_PIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(400);                      // wait for a second
    digitalWrite(LED_PIN, LOW);   // turn the LED off by making the voltage LOW
    delay(400);                      // wait for a second
  }
}

void dumpCANFrame(can_frame &canMsg, String msg = "")
{
  Serial.print(msg + "CAN ID: ");
  Serial.print(canMsg.can_id, HEX);
  Serial.print(", CAN DATALEN: ");
  Serial.print(canMsg.can_dlc, HEX);
  Serial.print(", CAN DATA: ");
  for (int i = 0; i < canMsg.can_dlc; i++)
  {
    Serial.print("0x");
    Serial.print(canMsg.data[i], HEX);
    Serial.print(", ");
  }
  Serial.println("\n");
  
}

void SendHB(void)
{
  canMsg.can_id  = CAN_MSG_ID;           //CAN id as 0x036
  canMsg.can_dlc = 3;               //CAN data length as 8
  canMsg.data[0] = 'H';               //Update humidity value in [0]
  canMsg.data[1] = 'B';               //Update temperature value in [1]
  canMsg.data[1] = '1';               //Update temperature value in [1]
  // canMsg.data[2] = 0x00;            //Rest all with 0
  // canMsg.data[3] = 0x00;
  // canMsg.data[4] = 0x00;
  // canMsg.data[5] = 0x00;
  // canMsg.data[6] = 0x00;
  // canMsg.data[7] = 0x00;

  dumpCANFrame(canMsg, "HB 1");
  SendCANMessge(canMsg);
  msgIndicate();
  
}

uint8_t readReg(uint8_t addr) {
  uint8_t val;
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  digitalWrite(10, LOW);
  SPI.transfer(0x03);         // READ instruction
  SPI.transfer(addr);
  val = SPI.transfer(0x00);
  digitalWrite(10, HIGH);
  SPI.endTransaction();
  return val;
}

void dumpStatus() {
  uint8_t canstat = readReg(0x0E);  // CANSTAT
  uint8_t canctrl = readReg(0x0F);  // CANCTRL
  uint8_t eflg = readReg(0x2D);     // EFLG
  uint8_t tec  = readReg(0x1C);     // TEC
  uint8_t rec  = readReg(0x1D);     // REC
  Serial.print("CANSTAT=0x"); Serial.print(canstat, HEX);
  Serial.print(" CANCTRL=0x"); Serial.print(canctrl, HEX);
  Serial.print(" EFLG=0x"); Serial.print(eflg, HEX);
  Serial.print(" TEC="); Serial.print(tec);
  Serial.print(" REC="); Serial.println(rec);
}


void setup()
{
  previousMillis = 0;
  while (!Serial);
  Serial.begin(115200);
  Serial.println("Node 1 : Jatin Gandhi: CAN Read/Write Example");
  pinMode(LED_PIN, OUTPUT);
  SPI.begin();               //Begins SPI communication
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ); //Sets CAN at speed 500KBPS and Clock 8MHz
  mcp2515.setNormalMode();
  msgIndicate();
}


void loop()
{
  struct can_frame canMsg;
  unsigned long currentMillis = millis();

  // if (currentMillis - previousMillis >= heartbeat_interval)
  // {
  //   previousMillis = currentMillis;
  //   dumpStatus();
  //   SendHB();
  // }

  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) // To receive data (Poll Read)
  {
    // msgIndicate();
    // dumpCANFrame(canMsg, "Node 1: Received Msg ");
    // Serial.println(canMsg.can_id, HEX);
    if (canMsg.can_id != CAN_MSG_ID)
    {
      // Message is not for us., 
      return;
    }
    if ( 'O' == canMsg.data[0] && 'N' == canMsg.data[1] )
    {
      Serial.println("Turning LED ON");
      digitalWrite(LED_PIN, HIGH);  // turn the LED on (HIGH is the voltage level)

    }else if ('O' == canMsg.data[0] && 'F' == canMsg.data[1] && 'F' == canMsg.data[2])
    {
        Serial.println("Turning LED OFF");
        digitalWrite(LED_PIN, LOW);   // turn the LED off by making the voltage LOW
    }
  }
}
