/*
 * LPS22HB.h
 *
 *  Created on: Dec 9, 2023
 *      Author: Adam Iwachów
 *
 *      Basic LPS22HB (pressure sensor) driver, created just to make it work
 */

#ifndef LPS22HB_H_
#define LPS22HB_H_

#include "i2c.h"
#include "stm32l4xx_hal.h"
//#include "I2CBus_Handler.h"


// macros
#define IS_FIFO_FULL(data)		(((data) & (0x1F)) == (0x1F))	// fifo stored full data
// REGISTERS


#define LPS22HB_I2C_ADDRESS_I	 (0x5C << 1) 			// 0x5D due to SDO which is pulled low
#define  LPS22HB_WHO_AM_I            	 0x0F   // Who am I (Default: 10110001)


#define  LPS22HB_THS_P_L             	 0x0C   // Pressure threshold registers (Default: 00000000)
#define  LPS22HB_THS_P_H              	 0x0D   // Pressure threshold registers (Default: 00000000)


#define LPS22HB_CTRL_REG1            	 0x10   // Control registers (Default: 00000000)
#define LPS22HB_CTRL_REG2           	 0x11   // Control registers (Default: 00010000)
#define LPS22HB_CTRL_REG3           	 0x12   // Control registers (Default: 00000000)


#define  LPS22HB_REF_P_XL            	 0x15   // Reference pressure registers (Default: 00000000)
#define  LPS22HB_REF_P_L             	 0x16   // Reference pressure registers (Default: 00000000)
#define  LPS22HB_REF_P_H             	 0x17   // Reference pressure registers (Default: 00000000)

#define  LPS22HB_RPDS_L              	 0x18   // Pressure offset registers (Default: 00000000)
#define  LPS22HB_RPDS_H              	 0x19   // Pressure offset registers (Default: 00000000)

#define  LPS22HB_RES_CONF            	 0x1A   // Resolution register (Default: 00000000)


#define  LPS22HB_INTERRUPT_CFG        	 0x0B   // Interrupt register (Default: 00000000)
#define  LPS22HB_INT_SOURCE           	 0x25   // Interrupt register

#define  LPS22HB_FIFO_CTRL           	 0x14   // FIFO configuration register (Default: 00000000)
#define  LPS22HB_FIFO_STATUS         	 0x26   // FIFO status register
#define  LPS22HB_STATUS              	 0x27   // Status register

#define  LPS22HB_PRESS_OUT_XL       	 0x28   // Pressure output registers
#define  LPS22HB_PRESS_OUT_L          	 0x29   // Pressure output registers
#define  LPS22HB_PRESS_OUT_H          	 0x2A   // Pressure output registers

#define  LPS22HB_TEMP_OUT_L           	 0x2B   // Temperature output registers
#define  LPS22HB_TEMP_OUT_H           	 0x2C   // Temperature output registers

#define  LPS22HB_RESERVED_2D_32       	 0x2D   // Reserved
#define  LPS22HB_LPFP_RES            	 0x33   // Filter reset register (Default: Unknown)





typedef struct{

	I2C_HandleTypeDef *i2cHandle;

	uint8_t address;

	float pressure_hPa;
	float temp_cel;
	float altitude;

	// for DMA
	volatile uint8_t RawTemp[2];
	volatile uint8_t RawPress[3];

}LPS22HB_t;


uint8_t LPS22HB_INIT(LPS22HB_t *dev, I2C_HandleTypeDef *i2cHandle, uint8_t address);
HAL_StatusTypeDef LPS22HB_READ_TEMP(LPS22HB_t *dev);
HAL_StatusTypeDef LPS22HB_READ_PRESSURE(LPS22HB_t *dev);
uint8_t LPS22HB_PRESSURE_READY(LPS22HB_t *dev);

void LPS22HB_SET_CALIB(LPS22HB_t *dev, uint16_t value);


#endif /* LPS22HB_H_ */
