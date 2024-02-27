/*
 * BMI088.c
 *
 *  Created on: Dec 11, 2023
 *      Author: Adam Iwachów
 */


#include <BMI088.h>
#define I2C_MAX_TIME 100
#define IS_BIT_SET(DATA, BIT) ((DATA & (1 << BIT)) >> BIT)

/*
 *  I2C COMMUNICATION POLLING FUNCTIONS
 */
// select: 0 - acc, 1 - gyro
 static HAL_StatusTypeDef  BMI088_Read_Reg(BMI088_t *dev, uint8_t reg, uint8_t *data, uint8_t select)
{
	if(select){
		return HAL_I2C_Mem_Read(dev->i2cHandle, dev->GYROaddress, reg, I2C_MEMADD_SIZE_8BIT, data, 1, I2C_MAX_TIME);

	}
	else{
		return HAL_I2C_Mem_Read(dev->i2cHandle, dev->ACCaddress, reg, I2C_MEMADD_SIZE_8BIT, data, 1, I2C_MAX_TIME);
	}
}

 static HAL_StatusTypeDef  BMI088_Read_Regs(BMI088_t *dev, uint8_t reg, uint8_t *data, uint8_t length,uint8_t select)
{
		if(select){
			return HAL_I2C_Mem_Read(dev->i2cHandle, dev->GYROaddress, reg, I2C_MEMADD_SIZE_8BIT, data, length, I2C_MAX_TIME);
		}
		else{
			return HAL_I2C_Mem_Read(dev->i2cHandle, dev->ACCaddress, reg, I2C_MEMADD_SIZE_8BIT, data, length, I2C_MAX_TIME);
			}
}

 static HAL_StatusTypeDef  BMI088_Write_Reg(BMI088_t *dev, uint8_t reg, uint8_t *data, uint8_t select)
{
		if(select){
			return HAL_I2C_Mem_Write(dev->i2cHandle, dev->GYROaddress, reg, I2C_MEMADD_SIZE_8BIT, data, 1, I2C_MAX_TIME);
			}
		else{
			return HAL_I2C_Mem_Write(dev->i2cHandle, dev->ACCaddress, reg, I2C_MEMADD_SIZE_8BIT, data, 1, I2C_MAX_TIME);
			}
}

 static uint16_t CheckErrors(HAL_StatusTypeDef Status)
 {
	 uint8_t errors = 0x00; // MSB - Hard errors (HAL_ERROR),  LSB - soft errors(HAL_BUSY)

	 if(Status != HAL_OK)
	 {
		 if(Status == HAL_ERROR){
			 errors += (1 << 8);

		 }
		 else if(Status == HAL_BUSY){
			 errors++;

		 }
	 }

	 return errors;

 }





/*
 * I2C COMMUNICATION NON BLOCKING DIRECT MEMORY ACCESS FUNCTION
 */

 static HAL_StatusTypeDef  BMI088_Read_Regs_DMA(BMI088_t *dev, uint8_t reg, uint8_t *data, uint8_t length, uint8_t select)
{
	 if(select){
		return HAL_I2C_Mem_Read_DMA(dev->i2cHandle, dev->GYROaddress, reg, I2C_MEMADD_SIZE_8BIT, data, length);
	 	 	 	}
	else{
	 return HAL_I2C_Mem_Read_DMA(dev->i2cHandle, dev->ACCaddress, reg, I2C_MEMADD_SIZE_8BIT, data, length);
	 	}
}


static uint8_t BMI088_ID_CORRECT(BMI088_t *dev, uint8_t ChipID, uint8_t select)
{
	HAL_StatusTypeDef Status;
	uint8_t errors = 0, Data = 0;

 	Status = BMI088_Read_Reg(dev, ChipID, &Data, select);
 	errors += (Status != HAL_OK);
 	return Data;

}

