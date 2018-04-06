#ifndef _LSOUT_H
#define _LSOUT_H

#include "DFX_NCV7240.h"

enum LSOUT_STATE {eLS_OFF, eLS_ON, eLS_INPUT};
enum LSOUT_ID {eLSOUT1=0,eLSOUT2=1,eLSOUT3=2,eLSOUT4=3,eLSOUT5=4,eLSOUT6=5,eLSOUT7=6,eLSOUT8=7};


class LSOUT
{
    private:
    DFX_NCV7240 *_pLS;
    
    public:
    LSOUT(uint8_t nCS); // The constructor
    
    void TurnOn(LSOUT_ID id);
        
    void TurnOff(LSOUT_ID id);
    
    void SetAll(LSOUT_STATE state);
};
#endif