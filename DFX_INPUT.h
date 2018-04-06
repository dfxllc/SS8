#ifndef _DFX_INPUT_H
#define _DFX_INPUT_H
#include "application.h"

enum INPUT_MODE {eIN_MODE_BATT, eIN_MODE_GND, eIN_MODE_ANALOG};

enum INPUT_ID {eIN1=0, eIN2=1, eIN3=2, eIN4=3, eIN5=4};


class DFX_INPUT
{
    private:
    uint8_t _inputpin;
    uint8_t _pullupdownpin;
    INPUT_MODE _mode;
    
    public:
    DFX_INPUT(uint8_t inputpin, uint8_t pullupdownpin); // The constructor
    DFX_INPUT(uint8_t inputpin, uint8_t pullupdownpin, INPUT_MODE mode);
    
    void SetMode(INPUT_MODE mode);
    int iRead();
};
#endif