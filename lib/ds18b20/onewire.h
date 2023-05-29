/* onewire.h - a part of avr-ds18b20 library
 *
 * Copyright (C) 2016 Jacek Wieczorek
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

/**
	\file
	\brief 1wire protocol functions
*/

#ifndef ONEWIRE_H
#define ONEWIRE_H


#include "ds18b20.h"

#define ONEWIRE_ERROR_OK 	0 //! Communication success
#define ONEWIRE_ERROR_COMM 	1 //! Communication failure

typedef struct sensor_t sensor_t;
struct sensor_t{
	uint8_t pin;
	uint16_t value;
};

/**
	\brief Initializes 1wire bus (basically sends a reset pulse)
	\param port A pointer to the port output register
	\param direction A pointer to the port direction register
	\param portin A pointer to the port input register
	\param mask A bit mask, determining to which pin the device is connected
	\returns \ref ONEWIRE_ERROR_OK on success
*/
uint8_t onewireInit( sensor_t * sensor );

/**
	\brief Sends a single bit over 1wire bus
	\param port A pointer to the port output register
	\param direction A pointer to the port direction register
	\param portin A pointer to the port input register
	\param mask A bit mask, determining to which pin the device is connected
	\param bit The bit value
	\returns the bit value
*/
uint8_t onewireWriteBit(sensor_t * sensor, uint8_t bit );

/**
	\brief Sends a byte over 1wire bus
	\param port A pointer to the port output register
	\param direction A pointer to the port direction register
	\param portin A pointer to the port input register
	\param mask A bit mask, determining to which pin the device is connected
	\param data Data byte to be sent
*/
void onewireWrite(sensor_t * sensor, uint8_t data );

/**
	\brief Reads a single bit from 1wire bus
	\param port A pointer to the port output register
	\param direction A pointer to the port direction register
	\param portin A pointer to the port input register
	\param mask A bit mask, determining to which pin the device is connected
	\returns received bit value
*/
uint8_t onewireReadBit( sensor_t * sensor);

/**
	\brief Reads a byte from 1wire bus
	\param port A pointer to the port output register
	\param direction A pointer to the port direction register
	\param portin A pointer to the port input register
	\param mask A bit mask, determining to which pin the device is connected
	\returns received byte value
*/
uint8_t onewireRead( sensor_t * sensor );

#endif
