/**
@file
Arduino library for communicating with Modbus slaves over RS232/485 (via RTU protocol).
*/
/*
  ModbusMaster.cpp - Arduino library for communicating with Modbus slaves
  over RS232/485 (via RTU protocol).

  This file is part of ModbusMaster.

  ModbusMaster is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  ModbusMaster is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with ModbusMaster.  If not, see <http://www.gnu.org/licenses/>.

  Written by Doc Walker (Rx)
  Copyright Â© 2009-2013 Doc Walker <4-20ma at wvfans dot net>
  Adapted for Spark Core by Paul Kourany, March 14, 2014
  Further modifications and port to Particle Photon by Anurag Chugh, July 5, 2016 <lithiumhead at gmail dot com>
  Further modifications to use with DFX SS8 and allow specifying slave address in call by DJH 12/27/17.
*/


/* _____PROJECT INCLUDES_____________________________________________________ */
#include "application.h"
#include "DFX_MODBUS.h"

/* _____GLOBAL VARIABLES_____________________________________________________ */
USARTSerial MBSerial = Serial1;		 ///< Pointer to Serial1 class object
uint8_t MBTXEnablePin = D6;			 ///< GPIO used to control the RS485 receiver.  Set HIGH to enable transmitter
uint8_t MBnRXEnablePin = D7;     ///< GPIO used to control the RS485 receiver.  Set LOW to enable receiver
uint8_t MBUseEnablePin = 1;			 ///< Should a TX_ENABLE pin be used? 0 = No, 1 = Yes
uint8_t MBDebugSerialPrint = 0;		 ///< Do you want the TX and RX fraimes printed out on Serial for debugging? 0 = No, 1 = Yes



// Fix to define word type conversion function
uint16_t word(uint8_t high, uint8_t low)
{
	return (uint16_t(high) << 8) + uint16_t(low);
}



/* _____PROJECT INCLUDES_____________________________________________________ */
// functions to calculate Modbus Application Data Unit CRC
// fix for #include <util/crc16.h>
uint16_t _crc16_update(uint16_t crc, uint8_t a)
{
	crc ^= a;
	for (int i = 0; i < 8; ++i)
	{
    crc = (crc & 1) ? ((crc >> 1) ^ 0xA001) : (crc >> 1);
	}
	return crc;
}



/* _____PUBLIC FUNCTIONS_____________________________________________________ */
/**
Constructor.

Creates class object using default serial port 0, Modbus slave ID 1.

@ingroup setup
*/
ModbusMaster::ModbusMaster(void)
{
  _u8MBSlave = 1;
}

void ModbusMaster::withSlaveAddress(uint8_t u8SlaveAddress)
{
   _u8MBSlave = u8SlaveAddress;
}


/**
Initialize class object.

Sets up the serial port using default 9600 baud rate.
Call once class has been instantiated, typically within setup().

@ingroup setup
*/
void ModbusMaster::begin(void)
{
	begin(9600);
}


/**
Initialize class object.

Sets up the serial port using specified baud rate.
Call once class has been instantiated, typically within setup().

@overload ModbusMaster::begin(uint16_t u16BaudRate, uint8_t txrxpin)
@param u16BaudRate baud rate, in standard increments (300..115200)
@ingroup setup
*/
void ModbusMaster::begin(uint16_t u16BaudRate)
{
	_u8TransmitBufferIndex = 0;
	u16TransmitBufferLength = 0;
	MBSerial = Serial1;
	MBSerial.begin(u16BaudRate);
}


/**
Enable printing of debug messages.

When called, it enables the printing of debug messages
and the sent and received Modbus frames.
The messages are printed on "Serial" interface which needs to be preconfigured beforehand.

Beware, enabling this messes up the timings for RS485 Transactions, causing them to fail.

@ingroup setup
*/
void ModbusMaster::enableDebug()
{
	MBDebugSerialPrint = 1;
}


/**
Disables printing od debug messages.

Does opposite of what enableDebug() does

@ingroup setup
*/
void ModbusMaster::disableDebug()
{
	MBDebugSerialPrint = 0;
}

/**
Enable and set the GPIO pin that is required by some RS485 Drivers to switch from receiving to transmitting mode

@ingroup setup
*/
void ModbusMaster::enableTXpin(uint8_t pin)
{
	pinMode(pin, OUTPUT);
	MBTXEnablePin = pin;
	MBUseEnablePin = 1;
}


