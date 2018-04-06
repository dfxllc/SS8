#ifndef H_DD_ADC_H
#define H_DD_ADC_H

#include "DFX_NCV7240.h"
#include "LSOUT.h"
#include "DFX_MODBUS.h"

#define iRS485_nRE D7
#define iRS485_DE D6
#define iPUPD1 D5
#define iPUPD2 D4
#define iPUPD3 D3
#define iAD2 A0
#define inCS_FRAM A2
#define iSCLK A3
#define iMISO A4
#define iMOSI A5
#define iAD1 DAC
#define iAD0 WKP
#define iRS485_RX RX
#define iRS485_TX TX

class SS8_BSP_
{
	public:
  void vSetupIO(void);
	LSOUT lsout;
	ModbusMaster ModBus;
	private:


};

extern SS8_BSP_ SS8_BSP;

#endif
