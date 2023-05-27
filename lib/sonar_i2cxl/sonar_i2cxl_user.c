/*
 * sonar_i2cxl_user.c
 *
 * Created: 1/05/2019 5:54:43 p. m.
 *  Author: Daniel_Moreno
 */ 

#include <atmel_start.h>
#include "sonar_i2cxl.h"
#include "sonar_i2cxl_user.h"
#include "usb_printf.h"

uint16_t MB7040_get_avg_measure (I2C_MB7040 *i2c_US_object, uint8_t address){
	uint8_t i=0, j=0;
	uint16_t tmp, US_avg=0;
	uint16_t US_array[avgcounter_US];

	for(i=0;i<avgcounter_US;i++){
		///Get river level in sensor
		MB7040SetForAReaing(i2c_US_object);
		delay_ms(US_measurement_interval);
		US_array[i] = MB7040GetLevelReading(i2c_US_object,address);		
	}

	//sorting
	for (i = 0; i < avgcounter_US; i++)                     //Loop for ascending ordering
	{
		for (j = 0; j < avgcounter_US; j++)             //Loop for comparing other values
		{
			if (US_array[j] > US_array[i])                //Comparing other array elements
			{
				tmp = US_array[i];         //Using temporary variable for storing last value
				US_array[i] = US_array[j];            //replacing value
				US_array[j] = tmp;             //storing last value
			}
		}
	}
	
	for (i = 0; i < avgcounter_US; i++)                     
	{
		usb_printf("Array i: %d, valor: %d\n\r",i,US_array[i]);
	}

	uint8_t band=2;
	j=0;
	for (i = avgcounter_US/2-band; i < avgcounter_US/2+band; i++)
	{
		US_avg+=US_array[i];
		j++;
	}
	
	//averaging eachsensor measurements
	US_avg/=j;
	
	//damage verification
	if(US_avg==0){
		usb_printf("\t\t--WARNING: sensor disconnected/damaged\r\n");
	}
	
	return US_avg;

}