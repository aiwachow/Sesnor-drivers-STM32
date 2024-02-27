/*
 * aht20.c
 *
 *  Created on: Aug 22, 2023
 *      Author: Adam Iwachów
 */


#include "stm32f4xx_hal.h"
#include "i2c.h"

#include "aht20.h"
#include "math.h"




// I2C communication protocol

 HAL_StatusTypeDef AHT20_Read_Reg(AHT20_t *dev, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, AHT20_I2C_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY);
}

 HAL_StatusTypeDef AHT20_Read_Regs(AHT20_t *dev, uint8_t reg, uint8_t *data, uint8_t length)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, AHT20_I2C_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, data, length, HAL_MAX_DELAY);
}

 HAL_StatusTypeDef AHT20_Write_Reg(AHT20_t *dev, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Write(dev->i2cHandle, AHT20_I2C_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY);
}

 HAL_StatusTypeDef AHT20_Write_Regs(AHT20_t *dev, uint8_t reg, uint8_t *data,uint8_t length)
{
	return HAL_I2C_Mem_Write(dev->i2cHandle, AHT20_I2C_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, data, length, HAL_MAX_DELAY);
}

uint8_t AHT20_INIT(AHT20_t *dev, I2C_HandleTypeDef *i2cHandle)
{

	dev->i2cHandle = i2cHandle;

	HAL_StatusTypeDef Status;
	uint8_t errors = 0, data8 = 0;

	dev->temp_C   = 0.0f;
	dev->humidity_pct = 0.0f;

//	 SOFT RESET


	Status = AHT20_Read_Reg(dev, AHT20_STATUS, &data8);
	errors += (Status != HAL_OK);

	// Initialization command
	uint8_t data16[2] = {0x08, 0x00};
	Status = AHT20_Write_Regs(dev, AHT20_REG_INIT, data16, 2);
	errors += (Status != HAL_OK);
	HAL_Delay(50);

	// Check the calibration status
	Status = AHT20_Read_Reg(dev, AHT20_STATUS, &data8);
	errors += (Status != HAL_OK);
	while((~(data8 >> 3) & 0x01)){ 						// wait until calibration is set
		AHT20_Read_Reg(dev, AHT20_STATUS, &data8);
	}




	return errors;

}
uint8_t AHT20_READ_TEMP_HUMIDITY(AHT20_t *dev)
{

	HAL_StatusTypeDef Status;
	uint8_t errors = 0, data8 = 0;
	uint8_t raw_data[7] = {0};

	// Trigger the measurement
	uint8_t data16[2] = {0x33, 0x00};
	Status = AHT20_Write_Regs(dev, AHT20_REG_TRIG_MEASURE, data16, 2);
	errors += (Status != HAL_OK);
	HAL_Delay(75);


	// Check if the measurement is complited
	AHT20_Read_Reg(dev, AHT20_STATUS, &data8);
	while(((data8 >> 7) & 0x01)){						// wait until measure is completed
			AHT20_Read_Reg(dev, AHT20_STATUS, &data8);
		}

	// Read raw data
	Status = AHT20_Read_Regs(dev, AHT20_STATUS, raw_data, 7);
	errors += (Status != HAL_OK);

	uint32_t RAW_HUMIDITY_DATA =  ((uint32_t) raw_data[1] << 16 | (uint32_t) raw_data[2] << 8 | (uint32_t) raw_data[3]) >> 4;
	int32_t RAW_TEMP_DATA	  =  ((int32_t)(raw_data[3] & 0x0F) << 16 | (int32_t) raw_data[4] << 8 | (int32_t) raw_data[5]);


	dev->humidity_pct = (RAW_HUMIDITY_DATA/(float)(1<<20)) * 100;
	dev->temp_C   	  = (RAW_TEMP_DATA/(float)(1<<20)) * 200 - 50;



	return errors;


}


