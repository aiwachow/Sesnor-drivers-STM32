/*
 * LPS25HB_I2C.h
 *
 *  Created on: Sep 22, 2023
 *      Author: Adam Iwachów
 *
 *      Basic LPS25HB (pressure sensor) driver, created just to make it work
 */

#ifndef LPS25HB_I2C_H_
#define LPS25HB_I2C_H_

#include "i2c.h"
#include "stm32l4xx_hal.h"

// macros
#define IS_FIFO_FULL(data)		(((data) & (0x1F)) == (0x1F))	// fifo stored full data

#define LPS25HB_I2C_ADDRESS 	(0x5D << 1) 			// 0x5D due to SDO which is pulled high

#define LPS25HB_REF_P_XL  			0x08 					//00000000 R/W
#define LPS25HB_REF_P_L  			0x09					// 00000000 R/W
#define LPS25HB_REF_P_H 			0x0A 					// 00000000 R/W

#define LPS25HB_WHO_AM_I  			0x0F 					// 10111101 Who am I register R

#define LPS25HB_RES_CONF 			0x10 					// 00001111 Resolution register R/W
#define LPS25HB_CTRL_REG1 			0x20					// 00000000 R/W

#define LPS25HB_CTRL_REG2 			0x21					// 00000000 R/W
#define LPS25HB_CTRL_REG3 			0x22					// 00000000 R/W
#define LPS25HB_CTRL_REG4 			0x23					//  00000000 R/W


#define LPS25HB_INTERRUPT_CFG 		0x24 					// 00000000 R/W
#define LPS25HB_INT_SOURCE  		0x25 					// 00000000, output R

#define LPS25HB_STATUS_REG  		0x27 					// 00000000, output Status register R

#define LPS25HB_PRESS_OUT_XL 		0x28 					// output R Pressure output
#define LPS25HB_PRESS_OUT_L  		0x29					// 29 output R
#define LPS25HB_PRESS_OUT_H  		0x2A 					// output R

#define LPS25HB_TEMP_OUT_L 			0x2B 					// output Temperature output R
#define LPS25HB_TEMP_OUT_H 			0x2C 					// output R

#define LPS25HB_FIFO_CTRL 			0x2E 					// 00000000 FIFO configure R/W
#define LPS25HB_FIFO_STATUS 		0x2F 					// 00100000, output R
#define LPS25HB_THS_P_L 			0x30 					// 00000000 Pressure threshold R/W
#define LPS25HB_THS_P_H 			0x31 					// 00000000 R/W

#define LPS25HB_RPDS_L 				0x39 					// 00000000 Pressure offset R/W
#define LPS25HB_RPDS_H 				0x3A 					// 00000000 registers R/W





typedef struct{

	I2C_HandleTypeDef *i2cHandle;

	uint8_t address;
	uint8_t MEAN_flag;

	float pressure_hPa;
	float Start_press_hPa;


	float temp_cel;
	float altitude;

	// for DMA
	volatile uint8_t RawTemp[2];
	volatile uint8_t RawPress[3];

}LPS25HB_t;


HAL_StatusTypeDef LPS25HB_INIT(LPS25HB_t *dev, I2C_HandleTypeDef *i2cHandle, uint8_t address);

HAL_StatusTypeDef LPS25HB_READ_TEMP(LPS25HB_t *dev);

HAL_StatusTypeDef LPS25HB_READ_PRESSURE(LPS25HB_t *dev);

HAL_StatusTypeDef LPS25HB_PRESSURE_OFFSET(LPS25HB_t *dev, int16_t offset);

// DMA
uint8_t LPS25HB_READ_PRESS_DMA(LPS25HB_t *dev);
 void LPS25HB_READ_PRESS_DMA_COMPLETE(LPS25HB_t *dev);

// IT
uint8_t LPS25HB_READ_PRESS_IT(LPS25HB_t *dev);
void LPS25HB_READ_PRESS_IT_COMPLETE(LPS25HB_t *dev);


 // FIFO
float LPS25HB_FIFO_MEAN(LPS25HB_t *dev);
 uint8_t  LPS25HB_FIFO_STATUS_CHECK(LPS25HB_t *dev);



void LPS25HB_ESTIMATE_ALTITUDE(LPS25HB_t *dev);

HAL_StatusTypeDef LPS25HB_REBOOT(LPS25HB_t *dev);

HAL_StatusTypeDef LPS25HB_ENABLE_INT(LPS25HB_t *dev);







void SWAP_TO_POLLING_MODE(void);

#endif /* LPS25HB_I2C_H_ */