/**
Disables the use of a GPIO pin to trigger RS485 driver to TX mode

Does opposite of what enableTXpin() does

@ingroup setup
*/
void ModbusMaster::disableTXpin()
{
	MBUseEnablePin = 0;
}


void ModbusMaster::beginTransmission(uint16_t u16Address)
{
	_u16WriteAddress = u16Address;
	_u8TransmitBufferIndex = 0;
	u16TransmitBufferLength = 0;
}

// eliminate this function in favor of using existing MB request functions
uint8_t ModbusMaster::requestFrom(uint16_t address, uint16_t quantity)
{
	uint8_t read;
	// clamp to buffer length
	if(quantity > ku8MaxBufferSize)
  {
		quantity = ku8MaxBufferSize;
	}
	// set rx buffer iterator vars
	_u8ResponseBufferIndex = 0;
	_u8ResponseBufferLength = read;

	return read;
}


void ModbusMaster::sendBit(bool data)
{
	uint8_t txBitIndex = u16TransmitBufferLength % 16;
	if ((u16TransmitBufferLength >> 4) < ku8MaxBufferSize)
  {
		if (0 == txBitIndex)
    {
			_u16TransmitBuffer[_u8TransmitBufferIndex] = 0;
		}
		bitWrite(_u16TransmitBuffer[_u8TransmitBufferIndex], txBitIndex, data);
		u16TransmitBufferLength++;
		_u8TransmitBufferIndex = u16TransmitBufferLength >> 4;
	}
}


void ModbusMaster::send(uint16_t data)
{
	if (_u8TransmitBufferIndex < ku8MaxBufferSize)
  {
		_u16TransmitBuffer[_u8TransmitBufferIndex++] = data;
		u16TransmitBufferLength = _u8TransmitBufferIndex << 4;
	}
}


void ModbusMaster::send(uint32_t data)
{
	send(lowWord(data));
	send(highWord(data));
}


void ModbusMaster::send(uint8_t data)
{
	send(word(0x00, data));	  //MSB = 0, LSB = data
}

uint8_t ModbusMaster::available(void)
{
	return _u8ResponseBufferLength - _u8ResponseBufferIndex;
}


uint16_t ModbusMaster::receive(void)
{
	if (_u8ResponseBufferIndex < _u8ResponseBufferLength)
  {
		return _u16ResponseBuffer[_u8ResponseBufferIndex++];
	}
	else
  {
		return 0xFFFF;
	}
}

/**
Set idle time callback function (cooperative multitasking).

This function gets called in the idle time between transmission of data
and response from slave. Do not call functions that read from the serial
buffer that is used by ModbusMaster. Use of i2c/TWI, 1-Wire, other
serial ports, etc. is permitted within callback function.

@see ModbusMaster::ModbusMasterTransaction()
*/
void ModbusMaster::idle(void (*idle)())
{
	_idle = idle;
}


/**
Retrieve data from response buffer.

@see ModbusMaster::clearResponseBuffer()
@param u8Index index of response buffer array (0x00..0x3F)
@return value in position u8Index of response buffer (0x0000..0xFFFF)
@ingroup buffer
*/
uint16_t ModbusMaster::getResponseBuffer(uint8_t u8Index)
{
	if (u8Index < ku8MaxBufferSize)
  {
		return _u16ResponseBuffer[u8Index];
	}
  else
  {
		return 0xFFFF;
	}
}


/**
Clear Modbus response buffer.

@see ModbusMaster::getResponseBuffer(uint8_t u8Index)
@ingroup buffer
*/
void ModbusMaster::clearResponseBuffer()
{
  for (uint8_t i=0; i<ku8MaxBufferSize; i++)
  {
     _u16ResponseBuffer[i]=0;
  }
}


/**
Place data in transmit buffer.

@see ModbusMaster::clearTransmitBuffer()
@param u8Index index of transmit buffer array (0x00..0x3F)
@param u16Value value to place in position u8Index of transmit buffer (0x0000..0xFFFF)
@return 0 on success; exception number on failure
@ingroup buffer
*/
uint8_t ModbusMaster::setTransmitBuffer(uint8_t u8Index, uint16_t u16Value)
{
	if (u8Index < ku8MaxBufferSize)
  {
		_u16TransmitBuffer[u8Index] = u16Value;
		return ku8MBSuccess;
	}
  else
  {
		return ku8MBIllegalDataAddress;
	}
}


