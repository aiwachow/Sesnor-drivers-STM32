
/*
 * ICM_42688_P.c
 *
 *  Created on: Dec 10, 2023
 *      Author: Adam Iwachów
 *
 *      Work in progress, driver do not work :(
 */
#define I2C_MAX_TIME 100
#include <ICM_42688_P.h>
/*
 *  I2C COMMUNICATION POLLING FUNCTIONS
 */
 static HAL_StatusTypeDef  ICM_42688_Read_Reg(ICM_42688_t *dev, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, 1, I2C_MAX_TIME);
}

 static HAL_StatusTypeDef  ICM_42688_Read_Regs(ICM_42688_t *dev, uint8_t reg, uint8_t *data, uint8_t length)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, length, I2C_MAX_TIME);
}

 static HAL_StatusTypeDef  ICM_42688_Write_Reg(ICM_42688_t *dev, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Write(dev->i2cHandle, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, 1, I2C_MAX_TIME);
}






uint8_t ICM_42688_INIT(ICM_42688_t *dev, I2C_HandleTypeDef *i2cHandle, uint8_t address)
 {
	 HAL_StatusTypeDef Status;
	 uint8_t errors = 0;
	 uint8_t Data_write = 0;
	 uint8_t Data_read = 0;

		dev->i2cHandle		= i2cHandle;
		dev->address 		= address;

		dev->GYROrange = ICM_GYRO_SCALE_62_5DPS;
		dev->ACCrange = ICM_ACCEL_FS_SEL_2G;

		dev->Gyr_ScaleFactor= 0.0f;
		dev->Acc_ScaleFactor= 0.0f;

		dev->Temperature = 0.0f;



	/* Check twice if the device is alive */

	if(HAL_I2C_IsDeviceReady(dev->i2cHandle, dev->address, 2, I2C_MAX_TIME) == HAL_OK){

	 Status = ICM_42688_Read_Reg(dev,  ICM_WHO_AM_I, &Data_read);
	 errors += (Status != HAL_OK);

	}

	 if(Data_read != 0x47 ){
		 return 0xFF;
	 }

	 Data_read = 0;

	 /* if ok lets do initialization */


	 // Place acc and gyro in low noise mode
	 Data_write = 0x0f;
	 Status = ICM_42688_Write_Reg(dev, ICM_PWR_MGMT0 ,&Data_write);
	 	HAL_Delay(100);

	 ICM_42688_SetFullScale_ACC_GYRO_Ranges(dev,ICM_ACCEL_FS_SEL_2G,ICM_ACCEL_ODR_12_5Hz, ICM_GYRO_SCALE_62_5DPS, ICM_GYRO_DATARATE_12_5Hz);
	 ICM_42688_ReadAccGyroTemp(dev);

		return errors;
 }



