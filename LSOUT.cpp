#include "application.h"
#include "LSOUT.h"


void LSOUT::vSetup(uint8_t nCS) // The constructor
{
   _pLS = new DFX_NCV7240(nCS);
};

void LSOUT::TurnOn(LSOUT_ID id)
{
   _pLS->TurnOn(uint8_t(id));
};

void LSOUT::TurnOff(LSOUT_ID id)
{
   _pLS->TurnOff(uint8_t(id));
};

void LSOUT::SetAll(LSOUT_STATE state)
{
    switch (state)
    {
        case eLS_OFF:  _pLS->SetAll(NCV7240_STANDBY); break;
        case eLS_ON:   _pLS->SetAll(NCV7240_ON); break;
        case eLS_INPUT:_pLS->SetAll(NCV7240_INPUT); break;
    }
}