/**
Clear Modbus transmit buffer.

@see ModbusMaster::setTransmitBuffer(uint8_t u8Index, uint16_t u16Value)
@ingroup buffer
*/
void ModbusMaster::clearTransmitBuffer()
{
	for (uint8_t i = 0; i < ku8MaxBufferSize; i++)
  {
	  _u16TransmitBuffer[i] = 0;
	}
}


/**
Modbus function 0x01 Read Coils.

This function code is used to read from 1 to 2000 contiguous status of
coils in a remote device. The request specifies the starting address,
i.e. the address of the first coil specified, and the number of coils.
Coils are addressed starting at zero.

The coils in the response buffer are packed as one coil per bit of the
data field. Status is indicated as 1=ON and 0=OFF. The LSB of the first
data word contains the output addressed in the query. The other coils
follow toward the high order end of this word and from low order to high
order in subsequent words.

If the returned quantity is not a multiple of sixteen, the remaining
bits in the final data word will be padded with zeros (toward the high
order end of the word).

@param u16ReadAddress address of first coil (0x0000..0xFFFF)
@param u16BitQty quantity of coils to read (1..2000, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup discrete
*/
uint8_t ModbusMaster::readCoils(uint16_t u16ReadAddress, uint16_t u16BitQty)
{
	_u16ReadAddress = u16ReadAddress;
	_u16ReadQty = u16BitQty;
	return ModbusMasterTransaction(ku8MBReadCoils);
}

uint8_t ModbusMaster::readCoils(uint8_t u8SlaveAddress, uint16_t u16ReadAddress, uint16_t u16BitQty)
{
  _u8MBSlave = u8SlaveAddress;
  readCoils(u16ReadAddress, u16BitQty);
}


/**
Modbus function 0x02 Read Discrete Inputs.

This function code is used to read from 1 to 2000 contiguous status of
discrete inputs in a remote device. The request specifies the starting
address, i.e. the address of the first input specified, and the number
of inputs. Discrete inputs are addressed starting at zero.

The discrete inputs in the response buffer are packed as one input per
bit of the data field. Status is indicated as 1=ON; 0=OFF. The LSB of
the first data word contains the input addressed in the query. The other
inputs follow toward the high order end of this word, and from low order
to high order in subsequent words.

If the returned quantity is not a multiple of sixteen, the remaining
bits in the final data word will be padded with zeros (toward the high
order end of the word).

@param u16ReadAddress address of first discrete input (0x0000..0xFFFF)
@param u16BitQty quantity of discrete inputs to read (1..2000, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup discrete
*/
uint8_t ModbusMaster::readDiscreteInputs(uint16_t u16ReadAddress, uint16_t u16BitQty)
{
	_u16ReadAddress = u16ReadAddress;
	_u16ReadQty = u16BitQty;
	return ModbusMasterTransaction(ku8MBReadDiscreteInputs);
}

uint8_t ModbusMaster::readDiscreteInputs(uint8_t u8SlaveAddress, uint16_t u16ReadAddress, uint16_t u16BitQty)
{
    _u8MBSlave = u8SlaveAddress;
   readDiscreteInputs(u16ReadAddress, u16BitQty);
}


/**
Modbus function 0x03 Read Holding Registers.

This function code is used to read the contents of a contiguous block of
holding registers in a remote device. The request specifies the starting
register address and the number of registers. Registers are addressed
starting at zero.

The register data in the response buffer is packed as one word per
register.

@param u16ReadAddress address of the first holding register (0x0000..0xFFFF)
@param u16ReadQty quantity of holding registers to read (1..125, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup register
*/
uint8_t ModbusMaster::readHoldingRegisters(uint16_t u16ReadAddress,  uint16_t u16ReadQty)
{
	_u16ReadAddress = u16ReadAddress;
	_u16ReadQty = u16ReadQty;
	return ModbusMasterTransaction(ku8MBReadHoldingRegisters);
}

