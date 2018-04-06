#ifndef _LEDS_H
#define _LEDS_H
#include "application.h"

enum LED_STATE {eLED_OFF, eLED_RED, eLED_GREEN, eLED_BLUE, eLED_YELLOW};
enum LED_ID {eLED1=0, eLED2=2, eLED3=3, eLED4=4, eLED5=5, eLED6=6};


class LEDS
{
    private:
    DFX_NCV7240 *_pLS;
    enum LED_STATE eLed[6];

    public:
    void vSetup(uint8_t nCS); // The constructor

    void Set(enum LED_ID eLedID, enum LED_STATE eLedState);
    void AllOff();
    void AllOn();
};

#endif
