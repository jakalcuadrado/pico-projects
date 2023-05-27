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


#include "onewire.h"


//! Initializes 1wire bus before transmission
uint8_t onewireInit( sensor_t * sensor )
{
	uint8_t response = 0;
	//uint8_t sreg = SREG; //Store status register

	#ifdef ONEWIRE_AUTO_CLI
		//cli( );
	#endif
	gpio_set_pin_level(sensor->pin,1);
	gpio_set_pin_direction(sensor->pin, GPIO_DIRECTION_OUT);
	gpio_set_pin_level(sensor->pin,0);
	
	//*port |= mask; //Write 1 to output
	//*direction |= mask; //Set port to output
	//*port &= ~mask; //Write 0 to output

	delay_us( 600 );
	
	gpio_set_pin_direction(sensor->pin, GPIO_DIRECTION_IN);
	//*direction &= ~mask; //Set port to input

	delay_us( 70 );
	response=gpio_get_pin_level(sensor->pin);
	//response = *portin & mask; //Read input

	delay_us( 200 );
	
	//gpio_set_pin_level(sensor->pin,1);
	//gpio_set_pin_direction(sensor->pin, GPIO_DIRECTION_OUT);
	

	//*port |= mask; //Write 1 to output
	//*direction |= mask; //Set port to output

	delay_us( 500 );

	//SREG = sreg; //Restore status register

	return response != 0 ? ONEWIRE_ERROR_COMM : ONEWIRE_ERROR_OK;
}

//! Sends a single bit over the 1wire bus
uint8_t onewireWriteBit(sensor_t * sensor, uint8_t bit )
{
	//uint8_t sreg = SREG;

	#ifdef ONEWIRE_AUTO_CLI
		//cli( );
	#endif
	gpio_set_pin_level(sensor->pin,1);
	gpio_set_pin_direction(sensor->pin, GPIO_DIRECTION_OUT);
	gpio_set_pin_level(sensor->pin,0);
	//*port |= mask; //Write 1 to output
	//*direction |= mask;
	//*port &= ~mask; //Write 0 to output

	if ( bit == 0 ) delay_us( 50 );
	else delay_cycles(5);
	
	gpio_set_pin_direction(sensor->pin, GPIO_DIRECTION_IN);
	//*port |= mask;

	if ( bit == 0 ) delay_cycles(200);
	else delay_us( 50 );


	//SREG = sreg;

	return bit != 0;
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
	//uint8_t sreg = SREG;

	#ifdef ONEWIRE_AUTO_CLI
		//cli( );
	#endif
	gpio_set_pin_level(sensor->pin,0);
	gpio_set_pin_direction(sensor->pin, GPIO_DIRECTION_OUT);

	//*port |= mask; //Write 1 to output
	//*direction |= mask;
	//*port &= ~mask; //Write 0 to output
	delay_cycles(3);
	gpio_set_pin_direction(sensor->pin, GPIO_DIRECTION_IN);
	//*direction &= ~mask; //Set port to input
	bit =(gpio_get_pin_level(sensor->pin) != 0 );
	//bit = ( ( *portin & mask ) != 0 ); //Read input
	delay_us( 60 );
	//SREG = sreg;

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
