#ifndef I2CXL_SONAR_H_
#define I2CXL_SONAR_H_

#include <stdint.h>


#define SONAR_I2CXL_DEFAULT_ADDRESS				0x70		///< Address of the device
#define SONAR_I2CXL_RANGE_COMMAND				0x51		///< Address of the Range Command Register
#define SONAR_I2CXL_CHANGE_ADDRESS_COMMAND_1	0xAA		///< Address of the Change Command address Register 1
#define SONAR_I2CXL_CHANGE_ADDRESS_COMMAND_2	0xA5		///< Address of the Change Command address Register 2

//Sensor reading commands
	//UltraSound commands
	#define ADD_I2C_US_MAIN	0x70	//112 Dec
	#define ADD_I2C_US_SEC	0xA0	//160 Dec
	#define RangeCommand	0x51	//81 Dec

typedef struct I2C_MB7040 I2C_MB7040;
struct I2C_MB7040
{
	struct i2c_m_sync_desc *i2c;
	int16_t address;
	int32_t address_length;
};


void MB7040Init(I2C_MB7040 *i2c_mb7040, struct i2c_m_sync_desc* i2c, int16_t address, int32_t address_length);
void MB7040SetSlaveAddress(I2C_MB7040 *i2c_mb7040);
uint16_t MB7040ConvertToUint16(uint8_t* buffer);
uint16_t MB7040GetRegisterValue(I2C_MB7040 *i2c_mb7040, uint8_t registerAddress);
uint16_t MB7040GetLevelReading(I2C_MB7040 *i2c_mb7040, uint8_t registerAddress);
void MB7040SetForAReaing(I2C_MB7040 *i2c_mb7040);
void MB7040SetRegisterValue(I2C_MB7040 *i2c_mb7040, uint8_t registerAddress);

#endif /* I2CXL_SONAR_H */