uint8_t BMI088_INIT(BMI088_t *dev, I2C_HandleTypeDef *i2cHandle)
 {
	 HAL_StatusTypeDef Status;
	 uint8_t errors = 0;
	 uint8_t Data_write = 0;
	 uint8_t Data_read = 0;

		dev->i2cHandle		= i2cHandle;
		dev->ACCaddress 	= BMI088_ACC_I2C_ADDR;
		dev->GYROaddress 	= BMI088_GYR_I2C_ADDR;

		dev->Gyr_ScaleFactor= 0.0f;
		dev->Acc_ScaleFactor= 0.0f;

		dev->acc_ms2[0] = 0.0f;
		dev->acc_ms2[1] = 0.0f;
		dev->acc_ms2[2] = 0.0f;

		dev->gyr_degs[0] = 0.0f;
		dev->gyr_degs[1] = 0.0f;
		dev->gyr_degs[2] = 0.0f;

		dev->Temperature = 0.0f;

		dev->GYROrange = BMI088_GYRO_SCALE_1000DPS;
		dev->ACCrange  = BMI088_ACCEL_FS_SEL_3G;


	/* Check twice if the device is alive */
    HAL_Delay(10);
	if(HAL_I2C_IsDeviceReady(dev->i2cHandle, dev->ACCaddress, 2, I2C_MAX_TIME) != HAL_OK ||
			HAL_I2C_IsDeviceReady(dev->i2cHandle, dev->GYROaddress, 2, I2C_MAX_TIME) != HAL_OK){
		 return 0xFF;
	}
	if(BMI088_ID_CORRECT(dev,BMI088_ACC_CHIP_ID, I2CAcc) != 0x1E ||  BMI088_ID_CORRECT(dev,BMI088_GYR_CHIP_ID, I2CGyro) != 0x0F){
		 return 0xFF;
	 }

	 Data_read = 0;

	 /* if ok lets do accelerometer initialization */

	 /* Note*:
	  * BMI088 has separate logic for accelerometer and gyroscope
	  * it means that gyro and acc works like two separate devices
	  */



	 // check first acc fatal errors
	 Status = BMI088_Read_Reg(dev, BMI088_ACC_ERR_REG ,&Data_read, I2CAcc);
	 if (IS_BIT_SET(Data_read, 0)){
	     // Do soft reset
		 Data_write = 0xB6;
		 BMI088_Write_Reg(dev, BMI088_ACC_SOFTRESET  ,&Data_write, I2CAcc);
		 errors += (Status != HAL_OK);
		 HAL_Delay(100);
	 }

     errors += (CheckErrors(BMI088_Read_Reg(dev, BMI088_ACC_ERR_REG ,&Data_read, I2CAcc)));


    // acc data rate and low pass
    // default data rate 100hz
	Data_write = 0xA8;
	Status = BMI088_Write_Reg(dev, BMI088_ACC_CONF ,&Data_write, I2CAcc);
	errors += (Status != HAL_OK);

	Status = BMI088_Write_Reg(dev, BMI088_ACC_RANGE ,&dev->ACCrange, I2CAcc);
	errors += (Status != HAL_OK);

	/* ACC INTERRUPTS */

	Data_write = 0x0A; // INT1 active high, output
	Status = BMI088_Write_Reg(dev, BMI088_INT1_IO_CONF ,&Data_write,  I2CAcc);
	errors += (Status != HAL_OK);

	Data_write = 0x04; // INT1 active high, output
	Status = BMI088_Write_Reg(dev, BMI088_INT1_INT2_MAP_DATA ,&Data_write,  I2CAcc);
	errors += (Status != HAL_OK);


	/*
	 * Note*:
	 * To switch acc into normal mode we have to:
	 * Power the sensor, wait 1ms
	 * write 4 to BMI088_ACC_PWR_CTRL
	 * wait 50ms
	 */

    // put acc in active mode
	Data_write = 0x00;
	Status = BMI088_Write_Reg(dev, BMI088_ACC_PWR_CONF ,&Data_write,  I2CAcc);
	errors += (Status != HAL_OK);
	 HAL_Delay(10);
	 // then check if acc is active
	Status = BMI088_Read_Reg(dev, BMI088_ACC_PWR_CONF ,&Data_read, I2CAcc);
	if(Data_read != 0x00){
	 	 return 0xFF;
	 	     // error handler (maybe in future)
	}

    // turn on acc
	Data_write = 0x04;
	Status = BMI088_Write_Reg(dev, BMI088_ACC_PWR_CTRL ,&Data_write,  I2CAcc);
	errors += (Status != HAL_OK);
	HAL_Delay(100);



	/* Pre-compute accelerometer conversion from LSB to acceleration (m/s^2) */
	// pow(2, content of the ACC_RANGE register + 1)
	switch(dev->ACCrange)
	{
	case 0x00:
		dev->Acc_ScaleFactor = 9.81f / 32768.0f * pow(2, 0 + 1) * 1.5f;
		break;
	case 0x01:
		dev->Acc_ScaleFactor = 9.81f / 32768.0f * pow(2, 1 + 1) * 1.5f;
			break;

	case 0x02:
		dev->Acc_ScaleFactor = 9.81f / 32768.0f * pow(2, 2 + 1) * 1.5f;
			break;
	case 0x03:
		dev->Acc_ScaleFactor = 9.81f / 32768.0f * pow(2, 3 + 1) * 1.5f;
		break;

	default:
		break;
	}






	// Gyroscope init



		   // Gyr data rate and low pass
		    // default data rate 100hz
			Data_write = 0x07;
			Status = BMI088_Write_Reg(dev, BMI088_GYR_BANDWIDTH ,&Data_write, I2CGyro);
			errors += (Status != HAL_OK);

			Status = BMI088_Write_Reg(dev, BMI088_GYR_RANGE ,&dev->GYROrange, I2CGyro);
			errors += (Status != HAL_OK);

			/* GYRO INTERRUPTS */

			Data_write = 0x80; // Enables the new data interrupt to be triggered
			Status = BMI088_Write_Reg(dev, BMI088_GYR_INT_CTRL ,&Data_write,  I2CGyro);
			errors += (Status != HAL_OK);

			Data_write = 0x01; // INT3 active high, push-pull
			Status = BMI088_Write_Reg(dev, BMI088_GYR_INT3_INT4_IO_CONF ,&Data_write,  I2CGyro);
			errors += (Status != HAL_OK);


			Data_write = 0x01;
			Status = BMI088_Write_Reg(dev, BMI088_GYR_INT3_INT4_IO_MAP ,&Data_write,  I2CGyro);
			errors += (Status != HAL_OK);



			/* Pre-compute gyroscope conversion from LSB to angular velocity (rads per second) */
			dev->Gyr_ScaleFactor = (1.0f / (180.0f / 3.14159f)) * 1000.0f / 32768.0f;


		return errors;
 }


