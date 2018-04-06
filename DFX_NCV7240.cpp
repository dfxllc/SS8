#include "application.h"
#include "DFX_NCV7240.h"

uint16_t u16Translate(enum DFX_NCV7240_PIN_STATE eVal)
{ 
    return ((eVal == NCV7240_STANDBY) ? 0 :
            (eVal == NCV7240_INPUT) ? 1 :
            (eVal == NCV7240_ON) ? 2 :
            (eVal == NCV7240_OFF) ? 3 : 0);
};
    
uint16_t NCV7240_ControlWord(enum DFX_NCV7240_PIN_STATE eOut[])
{
    return  (u16Translate(eOut[7]) << 14) | 
            (u16Translate(eOut[6]) << 12) |
            (u16Translate(eOut[5]) << 10) |
            (u16Translate(eOut[4]) << 8)  |
            (u16Translate(eOut[3]) << 6)  |
            (u16Translate(eOut[2]) << 4)  |
            (u16Translate(eOut[1]) << 2)  |
            (u16Translate(eOut[0]) << 0);            
};

uint16_t NCV7240_u16Update(uint8_t nCS, enum DFX_NCV7240_PIN_STATE eOut[])
{
    uint8_t txbuffer[2], rxbuffer[2];
    uint16_t u16Temp = NCV7240_ControlWord(eOut);
    txbuffer[0] = u16Temp >> 8;
    txbuffer[1] = u16Temp & 0xff;
    rxbuffer[0]=0;
    rxbuffer[1]=0;
    
    
    SPI.beginTransaction(__SPISettings(10000, MSBFIRST, SPI_MODE1));
    
    SPI.begin(SPI_MODE_MASTER, nCS);
    digitalWrite(nCS, LOW);
    SPI.transfer(txbuffer, rxbuffer, sizeof(rxbuffer), NULL);
    digitalWrite(nCS, HIGH);
    SPI.end();
    
    SPI.endTransaction();

    
    return uint16_t(rxbuffer[0])<<8 + uint16_t(rxbuffer[1]);
}



DFX_NCV7240::DFX_NCV7240(uint8_t u8nCS)
{
   _u8nCS = u8nCS;
   pinMode(_u8nCS, OUTPUT);
   
   for (uint8_t i=0; i<NO_OF_OUTPUTS; i++)
   {
       eLS[i] = NCV7240_STANDBY;
   }
   
   NCV7240_u16Update(_u8nCS, eLS);
};
    
DFX_NCV7240::DFX_NCV7240(uint8_t u8nCS, enum DFX_NCV7240_PIN_STATE eDefaultOut)
{
   _u8nCS = u8nCS;
   pinMode(_u8nCS, OUTPUT);
      
   for (uint8_t i=0; i<NO_OF_OUTPUTS; i++)
   {
       eLS[i] = eDefaultOut;
   }
   
   NCV7240_u16Update(_u8nCS, eLS);
};

uint16_t DFX_NCV7240::TurnOn(uint8_t u8Channel)
{
    if (u8Channel < NO_OF_OUTPUTS)
    {
       eLS[u8Channel] = NCV7240_ON;
    }
    
   return NCV7240_u16Update(_u8nCS, eLS);
};

uint16_t DFX_NCV7240::TurnOff(uint8_t u8Channel)
{
    if (u8Channel < NO_OF_OUTPUTS)
    {
       eLS[u8Channel] = NCV7240_STANDBY;
    }
    
   return NCV7240_u16Update(_u8nCS, eLS);
};

uint16_t DFX_NCV7240::SetAll(enum DFX_NCV7240_PIN_STATE state)
{
    for (uint8_t i=0; i<NO_OF_OUTPUTS; i++)
    {
        eLS[i] = state;
    }
    
    return NCV7240_u16Update(_u8nCS, eLS);
};


uint16_t DFX_NCV7240::Set(  enum DFX_NCV7240_PIN_STATE eLS0,
                        enum DFX_NCV7240_PIN_STATE eLS1,
                        enum DFX_NCV7240_PIN_STATE eLS2,
                        enum DFX_NCV7240_PIN_STATE eLS3,
                        enum DFX_NCV7240_PIN_STATE eLS4,
                        enum DFX_NCV7240_PIN_STATE eLS5,
                        enum DFX_NCV7240_PIN_STATE eLS6,
                        enum DFX_NCV7240_PIN_STATE eLS7
                      )
{
    eLS[0] = eLS0;
    eLS[1] = eLS1;
    eLS[2] = eLS2;
    eLS[3] = eLS3;
    eLS[4] = eLS4;
    eLS[5] = eLS5;
    eLS[6] = eLS6;
    eLS[7] = eLS7;
    return NCV7240_u16Update(_u8nCS, eLS);
};

    

    





