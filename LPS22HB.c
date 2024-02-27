/*
 * LPS22HB.c
 *
 *  Created on: Dec 9, 2023
 *      Author: Adam Iwachów
 */



#include <LPS22HB.h>

#define I2C_MAX_TIME 100
#define Is_Pressure_DataReady(REG) ((REG) & 0x01)


/*
 *  I2C COMMUNICATION POLLING FUNCTIONS
 */
 static HAL_StatusTypeDef  LPS22HB_Read_Reg(LPS22HB_t *dev, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, 1, I2C_MAX_TIME);
}

 static HAL_StatusTypeDef  LPS22HB_Read_Regs(LPS22HB_t *dev, uint8_t reg, uint8_t *data, uint8_t length)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, length, I2C_MAX_TIME);
}

 static HAL_StatusTypeDef  LPS22HB_Write_Reg(LPS22HB_t *dev, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Write(dev->i2cHandle, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, 1, I2C_MAX_TIME);
}



uint8_t LPS22HB_INIT(LPS22HB_t *dev, I2C_HandleTypeDef *i2cHandle, uint8_t address)
 {
	 uint8_t Data_write = 0;
	 uint8_t Data_read = 0;

		dev->i2cHandle		= i2cHandle;
		dev->address 		= address;
		dev->pressure_hPa 	= 0.0f;
		dev->temp_cel 		= 0.0f;
		dev->altitude		= 0.0f;

	/* Check twice if the device is alive */

	if(HAL_I2C_IsDeviceReady(dev->i2cHandle, dev->address, 2, I2C_MAX_TIME) == HAL_OK){

		CheckI2Cfaults(LPS22HB_Read_Reg(dev,  LPS22HB_WHO_AM_I, &Data_read));
	}

	 if(Data_read != 0xB1){
		 return 0xFF;
	 }

	 Data_read = 0;

	 /* if ok lets do initialization */


	    /* Datasheet p. 37 */
		Data_write = (0x20 | 0x00); // Enable continuous mode with low pass filter and data rate = 10 Hz
		CheckI2Cfaults(LPS22HB_Write_Reg(dev, LPS22HB_CTRL_REG1, &Data_write));

		HAL_Delay(25);


		LPS22HB_SET_CALIB(dev, 1 * 16);

		// enable interrupts

		Data_write = 0x04; // enable interrupt on Data-ready, active high
		CheckI2Cfaults(LPS22HB_Write_Reg(dev,  LPS22HB_CTRL_REG3, &Data_write));

		HAL_Delay(25);

 }

HAL_StatusTypeDef LPS22HB_READ_TEMP(LPS22HB_t *dev)
{
	 uint8_t RawTemp[2];

	 CheckI2Cfaults(LPS22HB_Read_Regs(dev,  LPS22HB_TEMP_OUT_L, RawTemp, 2));

	 dev->temp_cel =((int16_t)(RawTemp[0] | RawTemp[1] << 8))/ 100.0f;
}


HAL_StatusTypeDef LPS22HB_READ_PRESSURE(LPS22HB_t *dev)
{
	 uint8_t RawPress[3];

	 CheckI2Cfaults(LPS22HB_Read_Regs(dev,  LPS22HB_PRESS_OUT_XL, RawPress, 3));

	 dev->pressure_hPa =((uint32_t)(RawPress[0] | RawPress[1] << 8 | RawPress[2] << 16)) / 4096.0f;
}

uint8_t LPS22HB_PRESSURE_READY(LPS22HB_t *dev)
{
	 uint8_t Data_read = 0;
	 CheckI2Cfaults(LPS22HB_Read_Reg(dev, LPS22HB_STATUS, &Data_read));
	 return Is_Pressure_DataReady(Data_read);
}



void LPS22HB_SET_CALIB(LPS22HB_t *dev, uint16_t value)
{
	uint8_t Data[2];
	Data[1] =  (uint8_t)(value / 256U);
	Data[0] =  (uint8_t)(value - (Data[1] * 256U));

	CheckI2Cfaults(LPS22HB_Write_Reg(dev, LPS22HB_RPDS_L, &Data[0]));
	CheckI2Cfaults(LPS22HB_Write_Reg(dev, LPS22HB_RPDS_H, &Data[1]));

}
