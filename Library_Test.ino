#include "ss8_bsp.h"

#define LED_TEST_DELAY 100

/* Function Prototypes */
static void vTestCANMessageTX(void);
static void vTestLEDs(void);
int iHBridgeModbusCloudFunctionHandler(String command);
int iGenericModbusCloudFunctionHandler(String command);
static void vSetModbusHBridgeDirection(const unsigned char u8Direction);
static void vSetModbusHBridgePercentage(const uint16_t u16Percentage);
int iProcessGenericModbusCommand(uint16_t u16SlaveAddress, uint16_t u16Command, uint16_t u16RegAddress, uint16_t u16Value);
void vLSOUTCloudHanlder(const char *event, const char *data);
void vPublishInputOnChanged(DFX_INPUT *inp, const char *h, const char *l);
void vInput1ChangeCallback(int iCurrentValue);
void vInput2ChangeCallback(int iCurrentValue);
void vInput3ChangeCallback(int iCurrentValue);

char buf[100];

CANChannel can(CAN_D1_D2);

/* Setup Digital/Analog Inputs */
DFX_INPUT ss8_input1(WKP, D5, eIN_MODE_BATT);
DFX_INPUT ss8_input2(DAC, D4, eIN_MODE_BATT);
DFX_INPUT ss8_input3(A0,  D3, eIN_MODE_BATT);

int iInput1 = 0;
int iInput2 = 0;
int iInput3 = 0;

void setup() {
  IPAddress myIp;
  SS8_BSP.vSetupIO();
  Serial.println("Starting Library Test App.");

  /* Seting up peripherals */
  can.begin(250000);
  Serial.begin(9600); /* USB COM Port on Photon/Electron */
  SS8_BSP.ModBus.begin(9600); /* MODBUS RTU RS845 */
  SS8_BSP.pLSOUT.SetAll(eLS_OFF);

  /* Setup cloud function handlers */
  Particle.function("hbridge", iHBridgeModbusCloudFunctionHandler);
  Particle.function("modbus", iGenericModbusCloudFunctionHandler);
  Particle.subscribe("lsout", vLSOUTCloudHanlder); // Subscribe to all things starting with the chars "lsout"

  /* Setup Input State Change Callback Functions */
  ss8_input1.vSetupCallbackOnChange(&vInput1ChangeCallback);
  ss8_input2.vSetupCallbackOnChange(&vInput2ChangeCallback);
  ss8_input3.vSetupCallbackOnChange(&vInput3ChangeCallback);


  /* Setup Cloud Variables */
  Particle.variable("Input1", iInput1);
  Particle.variable("Input2", iInput2);
  Particle.variable("Input3", iInput3);

  /* Print Local IP Address to Particle Console */
  myIp = WiFi.localIP();
  sprintf(buf, "\rLocal IP Address: %d.%d.%d.%d\n", myIp[0], myIp[1], myIp[2], myIp[3]);
  Spark.publish("Local IP Address", buf, 60, PRIVATE);

}

void loop() {
  delay(1000);
  vTestCANMessageTX();
  vTestLEDs();
  iInput1 = ss8_input1.iRead();
  iInput2 = ss8_input2.iRead();
//  iInput3 = ss8_input3.iRead();
  iInput3 = 0;
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

int iGenericModbusCloudFunctionHandler(String command)
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

int iHBridgeModbusCloudFunctionHandler(String command)
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

void vLSOUTCloudHanlder(const char *event, const char *data)
{
    LSOUT_ID id=eLSOUT1; // Not really needed - but the code below is fragil

    if (strcmp(event, "lsout1") == 0) id=eLSOUT1;
    if (strcmp(event, "lsout2") == 0) id=eLSOUT2;
    if (strcmp(event, "lsout3") == 0) id=eLSOUT3;
    if (strcmp(event, "lsout4") == 0) id=eLSOUT4;
    if (strcmp(event, "lsout5") == 0) id=eLSOUT5;
    if (strcmp(event, "lsout6") == 0) id=eLSOUT6;
    if (strcmp(event, "lsout7") == 0) id=eLSOUT7;
    if (strcmp(event, "lsout8") == 0) id=eLSOUT8;

    if (strncmp(data,"on",2) == 0) // Horrible code - but email appends \n\n\ to data
    {
       SS8_BSP.pLSOUT.TurnOn(id);
    }
    else
    {
       SS8_BSP.pLSOUT.TurnOff(id);
    }
}

void vInput1ChangeCallback(int iCurrentValue)
{
  Spark.publish("Input 1:", (iCurrentValue) ? "True" : "False", 60, PRIVATE);
}

void vInput2ChangeCallback(int iCurrentValue)
{
  Spark.publish("Input 2:", (iCurrentValue) ? "True" : "False", 60, PRIVATE);
}

void vInput3ChangeCallback(int iCurrentValue)
{
  Spark.publish("Input 3:", (iCurrentValue) ? "True" : "False", 60, PRIVATE);
}
