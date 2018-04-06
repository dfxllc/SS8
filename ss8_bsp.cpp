#include "ss8_bsp.h"
#include "DFX_NCV7240.h"
#include "LSOUT.h"
#include "LEDS.h"

SS8_BSP_ SS8_BSP;

void SS8_BSP_::vSetupIO(void)
{
  digitalWrite(iRS485_nRE, HIGH); pinMode(iRS485_nRE, OUTPUT);
  digitalWrite(iRS485_DE, HIGH); pinMode(iRS485_DE, OUTPUT);

  digitalWrite(inCS_FRAM,HIGH);
  pinMode(inCS_FRAM, OUTPUT);

  //SPI bus:
  pinMode(iMOSI, OUTPUT);
  pinMode(iSCLK, OUTPUT);
  pinMode(iMISO, INPUT);

  pinMode(iPUPD1, INPUT); // so at startup the pull up/downs are disabled
  pinMode(iPUPD2, INPUT); // so at startup the pull up/downs are disabled
  pinMode(iPUPD3, INPUT); // so at startup the pull up/downs are disabled

  pLEDs.vSetup(D0); /* Setup LEDs */
}