uint8_t BMI088_ReadAcc(BMI088_t *dev)
{
	HAL_StatusTypeDef Status;
    uint8_t ACCRawData[6];
    uint8_t errors = 0;

    int16_t XYZ_ACC_datas[3];

	Status = BMI088_Read_Regs(dev, BMI088_ACC_DATA ,ACCRawData,6, I2CAcc);
	errors += (Status != HAL_OK);

	XYZ_ACC_datas[0] = ((int16_t) ACCRawData[1] << 8 | ((int16_t) ACCRawData[0]));
	XYZ_ACC_datas[1] = ((int16_t) ACCRawData[3] << 8 | ((int16_t) ACCRawData[2]));
	XYZ_ACC_datas[2] = ((int16_t) ACCRawData[5] << 8 | ((int16_t) ACCRawData[4]));

	dev->acc_ms2[0] = dev->Acc_ScaleFactor * XYZ_ACC_datas[0];
	dev->acc_ms2[1] = dev->Acc_ScaleFactor * XYZ_ACC_datas[1];
	dev->acc_ms2[2] = dev->Acc_ScaleFactor * XYZ_ACC_datas[2];

	return errors;


}
void BMI088_ReadGyro(BMI088_t *dev)
{
	HAL_StatusTypeDef Status;
    uint8_t GYRORawData[6];
    uint8_t errors = 0;

	int16_t XYZ_Gryo_datas[3];

	Status = BMI088_Read_Regs(dev,BMI088_GYR_DATA ,GYRORawData, 6, I2CGyro);
	errors += (Status != HAL_OK);

	XYZ_Gryo_datas[0] = ((int16_t) GYRORawData[0] | ((int16_t) GYRORawData[1] << 8));
	XYZ_Gryo_datas[1] = ((int16_t) GYRORawData[2] | ((int16_t) GYRORawData[3] << 8));
	XYZ_Gryo_datas[2] = ((int16_t) GYRORawData[4] | ((int16_t) GYRORawData[5] << 8));

	dev->gyr_degs[0] =  XYZ_Gryo_datas[0] * dev->Gyr_ScaleFactor;
	dev->gyr_degs[1] =  XYZ_Gryo_datas[1] * dev->Gyr_ScaleFactor;
	dev->gyr_degs[2] =  XYZ_Gryo_datas[2] * dev->Gyr_ScaleFactor;


}



void  BMI088_ReadAccGyro_DMA(BMI088_t *dev)
{
    BMI088_Read_Regs_DMA(dev, BMI088_ACC_DATA ,dev->DMA_ACCRawData,6, I2CAcc);
    BMI088_Read_Regs_DMA(dev, BMI088_GYR_DATA ,dev->DMA_GYRORawData,6, I2CAcc);

}

void  BMI088_ReadAccGyro_DMA_COMPLETE(BMI088_t *dev)
{

}

