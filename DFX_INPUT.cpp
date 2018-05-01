#include "application.h"
#include "DFX_INPUT.h"



void DFX_INPUT::SetMode(INPUT_MODE mode)
{
    _mode = mode; /* Make note of mode so we can use it in iRead */
    switch (mode)
    {
        case eIN_MODE_BATT: // Enable pull down - digital input
            pinMode(_inputpin, INPUT);
            pinMode(_pullupdownpin, OUTPUT);
            digitalWrite(_pullupdownpin, LOW);
            break;

        case eIN_MODE_GND: // Enable pull up - digital input
            pinMode(_inputpin, INPUT);
            pinMode(_pullupdownpin, OUTPUT);
            digitalWrite(_pullupdownpin, HIGH);
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
    pfValueChangedCallBack = NULL;
}

void DFX_INPUT::vSetupCallbackOnChange(void (*pFunction)(int))
{
  pfValueChangedCallBack = pFunction;
}

int DFX_INPUT::iRead()
{
    int iReadValue = 0;

    switch (_mode)
    {
        case eIN_MODE_BATT:
          iReadValue = (digitalRead(_inputpin) == TRUE) ? TRUE : FALSE;
          break;

        case eIN_MODE_GND:
            iReadValue = (digitalRead(_inputpin) == FALSE) ? TRUE : FALSE;
            break;

        case eIN_MODE_ANALOG:
            iReadValue = analogRead(_inputpin);
            break;
    }

    if ((pfValueChangedCallBack != NULL) && (iReadValue != iPreviousValue))
    {
      pfValueChangedCallBack(iReadValue);
    }

    iPreviousValue = iReadValue;
    return iReadValue;
}
