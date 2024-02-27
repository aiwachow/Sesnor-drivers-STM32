/*
 * LPS25HB_I2C.c
 *
 *  Created on: Sep 22, 2023
 *      Author: Adam
 */

#include <LPS25HB_I2C.h>
#include <math.h>

typedef enum {false = 0, true = 1} bool;

#define I2C_MAX_TIME 100


/*
 *  I2C COMMUNICATION POLLING FUNCTIONS
 */
 static HAL_StatusTypeDef  LPS25HB_Read_Reg(LPS25HB_t *dev, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, 1, I2C_MAX_TIME);
}

 static HAL_StatusTypeDef  LPS25HB_Read_Regs(LPS25HB_t *dev, uint8_t reg, uint8_t *data, uint8_t length)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, length, I2C_MAX_TIME);
}

 static HAL_StatusTypeDef  LPS25HB_Write_Reg(LPS25HB_t *dev, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Write(dev->i2cHandle, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, 1, I2C_MAX_TIME);
}

/*
 * I2C COMMUNICATION NON BLOCKING DIRECT MEMORY ACCESS FUNCTIONS
 */

 static HAL_StatusTypeDef  LPS25HB_Read_Regs_DMA(LPS25HB_t *dev, uint8_t reg, uint8_t *data, uint8_t length)
{
	return HAL_I2C_Mem_Read_DMA(dev->i2cHandle, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, length);
}

 static HAL_StatusTypeDef  LPS25HB_Read_Regs_IT(LPS25HB_t *dev, uint8_t reg, uint8_t *data, uint8_t length)
{
	return HAL_I2C_Mem_Read_IT(dev->i2cHandle, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, length);
}



static uint8_t LPS25HB_ID_CORRECT(LPS25HB_t *dev)
{
	HAL_StatusTypeDef Status;
	uint8_t errors = 0, Data = 0;

 	Status = LPS25HB_Read_Reg(dev, LPS25HB_WHO_AM_I, &Data);
 	errors += (Status != HAL_OK);

 	return (Data == 0xBD) ?  true : false;

}




 HAL_StatusTypeDef LPS25HB_INIT(LPS25HB_t *dev, I2C_HandleTypeDef *i2cHandle, uint8_t address)
{
	HAL_StatusTypeDef Status;
	uint8_t errors = 0, Data = 0;

	dev->i2cHandle = i2cHandle;
	dev->address = address;
	dev->pressure_hPa = 0.0f;
	dev->temp_cel = 0.0f;
	dev->altitude = 0.0f;
	dev->Start_press_hPa = 1.0f;


	if(!LPS25HB_ID_CORRECT(dev) || (HAL_I2C_IsDeviceReady(dev->i2cHandle, dev->address, 2, I2C_MAX_TIME) != HAL_OK)){
		return errors = 0xff;
	}


	Data = 0xC0; // Enable LPS and set 25Hz data rate
	Status = LPS25HB_Write_Reg(dev, LPS25HB_CTRL_REG1, &Data);
	errors += (Status != HAL_OK);
	HAL_Delay(100);

	Data = 0x00; // Disable interrupt
	Status = LPS25HB_Write_Reg(dev, LPS25HB_CTRL_REG4, &Data);
	errors += (Status != HAL_OK);

	LPS25HB_ENABLE_INT(dev);

	LPS25HB_READ_PRESSURE(dev);
	LPS25HB_READ_TEMP(dev);



	return errors;
}
 HAL_StatusTypeDef LPS25HB_ENABLE_INT(LPS25HB_t *dev)
 {
	 HAL_StatusTypeDef Status;
	 uint8_t error = 0, Data = 0;

	Data = 0x01; // enable data ready interrupt
	Status = LPS25HB_Write_Reg(dev, LPS25HB_CTRL_REG4, &Data);
	error += (Status != HAL_OK);


		return error;


 }


