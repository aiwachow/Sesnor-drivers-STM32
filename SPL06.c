/*
 * SPL06.h
 *
 *  Created on: Dec 9, 2023
 *      Author: Adam
 */

#ifndef SRC_SPL06_C_
#define SRC_SPL06_C_

#include <SPL06.h>
#define I2C_MAX_TIME 100


#define Is_Pressure_DataReady(REG) ((REG) & 0x30)

/*
 *  I2C COMMUNICATION POLLING FUNCTIONS
 */
 static HAL_StatusTypeDef  SPL06_Read_Reg(SPL06_t *dev, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, 1, I2C_MAX_TIME);
}

 static HAL_StatusTypeDef  SPL06_Read_Regs(SPL06_t *dev, uint8_t reg, uint8_t *data, uint8_t length)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, length, I2C_MAX_TIME);
}

 static HAL_StatusTypeDef  SPL06_Write_Reg(SPL06_t *dev, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Write(dev->i2cHandle, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, 1, I2C_MAX_TIME);
}

/*
 * I2C COMMUNICATION NON BLOCKING DIRECT MEMORY ACCESS FUNCTIONS
 */

 static HAL_StatusTypeDef  SPL06_Read_Regs_DMA(SPL06_t *dev, uint8_t reg, uint8_t *data, uint8_t length)
{
	return HAL_I2C_Mem_Read_DMA(dev->i2cHandle, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, length);
}


