/*
 * sonar_i2cxl_user.h
 *
 * Created: 1/05/2019 5:54:53 p. m.
 *  Author: Daniel_Moreno
 */ 


#ifndef SONAR_I2CXL_USER_H_
#define SONAR_I2CXL_USER_H_

//Distance
	#define avgcounter_US 50
	
//UltraSound intervals
	#define US_measurement_interval 100 //time between measurements in miliseconds (min time: 50)	

uint16_t MB7040_get_avg_measure (I2C_MB7040 *i2c_US_object, uint8_t address);

#endif /* SONAR_I2CXL_USER_H_ */