uint8_t ModbusMaster::readHoldingRegisters(uint8_t u8SlaveAddress, uint16_t u16ReadAddress,  uint16_t u16ReadQty)
{
    _u8MBSlave = u8SlaveAddress;
   readHoldingRegisters(u16ReadAddress, u16ReadQty);
}

/**
Modbus function 0x04 Read Input Registers.

This function code is used to read from 1 to 125 contiguous input
registers in a remote device. The request specifies the starting
register address and the number of registers. Registers are addressed
starting at zero.

The register data in the response buffer is packed as one word per
register.

@param u16ReadAddress address of the first input register (0x0000..0xFFFF)
@param u16ReadQty quantity of input registers to read (1..125, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup register
*/
uint8_t ModbusMaster::readInputRegisters(uint16_t u16ReadAddress,  uint8_t u16ReadQty)
{
	_u16ReadAddress = u16ReadAddress;
	_u16ReadQty = u16ReadQty;
	return ModbusMasterTransaction(ku8MBReadInputRegisters);
}

uint8_t  ModbusMaster::readInputRegisters(uint8_t u8SlaveAddress, uint16_t u16ReadAddress,  uint8_t u16ReadQty)
{
    _u8MBSlave = u8SlaveAddress;
   readInputRegisters(u16ReadAddress,  u16ReadQty);
}


/**
Modbus function 0x05 Write Single Coil.

This function code is used to write a single output to either ON or OFF
in a remote device. The requested ON/OFF state is specified by a
constant in the state field. A non-zero value requests the output to be
ON and a value of 0 requests it to be OFF. The request specifies the
address of the coil to be forced. Coils are addressed starting at zero.

@param u16WriteAddress address of the coil (0x0000..0xFFFF)
@param u8State 0=OFF, non-zero=ON (0x00..0xFF)
@return 0 on success; exception number on failure
@ingroup discrete
*/
uint8_t ModbusMaster::writeSingleCoil(uint16_t u16WriteAddress, uint8_t u8State)
{
	_u16WriteAddress = u16WriteAddress;
	_u16WriteQty = (u8State ? 0xFF00 : 0x0000);
	return ModbusMasterTransaction(ku8MBWriteSingleCoil);
}

uint8_t ModbusMaster::writeSingleCoil(uint8_t u8SlaveAddress, uint16_t u16WriteAddress, uint8_t u8State)
{
    _u8MBSlave = u8SlaveAddress;
   writeSingleCoil(u16WriteAddress,  u8State);
}


/**
Modbus function 0x06 Write Single Register.

This function code is used to write a single holding register in a
remote device. The request specifies the address of the register to be
written. Registers are addressed starting at zero.

@param u16WriteAddress address of the holding register (0x0000..0xFFFF)
@param u16WriteValue value to be written to holding register (0x0000..0xFFFF)
@return 0 on success; exception number on failure
@ingroup register
*/
uint8_t ModbusMaster::writeSingleRegister(uint16_t u16WriteAddress,  uint16_t u16WriteValue)
{
	_u16WriteAddress = u16WriteAddress;
	_u16WriteQty = 0;
	_u16TransmitBuffer[0] = u16WriteValue;
	return ModbusMasterTransaction(ku8MBWriteSingleRegister);
}

uint8_t  ModbusMaster::writeSingleRegister(uint8_t u8SlaveAddress, uint16_t u16WriteAddress,  uint16_t u16WriteValue)
{
    _u8MBSlave = u8SlaveAddress;
   writeSingleRegister(u16WriteAddress,  u16WriteValue);
}


/**
Modbus function 0x0F Write Multiple Coils.

This function code is used to force each coil in a sequence of coils to
either ON or OFF in a remote device. The request specifies the coil
references to be forced. Coils are addressed starting at zero.

The requested ON/OFF states are specified by contents of the transmit
buffer. A logical '1' in a bit position of the buffer requests the
corresponding output to be ON. A logical '0' requests it to be OFF.

@param u16WriteAddress address of the first coil (0x0000..0xFFFF)
@param u16BitQty quantity of coils to write (1..2000, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup discrete
*/
uint8_t ModbusMaster::writeMultipleCoils(uint16_t u16WriteAddress,  uint16_t u16BitQty)
{
	_u16WriteAddress = u16WriteAddress;
	_u16WriteQty = u16BitQty;
	return ModbusMasterTransaction(ku8MBWriteMultipleCoils);
}

