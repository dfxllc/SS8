#include "ss8_bsp.h"

#define LED_TEST_DELAY 100

static void vTestCANMessageTX(void);
static void vTestLEDs(void);
int iModbusCloudFunctionHandler(String command);
int iModbusCloudFunctionHandler2(String command);
static void vSetModbusHBridgeDirection(const unsigned char u8Direction);
static void vSetModbusHBridgePercentage(const uint16_t u16Percentage);
int iProcessGenericModbusCommand(uint16_t u16SlaveAddress, uint16_t u16Command, uint16_t u16RegAddress, uint16_t u16Value);

char buf[100];

CANChannel can(CAN_D1_D2);

void setup() {
  IPAddress myIp;
  SS8_BSP.vSetupIO();
  Serial.println("Starting Library Test App.");

  /* Seting up peripherals */
  can.begin(250000);
  Serial.begin(9600); /* USB COM Port on Photon/Electron */
  SS8_BSP.ModBus.begin(9600); /* MODBUS RTU RS845 */

  /* Setup cloud function handlers */
  Particle.function("hbridge", iModbusCloudFunctionHandler);
  Particle.function("modbus", iModbusCloudFunctionHandler2);

  /* Print Local IP Address to Particle Console */
  myIp = WiFi.localIP();
  sprintf(buf, "\rLocal IP Address: %d.%d.%d.%d\n", myIp[0], myIp[1], myIp[2], myIp[3]);
  Spark.publish("Local IP Address", buf, 60, PRIVATE);

}

void loop() {
  delay(1000);
  vTestCANMessageTX();
  vTestLEDs();
}

static void vTestCANMessageTX(void)
{
  Serial.println("Testing CAN Message");

  CANMessage message;
  message.id = 0x0C000000;
  message.extended = true;
  message.len = 8;
  message.data[0] = 0x00;
  message.data[1] = 0x01;
  message.data[2] = 0x02;
  message.data[3] = 0x03;
  message.data[4] = 0x04;
  message.data[5] = 0x05;
  message.data[6] = 0x06;
  message.data[7] = 0x07;

  if (!can.transmit(message))
  {
      Serial.println("Message was not added to the queue.");
      delay(1000);
  }
  else
  {
      Serial.println("Message was added to the queue.");
  }

  if(can.errorStatus() == CAN_BUS_OFF) {
      Serial.println("Not properly connected to CAN bus");
  }
}

/* Test function for the LEDs. */
static void vTestLEDs(void)
{
  Serial.println("Testing LEDS");
  SS8_BSP.pLEDs.AllOn();
  delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.AllOff();
  delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.Set(eLED1, eLED_BLUE); delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.AllOff(); delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.Set(eLED2, eLED_RED); delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.AllOff(); delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.Set(eLED2, eLED_GREEN); delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.AllOff(); delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.Set(eLED2, eLED_YELLOW); delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.AllOff(); delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.Set(eLED3, eLED_GREEN); delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.AllOff(); delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.Set(eLED4, eLED_RED); delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.AllOff(); delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.Set(eLED5, eLED_GREEN); delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.AllOff(); delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.Set(eLED6, eLED_RED); delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.AllOff(); delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.Set(eLED6, eLED_GREEN); delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.AllOff(); delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.Set(eLED6, eLED_YELLOW); delay(LED_TEST_DELAY);
  SS8_BSP.pLEDs.AllOff(); delay(LED_TEST_DELAY);
}

int iModbusCloudFunctionHandler2(String command)
{
  char acData[20];
  command.toCharArray(acData, 20);
  char* pacArgument;
  int iCommand;
  int iSlaveAddress;
  int iValue;
  int iRegAddress;

  pacArgument = strtok(acData, ",");  /* Get 1st Item */

  if (pacArgument != NULL)
  {
    iSlaveAddress = atoi(pacArgument);

    pacArgument = strtok(NULL, ",");

    if (pacArgument != NULL)  /* Get 2nd Item */
    {
      iCommand = atoi(pacArgument);

      pacArgument = strtok(NULL, ",");

      if (pacArgument != NULL)  /* Get 3rd Item */
      {
        iRegAddress = atoi(pacArgument);

        pacArgument = strtok(NULL, ",");

        if (pacArgument != NULL)  /* Get 4th Item */
        {
          iValue = atoi(pacArgument);
          return iProcessGenericModbusCommand((uint16_t)iSlaveAddress, (uint16_t)iCommand, (uint16_t)iRegAddress, (uint16_t)iValue);
        }
      }
    }
  }

  return -1;
}

int iProcessGenericModbusCommand(uint16_t u16SlaveAddress, uint16_t u16Command, uint16_t u16RegAddress, uint16_t u16Value)
{
  int iReturnValue;
  switch (u16Command)
  {
    case 0:
      iReturnValue = (SS8_BSP.ModBus.writeSingleCoil(u16SlaveAddress, u16RegAddress, u16Value)) ? 1 : 0;
      break;
    case 1:
      iReturnValue = (SS8_BSP.ModBus.writeSingleRegister(u16SlaveAddress, u16RegAddress, u16Value)) ? 1 : 0;
      break;
      default:
        iReturnValue = -1;
        break;
  }

  return iReturnValue;
}

int iModbusCloudFunctionHandler(String command)
{
  char acData[10];
  command.toCharArray(acData, 10);
  char* pacArgument = strtok(acData, ",");

  if (pacArgument != NULL)
  {
    if (atoi(pacArgument) == 0)
    {
      vSetModbusHBridgeDirection(0);
    }
    else
    {
      vSetModbusHBridgeDirection(1);
    }
  }

  pacArgument = strtok(NULL, ",");

  if (pacArgument != NULL)
  {
    vSetModbusHBridgePercentage((uint16_t)atoi(pacArgument));
  }

  return 0;
}

static void vSetModbusHBridgeDirection(const unsigned char u8Direction)
{
  if (SS8_BSP.ModBus.writeSingleCoil(0x0A, 0, (u8Direction) ? 1 : 0))
  {
    Serial.println("Set coil");
  }
  else
  {
    Serial.println("Failed to set coil");
  }

  delay(500);

  if (SS8_BSP.ModBus.writeSingleCoil(0x0A, 1, (!u8Direction) ? 1 : 0))
  {
    Serial.println("Set coil");
  }
  else
  {
    Serial.println("Failed to set coil");
  }

  delay(500);
}

static void vSetModbusHBridgePercentage(const uint16_t u16Percentage)
{
  if (SS8_BSP.ModBus.writeSingleRegister(0x0A, 0, u16Percentage))
  {
    Serial.println("Wrote to holding register.");
  }
  else
  {
    Serial.println("Failed to write holding register.");
  }

  delay(500);
}