float LPS25HB_FIFO_MEAN(LPS25HB_t *dev)
{
		HAL_StatusTypeDef Status;
		uint8_t errors = 0, Data = 0;


		Data = 0x40; // enable fifo
		Status = LPS25HB_Write_Reg(dev, LPS25HB_CTRL_REG2, &Data);
		errors += (Status != HAL_OK);

		Data = 0xDF; // 32 mean
		Status = LPS25HB_Write_Reg(dev, LPS25HB_FIFO_CTRL, &Data);
		errors += (Status != HAL_OK);

		Status = LPS25HB_Read_Reg(dev, LPS25HB_FIFO_STATUS, &Data);
		errors += (Status != HAL_OK);

		while((Data & 0x0F) != 0x0F){
			LPS25HB_Read_Reg(dev, LPS25HB_FIFO_STATUS, &Data);
		}

		LPS25HB_READ_PRESSURE(dev);

		return dev->pressure_hPa;
}

uint8_t  LPS25HB_FIFO_STATUS_CHECK(LPS25HB_t *dev)
{
	HAL_StatusTypeDef Status;
	uint8_t Data = 0;

	if((Status = LPS25HB_Read_Reg(dev, LPS25HB_FIFO_STATUS, &Data)) != HAL_OK)
	 return 0xff;

	return Status;
}


HAL_StatusTypeDef LPS25HB_READ_TEMP(LPS25HB_t *dev)
{
	 HAL_StatusTypeDef Status;
	 uint8_t errors = 0;
	 uint8_t RawTemp[2];

	 Status = LPS25HB_Read_Regs(dev, LPS25HB_TEMP_OUT_L | 0x80, RawTemp, 2); // set msb enable auto increment p.25
	 errors += (Status != HAL_OK);

	 dev->temp_cel = 42.5f + ((int16_t)(RawTemp[0] | RawTemp[1] << 8)) / 480.0f;

	 return errors;
}


HAL_StatusTypeDef LPS25HB_READ_PRESSURE(LPS25HB_t *dev)
{
	 HAL_StatusTypeDef Status;
	 uint8_t errors = 0;

	 uint8_t RawPress[3];

	 Status = LPS25HB_Read_Regs(dev, LPS25HB_PRESS_OUT_XL | 0x80, RawPress, 3);
	 errors += (Status != HAL_OK);

	 if(dev->MEAN_flag)
	 {
		 dev->Start_press_hPa =((uint32_t)(RawPress[0] | RawPress[1] << 8 | RawPress[2] << 16)) / 4096.0f;
		 dev->MEAN_flag = 0;
	 }


	 dev->pressure_hPa =((uint32_t)(RawPress[0] | RawPress[1] << 8 | RawPress[2] << 16)) / 4096.0f;

	 return errors;
}

HAL_StatusTypeDef LPS25HB_PRESSURE_OFFSET(LPS25HB_t *dev, int16_t offset)
{
	 HAL_StatusTypeDef Status;
	 uint8_t errors = 0;


	 Status = LPS25HB_Write_Reg(dev, LPS25HB_RPDS_L, (uint8_t *)&offset);
	 errors += (Status != HAL_OK);

	 Status = LPS25HB_Write_Reg(dev, LPS25HB_RPDS_H, (uint8_t *)(offset >> 8));
	 errors += (Status != HAL_OK);


	 return errors;
}

inline uint8_t LPS25HB_READ_PRESS_DMA(LPS25HB_t *dev)
{
   return LPS25HB_Read_Regs_DMA(dev, LPS25HB_PRESS_OUT_XL | 0x80, (uint8_t *) dev->RawPress, 3);

}

inline void LPS25HB_READ_PRESS_DMA_COMPLETE(LPS25HB_t *dev)
{
	dev->pressure_hPa =((uint32_t)(dev->RawPress[0] | dev->RawPress[1] << 8 | dev->RawPress[2] << 16)) / 4096.0f;
}






inline void LPS25HB_ESTIMATE_ALTITUDE(LPS25HB_t *dev)
{
	dev->altitude = -29.271769 * dev->temp_cel * log(dev->pressure_hPa / dev->Start_press_hPa);
}





void SWAP_TO_POLLING_MODE(void)
{

}

