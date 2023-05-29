/* onewire.c - a part of avr-ds18b20 library
 *
 * Copyright (C) 2016 Jacek Wieczorek
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.	See the LICENSE file for details.
 */

/**
	\file
	\brief Implements 1wire protocol functions
*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "onewire.h"


//! Initializes 1wire bus before transmission
uint8_t onewireInit( sensor_t * sensor )
{
	// This will return false if no devices are present on the data bus
	uint8_t status = ONEWIRE_ERROR_COMM;
	gpio_init(sensor->pin);
	gpio_set_dir(sensor->pin, GPIO_OUT);
	gpio_put(sensor->pin, false); // bring low for 480us
	sleep_us(480);
	gpio_set_dir(sensor->pin, GPIO_IN); // let the data line float high
	sleep_us(70);					  // wait 70us
	if (!gpio_get(sensor->pin))
	{
		// see if any devices are pulling the data line low
		status = ONEWIRE_ERROR_OK;
	}
	sleep_us(410);
	return status;
}

//! Sends a single bit over the 1wire bus
uint8_t onewireWriteBit(sensor_t * sensor, uint8_t bit )
{
	gpio_set_dir(sensor->pin, GPIO_OUT);
	gpio_put(sensor->pin, false);
	sleep_us(3); // (spec 1-15us)
	if (bit)
	{
		gpio_put(sensor->pin, true);
		sleep_us(55);
	}
	else
	{
		sleep_us(60); // (spec 60-120us)
		gpio_put(sensor->pin, true);
		sleep_us(5); // allow bus to float high before next bit_out
	}
}

//! Transmits a byte over 1wire bus
void onewireWrite(sensor_t * sensor, uint8_t data )
{
	//uint8_t sreg = SREG; //Store status register
	uint8_t i = 0;

	#ifdef ONEWIRE_AUTO_CLI
		//cli( );
	#endif

	for ( i = 1; i != 0; i <<= 1 ) //Write byte in 8 single bit writes
		onewireWriteBit( sensor , data & i);

	//SREG = sreg;
}

//! Reads a bit from the 1wire bus
uint8_t onewireReadBit( sensor_t * sensor)
{
	uint8_t bit = 0;
	gpio_set_dir(sensor->pin, GPIO_OUT);
	gpio_put(sensor->pin, false);
	sleep_us(3); // (spec 1-15us)
	gpio_set_dir(sensor->pin, GPIO_IN);
	sleep_us(3); // (spec read within 15us)
	bit = gpio_get(sensor->pin);
	sleep_us(45);

	return bit;
}

//! Reads a byte from the 1wire bus
uint8_t onewireRead( sensor_t * sensor )
{
	//uint8_t sreg = SREG; //Store status register
	uint8_t data = 0;
	uint8_t i = 0;

	#ifdef ONEWIRE_AUTO_CLI
		//cli( );
	#endif

	for ( i = 1; i != 0; i <<= 1 ) //Read byte in 8 single bit reads
		data |= onewireReadBit( sensor ) * i;

	//SREG = sreg;

	return data;
}
