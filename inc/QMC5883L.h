/*
 * QMC5883L.h
 *
 *  Created on: 4 sty 2024
 *      Author: Adam Iwachów
 */

#ifndef QMC5883L_H_
#define QMC5883L_H_


#include "stm32l4xx_hal.h"
#include <math.h>


#define Gauss_To_uTesla    			(1.0f / 0.01f)



#define QMC5883L_ADDRESS  		    (0x0D << 1)
#define QMC5883L_WHO_AM_I			0x0D
#define QMC5883L_ID					0xFF



#define QMC5883L_DATA_READ_X_LSB	0x00
#define QMC5883L_DATA_READ_X_MSB	0x01
#define QMC5883L_DATA_READ_Y_LSB	0x02
#define QMC5883L_DATA_READ_Y_MSB	0x03
#define QMC5883L_DATA_READ_Z_LSB	0x04
#define QMC5883L_DATA_READ_Z_MSB	0x05
#define QMC5883L_TEMP_READ_LSB		0x07
#define QMC5883L_TEMP_READ_MSB		0x08
#define QMC5883L_STATUS		        0x06
#define QMC5883L_CONFIG_1			0x09
#define QMC5883L_CONFIG_2			0x0A
#define QMC5883L_CONFIG_3			0x0B





typedef struct{

	I2C_HandleTypeDef *i2cHandle;

	float CalMagData[3];
	float Yaw;
	float Mag_ScaleFactor;

	// calibration

	// hard iron
	float HardIronMagneticOffset[3];

	// soft iron
    float SoftIronMagneticMapping[3][3];



	uint8_t address;


}QMC5883L_t;
uint8_t  QMC5883L_ReadMag(QMC5883L_t *dev, float ALPHA_MAG);
uint8_t QMC5883L_INIT(QMC5883L_t *dev, I2C_HandleTypeDef *i2cHandle, uint8_t address);

#endif /* QMC5883L_H_ */
