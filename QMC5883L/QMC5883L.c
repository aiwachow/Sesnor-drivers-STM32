/*
 * QMC5883L.c
 *
 *  Created on: 4 sty 2024
 *      Author: Adam Iwachów
 */

#include "QMC5883L.h"
#define I2C_MAX_TIME 100

static float Rad_to_Deg = 57.295f;
/*
 *  I2C COMMUNICATION POLLING FUNCTIONS
 */
 static HAL_StatusTypeDef  QMC5883L_Read_Reg(QMC5883L_t *dev, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, 1, I2C_MAX_TIME);
}

 static HAL_StatusTypeDef  QMC5883L_Read_Regs(QMC5883L_t *dev, uint8_t reg, uint8_t *data, uint8_t length)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, length, I2C_MAX_TIME);
}

 static HAL_StatusTypeDef  QMC5883L_Write_Reg(QMC5883L_t *dev, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Write(dev->i2cHandle, dev->address, reg, I2C_MEMADD_SIZE_8BIT, data, 1, I2C_MAX_TIME);
}

uint8_t QMC5883L_INIT(QMC5883L_t *dev, I2C_HandleTypeDef *i2cHandle, uint8_t address)
 {
	 HAL_StatusTypeDef Status;
	 uint8_t errors = 0;
	 uint8_t Data_write = 0;
	 uint8_t Data_read = 0;



		dev->i2cHandle		= i2cHandle;
		dev->address 		= address;
		dev->CalMagData[0]  = 0.0f;


		dev->Mag_ScaleFactor = 0.0f ;
		dev->Yaw = 0.0f ;


		dev->HardIronMagneticOffset[0] = -5.1982f;
		dev->HardIronMagneticOffset[1] = 19.9262f;
		dev->HardIronMagneticOffset[2] = 0.7850f;

		dev->SoftIronMagneticMapping[0][0] =  1.0106f; dev->SoftIronMagneticMapping[0][1] = -0.0293f; dev->SoftIronMagneticMapping[0][2] =  0.0103f;
		dev->SoftIronMagneticMapping[1][0] = -0.0293f; dev->SoftIronMagneticMapping[1][1] =  0.9453f; dev->SoftIronMagneticMapping[1][2] = -0.00516f;
		dev->SoftIronMagneticMapping[2][0] =  0.0103f; dev->SoftIronMagneticMapping[2][1] = -0.0051f; dev->SoftIronMagneticMapping[2][2] =  1.0478f;



	/* Check twice if the device is alive */

	if(HAL_I2C_IsDeviceReady(dev->i2cHandle, dev->address, 2, I2C_MAX_TIME) == HAL_OK){

	 Status = QMC5883L_Read_Reg(dev,  QMC5883L_WHO_AM_I, &Data_read);
	 errors += (Status != HAL_OK);

	}

	 if(Data_read != QMC5883L_ID){
		 return 0xFF;
	 }

	 /* if ok lets do initialization	  */


	 // Data rate = 100hz, scale 2G, oversampling 256
	 Data_write = 0x49;
	 Status = QMC5883L_Write_Reg(dev,  QMC5883L_CONFIG_1, &Data_write);
	 errors += (Status != HAL_OK);


	 Data_write = 0x01; // datasheet p. 18
	 Status = QMC5883L_Write_Reg(dev,  QMC5883L_CONFIG_3, &Data_write);
	 errors += (Status != HAL_OK);

	 dev->Mag_ScaleFactor = 2.0f / 32768.0f;



		return errors;
 }



uint8_t  QMC5883L_ReadMag(QMC5883L_t *dev, float ALPHA_MAG)
{
	HAL_StatusTypeDef Status;
    uint8_t MagRawData[6];
    uint8_t errors = 0;

    int16_t XYZ_Mag_datas[3]       = {0};
    float   PreScale_Mag_Data[3]   = {0.0f};
    float   LP_PreScale_MagData[3]      = {0.0f};
    float   HARD_LP_MagData[3] = {0.0f};

	Status = QMC5883L_Read_Regs(dev, QMC5883L_DATA_READ_X_LSB ,MagRawData,6);
	errors += (Status != HAL_OK);

	// COMBINE DATA FROM REGISTERS
	XYZ_Mag_datas[0] = ((int16_t) MagRawData[1] << 8 | ((int16_t) MagRawData[0]));
	XYZ_Mag_datas[1] = ((int16_t) MagRawData[3] << 8 | ((int16_t) MagRawData[2]));
	XYZ_Mag_datas[2] = ((int16_t) MagRawData[5] << 8 | ((int16_t) MagRawData[4]));

	// SCALE RAW DATA, CONVERT TO MICRO TESLA
	PreScale_Mag_Data[0] = dev->Mag_ScaleFactor * XYZ_Mag_datas[0] * Gauss_To_uTesla;
	PreScale_Mag_Data[1] = dev->Mag_ScaleFactor * XYZ_Mag_datas[1] * Gauss_To_uTesla;
	PreScale_Mag_Data[2] = dev->Mag_ScaleFactor * XYZ_Mag_datas[2] * Gauss_To_uTesla;

	// LOW PASS
	LP_PreScale_MagData[0] = ALPHA_MAG * PreScale_Mag_Data[0] + (1.0f - ALPHA_MAG) *  LP_PreScale_MagData[0];
	LP_PreScale_MagData[1] = ALPHA_MAG * PreScale_Mag_Data[1] + (1.0f - ALPHA_MAG) *  LP_PreScale_MagData[1];
	LP_PreScale_MagData[2] = ALPHA_MAG * PreScale_Mag_Data[2] + (1.0f - ALPHA_MAG) *  LP_PreScale_MagData[2];

    // ELIMINATE HARD IRON EFFECT
	HARD_LP_MagData[0] = (LP_PreScale_MagData[0] - dev->HardIronMagneticOffset[0]);
	HARD_LP_MagData[1] = (LP_PreScale_MagData[1] - dev->HardIronMagneticOffset[1]);
	HARD_LP_MagData[2] = (LP_PreScale_MagData[2] - dev->HardIronMagneticOffset[2]);

	// ELIMIATE SOFT IRON EFFECT
	for(uint8_t i = 0; i < 3; i++)
	{
		dev->CalMagData[i] =   (dev->SoftIronMagneticMapping[i][0] * HARD_LP_MagData[0])
		                     + (dev->SoftIronMagneticMapping[i][1] * HARD_LP_MagData[1])
		                     + (dev->SoftIronMagneticMapping[i][2] * HARD_LP_MagData[2]);
	}



	return errors;


}



