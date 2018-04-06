#include "application.h"
#include "DFX_INPUT.h"



void DFX_INPUT::SetMode(INPUT_MODE mode)
{
    _mode = mode;
    switch (_mode)
    {
        case eIN_MODE_BATT: // Enable pull down - digital input
            pinMode(_inputpin, INPUT);
            pinMode(_pullupdownpin, OUTPUT);
            digitalWrite(_pullupdownpin, LOW);
            break;
        
        case eIN_MODE_GND: // Enable pull up - digital input
            pinMode(_inputpin, INPUT);
            pinMode(_pullupdownpin, OUTPUT);
            digitalWrite(_pullupdownpin, LOW);
            break;

        case eIN_MODE_ANALOG: // Disable pull up/down - analog input
            // pin will automatically be set to be analog
            analogRead(_inputpin); // Ignore result
            pinMode(_pullupdownpin, INPUT);
            break;        
    }
}

DFX_INPUT::DFX_INPUT(uint8_t inputpin, uint8_t pullupdownpin)
{
    _inputpin = inputpin;
    _pullupdownpin = pullupdownpin;
}

DFX_INPUT::DFX_INPUT(uint8_t inputpin, uint8_t pullupdownpin, INPUT_MODE mode)
{
    _inputpin = inputpin;
    _pullupdownpin = pullupdownpin;
    SetMode(mode);
}


int DFX_INPUT::iRead()
{
    int iReadValue;
    switch (_mode)
    {
        case eIN_MODE_BATT:
        case eIN_MODE_GND:
            iReadValue = digitalRead(_inputpin) == HIGH ? 0xffff : 0;  
            break;
            
        case eIN_MODE_ANALOG:
            iReadValue = analogRead(_inputpin);
            break;
    }
    
    return iReadValue;
}