uint8_t ModbusMaster::writeMultipleCoils(uint8_t u8SlaveAddress, uint16_t u16WriteAddress,  uint16_t u16BitQty)
{
  _u8MBSlave = u8SlaveAddress;
   writeMultipleCoils(u16WriteAddress,  u16BitQty);
}


uint8_t ModbusMaster::writeMultipleCoils()
{
	_u16WriteQty = u16TransmitBufferLength;
	return ModbusMasterTransaction(ku8MBWriteMultipleCoils);
}

uint8_t ModbusMaster::writeMultipleCoils(uint8_t u8SlaveAddress)
{
     _u8MBSlave = u8SlaveAddress;
     writeMultipleCoils();
}



/**
Modbus function 0x10 Write Multiple Registers.

This function code is used to write a block of contiguous registers (1
to 123 registers) in a remote device.

The requested written values are specified in the transmit buffer. Data
is packed as one word per register.

@param u16WriteAddress address of the holding register (0x0000..0xFFFF)
@param u16WriteQty quantity of holding registers to write (1..123, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup register
*/
uint8_t ModbusMaster::writeMultipleRegisters(uint16_t u16WriteAddress,  uint16_t u16WriteQty)
{
	_u16WriteAddress = u16WriteAddress;
	_u16WriteQty = u16WriteQty;
	return ModbusMasterTransaction(ku8MBWriteMultipleRegisters);
}

uint8_t  ModbusMaster::writeMultipleRegisters(uint8_t u8SlaveAddress, uint16_t u16WriteAddress,  uint16_t u16WriteQty)
{
   _u8MBSlave = u8SlaveAddress;
   writeMultipleRegisters(u16WriteAddress,  u16WriteQty);
}

// new version based on Wire.h
uint8_t ModbusMaster::writeMultipleRegisters()
{
	_u16WriteQty = _u8TransmitBufferIndex;
	return ModbusMasterTransaction(ku8MBWriteMultipleRegisters);
}

uint8_t ModbusMaster::writeMultipleRegisters(uint8_t u8SlaveAddress)
{
  _u8MBSlave = u8SlaveAddress;
	_u16WriteQty = _u8TransmitBufferIndex;
	return ModbusMasterTransaction(ku8MBWriteMultipleRegisters);
}


/**
Modbus function 0x16 Mask Write Register.

This function code is used to modify the contents of a specified holding
register using a combination of an AND mask, an OR mask, and the
register's current contents. The function can be used to set or clear
individual bits in the register.

The request specifies the holding register to be written, the data to be
used as the AND mask, and the data to be used as the OR mask. Registers
are addressed starting at zero.

The function's algorithm is:

Result = (Current Contents && And_Mask) || (Or_Mask && (~And_Mask))

@param u16WriteAddress address of the holding register (0x0000..0xFFFF)
@param u16AndMask AND mask (0x0000..0xFFFF)
@param u16OrMask OR mask (0x0000..0xFFFF)
@return 0 on success; exception number on failure
@ingroup register
*/
uint8_t ModbusMaster::maskWriteRegister(uint16_t u16WriteAddress,
                                        uint16_t u16AndMask,
                                        uint16_t u16OrMask)
{
	_u16WriteAddress = u16WriteAddress;
	_u16TransmitBuffer[0] = u16AndMask;
	_u16TransmitBuffer[1] = u16OrMask;
	return ModbusMasterTransaction(ku8MBMaskWriteRegister);
}

uint8_t  ModbusMaster::maskWriteRegister(uint8_t u8SlaveAddress, uint16_t u16WriteAddress, uint16_t u16AndMask, uint16_t u16OrMask)
{
  _u8MBSlave = u8SlaveAddress;
  maskWriteRegister(u16WriteAddress, u16AndMask, u16OrMask);
}

