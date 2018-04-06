#include "application.h"
#include "DFX_NCV7240.h"
#include "LEDS.h"

void vUpdate(DFX_NCV7240 *fn, enum LED_STATE eLed[])
{
  fn->Set(  ((eLed[eLED6] == eLED_RED)   || (eLed[eLED6] == eLED_YELLOW)) ? NCV7240_ON : NCV7240_STANDBY,
            ((eLed[eLED6] == eLED_GREEN) || (eLed[eLED6] == eLED_YELLOW)) ? NCV7240_ON : NCV7240_STANDBY,
            (eLed[eLED5] == eLED_GREEN)                                   ? NCV7240_ON : NCV7240_STANDBY,
            (eLed[eLED4] == eLED_RED)                                     ? NCV7240_ON : NCV7240_STANDBY,
            (eLed[eLED3] == eLED_GREEN)                                   ? NCV7240_ON : NCV7240_STANDBY,
            ((eLed[eLED2] == eLED_RED)   || (eLed[eLED2] == eLED_YELLOW)) ? NCV7240_ON : NCV7240_STANDBY,
            ((eLed[eLED2] == eLED_GREEN) || (eLed[eLED2] == eLED_YELLOW)) ? NCV7240_ON : NCV7240_STANDBY,
            (eLed[eLED1] == eLED_BLUE)                                    ? NCV7240_ON : NCV7240_STANDBY);

}



void LEDS::vSetup(uint8_t nCS)
{
    _pLS = new DFX_NCV7240(nCS);
    AllOff();
}


void LEDS::Set(enum LED_ID eLedID, enum LED_STATE eLedState)
{
    if (eLedID<=eLED6)
    {
        eLed[eLedID] = eLedState;
    }
    vUpdate(_pLS,eLed);
}

void LEDS::AllOff()
{
    for (uint8_t i=0; i<=eLED6; i++)
    {
        eLed[i]=eLED_OFF;
    }
    vUpdate(_pLS,eLed);
}

void  LEDS::AllOn()
{

   eLed[eLED1] = eLED_BLUE;
   eLed[eLED2] = eLED_YELLOW;
   eLed[eLED3] = eLED_GREEN;
   eLed[eLED4] = eLED_RED;
   eLed[eLED5] = eLED_GREEN;
   eLed[eLED6] = eLED_YELLOW;
   vUpdate(_pLS,eLed);
}