static uint8_t FirstMeasurment = 1;




 uint8_t SPL06_INIT(SPL06_t *dev, I2C_HandleTypeDef *i2cHandle, uint8_t address)
  {
 	 HAL_StatusTypeDef Status;
 	 uint8_t errors = 0;
 	 uint8_t Data_write = 0;
 	 uint8_t Data_read = 0;


        dev->ALPHA			= 0.2f;
 		dev->i2cHandle		= i2cHandle;
 		dev->address 		= address;
 		dev->temp_cel 		= 0.0f;
 		dev->altitude		= 0.0f;

 	/* Check twice if the device is alive */

 	if(HAL_I2C_IsDeviceReady(dev->i2cHandle, dev->address, 2, I2C_MAX_TIME) == HAL_OK){
 	 Status = SPL06_Read_Reg(dev,  SPL06_WHO_AM_I, &Data_read);
 	 errors += (Status != HAL_OK);
 	}
 	 if(Data_read != 0x10){
 		 return 0xFF;
 	 }

 	 Data_read = 0;

 	 /* if ok lets do initialization
 	  * first get the coefficients */

 	 // Wait for coefficients and sensor in general to be ready p. 23

 	Status = SPL06_Read_Reg(dev,  SPL06_MEAS_CFG, &Data_read);
 	errors += (Status != HAL_OK);

 	 while((Data_read & 0xC0) != 0xC0 ){
 		SPL06_Read_Reg(dev,  SPL06_MEAS_CFG, &Data_read);
 	 }


 	 uint8_t Coefficient_Data[18] = {0};
 	  Status = SPL06_Read_Regs(dev,  COEF_START, Coefficient_Data, 18);
 	  errors += (Status != HAL_OK);

 	 dev->c0 = ((uint16_t)Coefficient_Data[0] << 4) | ((uint16_t)Coefficient_Data[1] >> 4);
 	 dev->c1 = ((uint16_t)(Coefficient_Data[1] & 0x0F) << 8) | (uint16_t)Coefficient_Data[2];
 	 dev->c00 = ((uint32_t)Coefficient_Data[3] << 12) | ((uint32_t)Coefficient_Data[4] << 4) | ((uint16_t)Coefficient_Data[5] >> 4);
 	 dev->c10 = ((uint32_t)(Coefficient_Data[5] & 0x0F) << 16) | ((uint32_t)Coefficient_Data[6] << 8) | (uint32_t)Coefficient_Data[7];

 	 dev->c01 = ((uint16_t)Coefficient_Data[8] << 8) | (uint16_t)Coefficient_Data[9];
 	 dev->c11 = ((uint16_t)Coefficient_Data[10] << 8) | (uint16_t)Coefficient_Data[11];
 	 dev->c20 = ((uint16_t)Coefficient_Data[12] << 8) | (uint16_t)Coefficient_Data[13];
 	 dev->c21 = ((uint16_t)Coefficient_Data[14] << 8) | (uint16_t)Coefficient_Data[15];
 	 dev->c30 = ((uint16_t)Coefficient_Data[16] << 8) | (uint16_t)Coefficient_Data[17];

 	 // Check if MSB coefficient is signed
 	 dev->c0 = (dev->c0 & (1 << 11)) ? (dev->c0 | 0xF000) : dev->c0;
 	 dev->c1 = (dev->c1 & (1 << 11)) ? (dev->c1 | 0xF000) : dev->c1;
 	 dev->c00 = (dev->c00 & (1 << 19)) ? (dev->c00 | 0xFFF00000) : dev->c00;
 	 dev->c10 = (dev->c10 & (1 << 19)) ? (dev->c10 | 0xFFF00000) : dev->c10;
 	 dev->c01 = (dev->c01 & (1 << 11)) ? (dev->c01 | 0xF000) : dev->c01;
 	 dev->c11 = (dev->c11 & (1 << 11)) ? (dev->c11 | 0xF000) : dev->c11;
 	 dev->c20 = (dev->c20 & (1 << 11)) ? (dev->c20 | 0xF000) : dev->c20;
 	 dev->c21 = (dev->c21 & (1 << 11)) ? (dev->c21 | 0xF000) : dev->c21;
 	 dev->c30 = (dev->c30 & (1 << 11)) ? (dev->c30 | 0xF000) : dev->c30;



 		// Pressure Configuation 8Hz rate, 8x oversampling

 		Data_write = (0x30 | 0x03);

 		Status = SPL06_Write_Reg(dev, SPL06_PRS_CFG, &Data_write);
 		errors += (Status != HAL_OK);
 		HAL_Delay(25);

 		// Temperature Configuation 8Hz rate, 8x oversampling

 		Data_write = (0xB0 | 0x03);

 	 	Status = SPL06_Write_Reg(dev, SPL06_TMP_CFG, &Data_write);
 	 	errors += (Status != HAL_OK);
 	 	HAL_Delay(25);

 	 	//  measurement mode: temp and press continous

 		Data_write = 0x07;

 	 	Status = SPL06_Write_Reg(dev, SPL06_MEAS_CFG, &Data_write);
 	 	errors += (Status != HAL_OK);
 	 	HAL_Delay(25);


 		//  Enable data ready interrupt

 	 		Data_write = 0xA0;

 	 	 	Status = SPL06_Write_Reg(dev, SPL06_CFG_REG, &Data_write);
 	 	 	errors += (Status != HAL_OK);
 	 	 	HAL_Delay(25);


 	 	 	dev->pressure_hPa = 0.0f;

 	 	 	return errors;

  }

 HAL_StatusTypeDef SPL06_READ_TEMP_PRESS(SPL06_t *dev)
 {
 	 HAL_StatusTypeDef Status;
 	 uint8_t errors = 0;
 	 uint8_t RawData[6];
 	 float ScalingFactor = 7864320.0f;
 	 int32_t Traw_sc, Praw_sc;
 	 uint8_t Data_read = 0;

 	 Status = SPL06_Read_Regs(dev, SPL06_PSR_B2 , RawData, 6);
 	 errors += (Status != HAL_OK);

 	 Praw_sc = (((uint32_t)RawData[0] << 16) | ((uint32_t)RawData[1] << 8) | ((uint32_t)RawData[2]));
 	 Praw_sc = (Praw_sc & 1 << 23) ? (Praw_sc | 0xFF000000) : Praw_sc;

 	 Traw_sc = (((uint32_t)RawData[3] << 16) | ((uint32_t)RawData[4] << 8) | ((uint32_t)RawData[5]));
 	 Traw_sc = ( Traw_sc & 1 << 23) ? ( Traw_sc | 0xFF000000) :  Traw_sc;


 	float tempraw = (float) Traw_sc / ScalingFactor;
 	float presraw = (float) Praw_sc / ScalingFactor;

 	 dev->temp_cel = dev->c0*0.5f + dev->c1*tempraw;


 	 dev->pressure_hPa = ((dev->c00 + presraw * (dev->c10 + presraw * (dev->c20 + presraw * dev->c30)) +
 		tempraw * dev->c01 + tempraw * presraw * (dev->c11 + presraw*dev->c21))) / 100.0f;



 	 Status = SPL06_Read_Reg(dev, SPL06_INT_STS, &Data_read);
 	 return errors;
 }


 uint8_t SPL06_PRESS_AND_TEMP_READY(SPL06_t *dev)
 {
 	HAL_StatusTypeDef Status;
 	 uint8_t Data_read = 0;
 	 if((Status = SPL06_Read_Reg(dev, SPL06_MEAS_CFG, &Data_read)) != HAL_OK)
 		 return 0xff;

 	 return Is_Pressure_DataReady(Data_read);
 }

#endif /* SRC_SPL06_C_ */
