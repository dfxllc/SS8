#include "application.h"

#ifndef _DFX_NCV7240_H
#define _DFX_NCV7240_H

#define NO_OF_OUTPUTS 8

enum DFX_NCV7240_PIN_STATE {NCV7240_STANDBY,
                  NCV7240_INPUT,
                  NCV7240_ON,
                  NCV7240_OFF};
                  

class DFX_NCV7240
{
    public:
    DFX_NCV7240(uint8_t u8nCS);
    DFX_NCV7240(uint8_t u8nCS, enum DFX_NCV7240_PIN_STATE eDefaultOut);
    
    uint16_t TurnOn(uint8_t u8Channel);
    
    uint16_t TurnOff(uint8_t u8Channel);
    
    uint16_t SetAll(enum DFX_NCV7240_PIN_STATE state);
    
    uint16_t Set(  enum DFX_NCV7240_PIN_STATE eLS0,
                            enum DFX_NCV7240_PIN_STATE eLS1,
                            enum DFX_NCV7240_PIN_STATE eLS2,
                            enum DFX_NCV7240_PIN_STATE eLS3,
                            enum DFX_NCV7240_PIN_STATE eLS4,
                            enum DFX_NCV7240_PIN_STATE eLS5,
                            enum DFX_NCV7240_PIN_STATE eLS6,
                            enum DFX_NCV7240_PIN_STATE eLS7
                          );
    
    private:
    uint8_t _u8nCS;
    enum DFX_NCV7240_PIN_STATE eLS[NO_OF_OUTPUTS];
};
#endif
