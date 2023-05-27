#include <stdio.h>
#include <atmel_start.h>
#include "sonar_i2cxl.h"
#include "System_User.h"
#include "usb_printf.h"


void MB7040Init(I2C_MB7040 *i2c_mb7040, struct i2c_m_sync_desc* i2c, int16_t address, int32_t address_length)
{
	i2c_mb7040->i2c = i2c;
	i2c_mb7040->address = address;
	i2c_mb7040->address_length = address_length;
	i2c_m_sync_enable(i2c_mb7040->i2c);
}


void MB7040SetSlaveAddress(I2C_MB7040 *i2c_mb7040)
{
	i2c_m_sync_set_slaveaddr(i2c_mb7040->i2c, i2c_mb7040->address, i2c_mb7040->address_length);
}


uint16_t MB7040ConvertToUint16(uint8_t* buffer)
{
	return buffer[0] << 8 | buffer[1];
}

uint16_t MB7040GetRegisterValue(I2C_MB7040 *i2c_mb7040, uint8_t registerAddress)
{
	MB7040SetSlaveAddress(i2c_mb7040);
	
	uint8_t buffer[2];
	//int32_t result = i2c_m_sync_cmd_read(I2C_SENSOR_ULTRASONIC, registerAddress, buffer, sizeof(buffer));
	//if (result != 0)
	//{
		//printf("Failed to read from INA260 device, error code %ld\r\n", result);
	//}
	struct io_descriptor *I2C_0_io;
	i2c_m_sync_get_io_descriptor(I2C_SENSOR_ULTRASONIC, &I2C_0_io);
	if (io_read(I2C_0_io, &buffer[0], sizeof(buffer)) < 1)
	{
		buffer[0]=0;
		buffer[1]=0;
		usb_printf(KRED "Problem with Ultrasonic Sensor!! \n\r"KNRM);
	}
	
	//io_read(I2C_0_io, (buffer + 1), 1);
	
	//printf("buffer1: %d\r\n",buffer[0]);
	//printf("buffer2: %d\r\n",buffer[1]);

	return MB7040ConvertToUint16(buffer);
}

uint16_t MB7040GetLevelReading(I2C_MB7040 *i2c_mb7040, uint8_t registerAddress)
{
	return MB7040GetRegisterValue(i2c_mb7040, registerAddress);
}



void MB7040SetForAReaing(I2C_MB7040 *i2c_mb7040)
{
	MB7040SetRegisterValue(i2c_mb7040, SONAR_I2CXL_RANGE_COMMAND);
}

void MB7040SetRegisterValue(I2C_MB7040 *i2c_mb7040, uint8_t registerAddress)
{
	//uint8_t buffer[1];
	//buffer[1]= registerAddress;
	//MB7040SetSlaveAddress(i2c_mb7040);
	//printf("addr: %02x\r\n", registerAddress);
	//printf("a.....\r\n");
	struct io_descriptor *I2C_0_io;
	i2c_m_sync_get_io_descriptor(I2C_SENSOR_ULTRASONIC, &I2C_0_io);
	//io_write(I2C_0_io, buffer, sizeof(buffer));
	io_write(I2C_0_io, &registerAddress, sizeof(registerAddress));

}

