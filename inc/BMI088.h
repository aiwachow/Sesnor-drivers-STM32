/*
 * BMI088.h
 *
 *  Created on: Dec 11, 2023
 *      Author: Adam Iwachów
 *
 *      Basic BMI088 (6dof imu) driver, created just to make it work
 */

#ifndef BMI088_H_
#define BMI088_H_

#include "stm32l4xx_hal.h"
#include <math.h>
#define BMI088_ACC_I2C_ADDR (0x18 << 1)
#define BMI088_GYR_I2C_ADDR (0x68 << 1)
#define BMI088_I2C_TIMEOUT 100

/* Registers */
#define BMI088_ACC_CHIP_ID        0x00
#define BMI088_ACC_ERR_REG        0x02
#define BMI088_ACC_STATUS         0x03
#define BMI088_ACC_DATA           0x12
#define BMI088_TEMP_DATA          0x22
#define BMI088_ACC_CONF           0x40
#define BMI088_ACC_RANGE          0x41
#define BMI088_INT1_IO_CONF       0x53
#define BMI088_INT2_IO_CONF       0x54
#define BMI088_INT1_INT2_MAP_DATA 0x58
#define BMI088_ACC_SELF_TEST      0x6D
#define BMI088_ACC_PWR_CONF       0x7C
#define BMI088_ACC_PWR_CTRL       0x7D
#define BMI088_ACC_SOFTRESET      0x7E

#define BMI088_GYR_CHIP_ID           0x00
#define BMI088_GYR_DATA              0x02
#define BMI088_GYR_RANGE             0x0F
#define BMI088_GYR_BANDWIDTH         0x10
#define BMI088_GYR_LPM1              0x11
#define BMI088_GYR_SOFTRESET         0x14
#define BMI088_GYR_INT_CTRL          0x15
#define BMI088_GYR_INT3_INT4_IO_CONF 0x16
#define BMI088_GYR_INT3_INT4_IO_MAP  0x18



// Gyro scale
#define BMI088_GYRO_SCALE_2000DPS     0x00
#define BMI088_GYRO_SCALE_1000DPS     0x01
#define BMI088_GYRO_SCALE_500DPS      0x02
#define BMI088_GYRO_SCALE_250DPS      0x03
#define BMI088_GYRO_SCALE_125DPS      0x04


// Data rate
#define BMI088_GYRO_DATARATE_32kHz    0x01
#define BMI088_GYRO_DATARATE_16kHz    0x02
#define BMI088_GYRO_DATARATE_8kHz     0x03
#define BMI088_GYRO_DATARATE_4kHz     0x04
#define BMI088_GYRO_DATARATE_2kHz     0x05
#define BMI088_GYRO_DATARATE_1kHz     0x06  // Default
#define BMI088_GYRO_DATARATE_200Hz    0x07
#define BMI088_GYRO_DATARATE_100Hz    0x08
#define BMI088_GYRO_DATARATE_50Hz     0x09
#define BMI088_GYRO_DATARATE_25Hz     0x0A
#define BMI088_GYRO_DATARATE_12_5Hz   0x0B
#define BMI088_GYRO_DATARATE_500Hz    0x0F



// Full scale select for accelerometer
#define BMI088_ACCEL_FS_SEL_24G    0x03
#define BMI088_ACCEL_FS_SEL_12G    0x02
#define BMI088_ACCEL_FS_SEL_6G     0x01
#define BMI088_ACCEL_FS_SEL_3G     0x00

// Accelerometer Output Data Rate
#define BMI088_ACCEL_ODR_RESERVED  0x00
#define BMI088_ACCEL_ODR_32kHz     0x01
#define BMI088_ACCEL_ODR_16kHz     0x02
#define BMI088_ACCEL_ODR_8kHz      0x03
#define BMI088_ACCEL_ODR_4kHz      0x04
#define BMI088_ACCEL_ODR_2kHz      0x05
#define BMI088_ACCEL_ODR_1kHz      0x06  // Default
#define BMI088_ACCEL_ODR_200Hz     0x07
#define BMI088_ACCEL_ODR_100Hz     0x08
#define BMI088_ACCEL_ODR_50Hz      0x09
#define BMI088_ACCEL_ODR_25Hz      0x0A
#define BMI088_ACCEL_ODR_12_5Hz    0x0B
#define BMI088_ACCEL_ODR_6_25Hz    0x0C
#define BMI088_ACCEL_ODR_3_125Hz   0x0D
#define BMI088_ACCEL_ODR_1_5625Hz  0x0E
#define BMI088_ACCEL_ODR_500Hz     0x0F

#define I2CGyro 				   0x01
#define I2CAcc					   0x00

typedef struct {

	// handle for i2c
	I2C_HandleTypeDef *i2cHandle;

	// Acceleration and gyroscope data
	float acc_ms2[3];
	float gyr_degs[3];
	float temp_C;

	uint8_t GYROrange;
	uint8_t ACCrange;

	float Gyr_ScaleFactor;
    float Acc_ScaleFactor;


    float Temperature;


	uint8_t GYROaddress;
	uint8_t ACCaddress;

	// DMA BUFFER

	uint8_t DMA_ACCRawData[6];
    uint8_t DMA_GYRORawData[6];

} BMI088_t;
uint8_t BMI088_INIT(BMI088_t *dev, I2C_HandleTypeDef *i2cHandle);
uint8_t BMI088_ReadAcc(BMI088_t *dev);
void BMI088_ReadGyro(BMI088_t *dev);

void  BMI088_ReadAccGyro_DMA(BMI088_t *dev);
void  BMI088_ReadAccGyro_DMA_COMPLETE(BMI088_t *dev);

#endif /* BMI088_H_ */