void ICM_42688_SetFullScale_ACC_GYRO_Ranges(ICM_42688_t *dev, uint8_t AccRange,uint8_t AccDataRate, uint8_t GyroRange,uint8_t GyroDataRate)
{
	HAL_StatusTypeDef Status;
	 uint8_t Data_write = 0;
	 uint8_t Data_read = 0;

	dev->ACCrange = AccRange;
	dev->GYROrange = GyroRange;

	Status = ICM_42688_Read_Reg(dev, ICM_GYRO_CONFIG0 ,&Data_read);
	Data_read &= 0x10; Data_read |= (GyroRange << 4) | (GyroDataRate);
	Data_write = Data_read;
	while((Status = ICM_42688_Write_Reg(dev, ICM_GYRO_CONFIG0 ,&Data_write)) != HAL_OK);


	Status = ICM_42688_Read_Reg(dev, ICM_ACCEL_CONFIG0 , &Data_read);
	Data_read &= 0x10; Data_read |= (AccRange << 4) | (AccDataRate);
	Data_write = Data_read;
	while((Status = ICM_42688_Write_Reg(dev,  ICM_ACCEL_CONFIG0 ,&Data_write)) != HAL_OK);


	float resolution = pow(2.0f, 15.0f);


	switch(dev->ACCrange){
	case ICM_ACCEL_FS_SEL_2G:
		dev->Acc_ScaleFactor = (2.0f /  resolution);
	break;
	case ICM_ACCEL_FS_SEL_4G:
		dev->Acc_ScaleFactor = (4.0f /  resolution);
	break;
	case ICM_ACCEL_FS_SEL_8G:
		dev->Acc_ScaleFactor = (8.0f /  resolution);
	break;
	case ICM_ACCEL_FS_SEL_16G:
		dev->Acc_ScaleFactor = (16.0f /  resolution);
	break;
	default:
			break;
	}


		switch(dev->GYROrange){
		case ICM_GYRO_SCALE_2000DPS:
			dev->Gyr_ScaleFactor = (2000.0f /  resolution);
		break;
		case ICM_GYRO_SCALE_1000DPS:
			dev->Gyr_ScaleFactor = (1000.0f /  resolution);
		break;
		case ICM_GYRO_SCALE_500DPS:
			dev->Gyr_ScaleFactor = (500.0f /  resolution);
		break;
		case ICM_GYRO_SCALE_250DPS:
			dev->Gyr_ScaleFactor = (250.0f /  resolution);
		break;
		case ICM_GYRO_SCALE_125DPS:
			dev->Gyr_ScaleFactor = (125.0f /  resolution);
		break;
		case ICM_GYRO_SCALE_62_5DPS:
			dev->Gyr_ScaleFactor = (62.5f /  resolution);
		break;
		case ICM_GYRO_SCALE_31_25DPS:
			dev->Gyr_ScaleFactor = (31.25f /  resolution);
		break;
		case ICM_GYRO_SCALE_15_625DPS:
			dev->Gyr_ScaleFactor = (15.625f /  resolution);
		break;
		default:
			break;
		}


	}


void ICM_42688_ReadAccGyroTemp(ICM_42688_t *dev)
{
	HAL_StatusTypeDef Status;
	uint8_t RawData[14];


	Status = ICM_42688_Read_Regs(dev, ICM_TEMP_DATA1 ,RawData, 14);

	int16_t XYZ_ACC_datas[3];
	int16_t XYZ_Gryo_datas[3];

	dev->Temperature= ((int16_t) RawData[0] << 8 | ((int16_t) RawData[1]));

	XYZ_ACC_datas[0] = ((int16_t) RawData[2] << 8 | ((int16_t) RawData[3]));
	XYZ_ACC_datas[1] = ((int16_t) RawData[4] << 8 | ((int16_t) RawData[5]));
	XYZ_ACC_datas[2] = ((int16_t) RawData[6] << 8 | ((int16_t) RawData[7]));

	XYZ_Gryo_datas[0] = ((int16_t) RawData[8] | ((int16_t) RawData[9] << 8));
	XYZ_Gryo_datas[1] = ((int16_t) RawData[10] | ((int16_t) RawData[11] << 8));
	XYZ_Gryo_datas[2] = ((int16_t) RawData[12] | ((int16_t) RawData[13] << 8));

	dev->acc_ms2[0] = 9.81f * XYZ_ACC_datas[0] * dev->Acc_ScaleFactor;
	dev->acc_ms2[1] = 9.81f * XYZ_ACC_datas[1] * dev->Acc_ScaleFactor;
	dev->acc_ms2[2] = 9.81f * XYZ_ACC_datas[2] * dev->Acc_ScaleFactor;

	dev->gyr_degs[0] =  XYZ_Gryo_datas[0] * dev->Gyr_ScaleFactor;
	dev->gyr_degs[1] =  XYZ_Gryo_datas[1] * dev->Gyr_ScaleFactor;
	dev->gyr_degs[2] =  XYZ_Gryo_datas[2] * dev->Gyr_ScaleFactor;

}