/**
Modbus function 0x17 Read Write Multiple Registers.

This function code performs a combination of one read operation and one
write operation in a single MODBUS transaction. The write operation is
performed before the read. Holding registers are addressed starting at
zero.

The request specifies the starting address and number of holding
registers to be read as well as the starting address, and the number of
holding registers. The data to be written is specified in the transmit
buffer.

@param u16ReadAddress address of the first holding register (0x0000..0xFFFF)
@param u16ReadQty quantity of holding registers to read (1..125, enforced by remote device)
@param u16WriteAddress address of the first holding register (0x0000..0xFFFF)
@param u16WriteQty quantity of holding registers to write (1..121, enforced by remote device)
@return 0 on success; exception number on failure
@ingroup register
*/
uint8_t ModbusMaster::readWriteMultipleRegisters(uint16_t u16ReadAddress,
                                                 uint16_t u16ReadQty,
                                                 uint16_t u16WriteAddress,
                                                 uint16_t u16WriteQty)
{
	_u16ReadAddress = u16ReadAddress;
	_u16ReadQty = u16ReadQty;
	_u16WriteAddress = u16WriteAddress;
	_u16WriteQty = u16WriteQty;
	return ModbusMasterTransaction(ku8MBReadWriteMultipleRegisters);
}

uint8_t ModbusMaster::readWriteMultipleRegisters(uint8_t u8SlaveAddress,
                                                 uint16_t u16ReadAddress,
                                                 uint16_t u16ReadQty,
                                                 uint16_t u16WriteAddress,
                                                 uint16_t u16WriteQty)
{
  _u8MBSlave = u8SlaveAddress;
  readWriteMultipleRegisters(u16ReadAddress, u16ReadQty, u16WriteAddress, u16WriteQty);
}

uint8_t ModbusMaster::readWriteMultipleRegisters(uint16_t u16ReadAddress,
                                                 uint16_t u16ReadQty)
{
	_u16ReadAddress = u16ReadAddress;
	_u16ReadQty = u16ReadQty;
	_u16WriteQty = _u8TransmitBufferIndex;
	return ModbusMasterTransaction(ku8MBReadWriteMultipleRegisters);
}

uint8_t ModbusMaster::readWriteMultipleRegisters(uint8_t u8SlaveAddress,
                                                 uint16_t u16ReadAddress,
                                                 uint16_t u16ReadQty)
{
    _u8MBSlave = u8SlaveAddress;
    readWriteMultipleRegisters(u16ReadAddress, u16ReadQty);
}

/* _____PRIVATE FUNCTIONS____________________________________________________ */

/**
Modbus transaction engine.
Sequence:
  - assemble Modbus Request Application Data Unit (ADU),
	based on particular function called
  - transmit request over selected serial port
  - wait for/retrieve response
  - evaluate/disassemble response
  - return status (success/exception)

@param u8MBFunction Modbus function (0x01..0xFF)
@return 0 on success; exception number on failure
*/
uint8_t ModbusMaster::ModbusMasterTransaction(uint8_t u8MBFunction)
{
	uint8_t u8ModbusADU[256];
	uint8_t u8ModbusADUSize = 0;
	uint8_t i, u8Qty;
	uint16_t u16CRC;
	uint32_t u32StartTime;
	uint8_t u8BytesLeft = 9;
	uint8_t u8MBStatus = ku8MBSuccess;

	// assemble Modbus Request Application Data Unit
	u8ModbusADU[u8ModbusADUSize++] = _u8MBSlave;
	u8ModbusADU[u8ModbusADUSize++] = u8MBFunction;

	switch(u8MBFunction) {
		case ku8MBReadCoils:
		case ku8MBReadDiscreteInputs:
		case ku8MBReadInputRegisters:
		case ku8MBReadHoldingRegisters:
		case ku8MBReadWriteMultipleRegisters:
			u8ModbusADU[u8ModbusADUSize++] = highByte(_u16ReadAddress);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16ReadAddress);
			u8ModbusADU[u8ModbusADUSize++] = highByte(_u16ReadQty);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16ReadQty);
			break;
	}

	switch(u8MBFunction) {
		case ku8MBWriteSingleCoil:
		case ku8MBMaskWriteRegister:
		case ku8MBWriteMultipleCoils:
		case ku8MBWriteSingleRegister:
		case ku8MBWriteMultipleRegisters:
		case ku8MBReadWriteMultipleRegisters:
			u8ModbusADU[u8ModbusADUSize++] = highByte(_u16WriteAddress);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16WriteAddress);
			break;
	}

	switch(u8MBFunction) {
		case ku8MBWriteSingleCoil:
			u8ModbusADU[u8ModbusADUSize++] = highByte(_u16WriteQty);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16WriteQty);
			break;

		case ku8MBWriteSingleRegister:
			u8ModbusADU[u8ModbusADUSize++] = highByte(_u16TransmitBuffer[0]);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16TransmitBuffer[0]);
			break;

		case ku8MBWriteMultipleCoils:
			u8ModbusADU[u8ModbusADUSize++] = highByte(_u16WriteQty);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16WriteQty);
			u8Qty = (_u16WriteQty % 8) ? ((_u16WriteQty >> 3) + 1) : (_u16WriteQty >> 3);
			u8ModbusADU[u8ModbusADUSize++] = u8Qty;
			for (i = 0; i < u8Qty; i++)
      {
				switch(i % 2)
        {
					case 0: // i is even
						u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16TransmitBuffer[i >> 1]);
						break;

					case 1: // i is odd
						u8ModbusADU[u8ModbusADUSize++] = highByte(_u16TransmitBuffer[i >> 1]);
						break;
				}
			}
			break;

		case ku8MBWriteMultipleRegisters:
		case ku8MBReadWriteMultipleRegisters:
			u8ModbusADU[u8ModbusADUSize++] = highByte(_u16WriteQty);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16WriteQty);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16WriteQty << 1);

			for (i = 0; i < lowByte(_u16WriteQty); i++)
      {
				u8ModbusADU[u8ModbusADUSize++] = highByte(_u16TransmitBuffer[i]);
				u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16TransmitBuffer[i]);
			}
			break;

		case ku8MBMaskWriteRegister:
			u8ModbusADU[u8ModbusADUSize++] = highByte(_u16TransmitBuffer[0]);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16TransmitBuffer[0]);
			u8ModbusADU[u8ModbusADUSize++] = highByte(_u16TransmitBuffer[1]);
			u8ModbusADU[u8ModbusADUSize++] = lowByte(_u16TransmitBuffer[1]);
			break;
	}

	// append CRC
	u16CRC = 0xFFFF;
	for (i = 0; i < u8ModbusADUSize; i++)
  {
		u16CRC = _crc16_update(u16CRC, u8ModbusADU[i]);
	}
	u8ModbusADU[u8ModbusADUSize++] = lowByte(u16CRC);
	u8ModbusADU[u8ModbusADUSize++] = highByte(u16CRC);
	u8ModbusADU[u8ModbusADUSize] = 0;

	if (MBSerial.available())
  { //Empty the receive buffer before beginning.
		MBSerial.read();
	}

	// transmit request
	if (MBUseEnablePin == 1)
  {  //Switch RS485 driver to transmitting mode.
		digitalWrite(MBTXEnablePin, HIGH);
	}

	if(MBDebugSerialPrint == 1)
  { // Print transmitted frame for Debugging purposes out on Serial
		Serial.print("TX: ");
	}
	for (i = 0; i < u8ModbusADUSize; i++)
  {
		MBSerial.write(u8ModbusADU[i]);
		if(MBDebugSerialPrint == 1)
    { // Print trasnmitted frame for Debugging purposes out on Serial
			Serial.print(u8ModbusADU[i], HEX);
			Serial.print(" ");
		}
	}
	if(MBDebugSerialPrint == 1)
  { // Print received frame for Debugging purposes out on Serial
		Serial.println(" ");
		Serial.print("RX: ");
	}

	u8ModbusADUSize = 0;

	MBSerial.flush(); //Wait for transmission to get completed

	if (MBUseEnablePin == 1)
  {  //Switch RS485 driver back to receiving mode.
		digitalWrite(MBTXEnablePin, LOW);
	}

	// loop until we run out of time or bytes, or an error occurs
	u32StartTime = millis();
	while (u8BytesLeft && !u8MBStatus)
  {
		if (MBSerial.available()) {
			u8ModbusADU[u8ModbusADUSize++] = MBSerial.read();
			if (u8ModbusADU[0] == 0)
      { //Incase the received character is zero, discard it
				u8ModbusADUSize--;
				continue;
			}
			if(MBDebugSerialPrint == 1)
      { // Print received frame for Debugging purposes out on Serial
				Serial.print(u8ModbusADU[u8ModbusADUSize-1], HEX);
				Serial.print(" ");
			}
			u8BytesLeft--;
		}
    else
    {
			if (_idle)
      {
				_idle();
			}
		}

		// evaluate slave ID, function code once enough bytes have been read
		if (u8ModbusADUSize == 5)
    {
			// verify response is for correct Modbus slave
			if (u8ModbusADU[0] != _u8MBSlave)
      {
				u8MBStatus = ku8MBInvalidSlaveID;
				break;
			}

			// verify response is for correct Modbus function code (mask exception bit 7)
			if ((u8ModbusADU[1] & 0x7F) != u8MBFunction)
      {
				u8MBStatus = ku8MBInvalidFunction;
				break;
			}

			// check whether Modbus exception occurred; return Modbus Exception Code
			if (bitRead(u8ModbusADU[1], 7))
      {
				u8MBStatus = u8ModbusADU[2];
				break;
			}

			// evaluate returned Modbus function code
			switch(u8ModbusADU[1])
      {
				case ku8MBReadCoils:
				case ku8MBReadDiscreteInputs:
				case ku8MBReadInputRegisters:
				case ku8MBReadHoldingRegisters:
				case ku8MBReadWriteMultipleRegisters:
					u8BytesLeft = u8ModbusADU[2];
					break;

				case ku8MBWriteSingleCoil:
				case ku8MBWriteMultipleCoils:
				case ku8MBWriteSingleRegister:
				case ku8MBWriteMultipleRegisters:
					u8BytesLeft = 3;
					break;

				case ku8MBMaskWriteRegister:
					u8BytesLeft = 5;
					break;
			}
		}
		if (millis() > (u32StartTime + ku8MBResponseTimeout))
    {
			u8MBStatus = ku8MBResponseTimedOut;
		}
	}

	if(MBDebugSerialPrint == 1)
  {  // Print received frame for Debugging purposes out on Serial
		Serial.println(" ");
	}
	// verify response is large enough to inspect further
	if (!u8MBStatus && u8ModbusADUSize >= 5)
  {
		// calculate CRC
		u16CRC = 0xFFFF;
		for (i = 0; i < (u8ModbusADUSize - 2); i++)
    {
			u16CRC = _crc16_update(u16CRC, u8ModbusADU[i]);
		}

		// verify CRC
		if (!u8MBStatus && (lowByte(u16CRC) != u8ModbusADU[u8ModbusADUSize - 2] ||
		     highByte(u16CRC) != u8ModbusADU[u8ModbusADUSize - 1]))
    {
			u8MBStatus = ku8MBInvalidCRC;
		}
	}

	// disassemble ADU into words
	if (!u8MBStatus)
  {
		// evaluate returned Modbus function code
		switch(u8ModbusADU[1])
    {
			case ku8MBReadCoils:
			case ku8MBReadDiscreteInputs:
				// load bytes into word; response bytes are ordered L, H, L, H, ...
				for (i = 0; i < (u8ModbusADU[2] >> 1); i++)
        {
					if (i < ku8MaxBufferSize)
          {
					_u16ResponseBuffer[i] = word(u8ModbusADU[2 * i + 4], u8ModbusADU[2 * i + 3]);
					}
					_u8ResponseBufferLength = i;
				}

				// in the event of an odd number of bytes, load last byte into zero-padded word
				if (u8ModbusADU[2] % 2)
        {
					if (i < ku8MaxBufferSize)
          {
					_u16ResponseBuffer[i] = word(0, u8ModbusADU[2 * i + 3]);
					}
					_u8ResponseBufferLength = i + 1;
				}
				break;

			case ku8MBReadInputRegisters:
			case ku8MBReadHoldingRegisters:
			case ku8MBReadWriteMultipleRegisters:
				// load bytes into word; response bytes are ordered H, L, H, L, ...
				for (i = 0; i < (u8ModbusADU[2] >> 1); i++)
        {
					if (i < ku8MaxBufferSize)
          {
						_u16ResponseBuffer[i] = word(u8ModbusADU[2 * i + 3], u8ModbusADU[2 * i + 4]);
					}
					_u8ResponseBufferLength = i;
				}
				break;
		}
	}

	_u8TransmitBufferIndex = 0;
	u16TransmitBufferLength = 0;
	_u8ResponseBufferIndex = 0;
	return u8MBStatus;
}
