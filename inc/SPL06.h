/*
 * SPL06.h
 *
 *  Created on: Dec 9, 2023
 *      Author: Adam Iwachów
 *
 *  Basic SPL06-001 (pressure sensor) driver, created just to make it work
 *  Note* max sample rate is 8Hz, no driver for higher freqs
 */

#ifndef SPL06_H_
#define SPL06_H_

#include "i2c.h"
#include "stm32l4xx_hal.h"


#define SPL06_I2C_ADDRESS_I		(0x77 << 1) 			// 0x5D due to SDO which is pulled low
#define  SPL06_WHO_AM_I            0x0D  // Who am I (Default: 00001010)

#define  SPL06_PSR_B2              0x00   // PSR[23:16] (r) 00h
#define  SPL06_PSR_B1              0x01   // PSR[15:8](r) 00h
#define  SPL06_PSR_B0              0x02   // PSR[7:0](r) 00h

#define  SPL06_TMP_B2              0x03   // TMP[23:16] (r) 00h
#define  SPL06_TMP_B1              0x04   // TMP[15:8] (r) 00h
#define  SPL06_TMP_B0              0x05   // TMP[7:0] (r) 00h

#define  SPL06_PRS_CFG             0x06   // PM_RATE [2:0] (rw) PM_PRC [3:0] (rw) 00h
#define  SPL06_TMP_CFG             0x07   // TMP_EXT (rw) TMP_RATE [2:0] (rw) TM_PRC [2:0] (rw) 00h
#define  SPL06_MEAS_CFG            0x08   // COEF_RDY (r) SENSOR_RDY (r) TMP_RDY (r) PRS_RDY (r) MEAS_CRTL [2:0] (rw) 00h
#define  SPL06_CFG_REG             0x09   // INT_HL (rw) INT_SEL [2:0] (rw) TMP_SHIFT_EN (rw) PRS_SHIFT_EN (rw) FIFO_EN (rw) 00h

#define  SPL06_INT_STS             0x0A   // INT_FIFO_FULL (r) INT_TMP (r) INT_PRS (r) 00h
#define  SPL06_FIFO_STS            0x0B   // FIFO_FULL (r) FIFO_EMPTY (r) 00h
#define  SPL06_RESET               0x0C   // FIFO_FLUSH (w) SOFT_RST [3:0] (w) 00h


#define COEF_START          0x10   // Start of COEF registers

// Coefficient Address 		Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
#define C0                  0x10   // c0 [11:4]
#define C0_C1               0x11   // c0 [3:0] c1 [11:8]
#define C1                  0x12   // c1[7:0]
#define C00                 0x13   // c00 [19:12]
#define C00_2               0x14   // c00 [11:4]
#define C00_C10             0x15   // c00 [3:0] c10 [19:16]
#define C10                 0x16   // c10 [15:8]
#define C10_2               0x17   // c10 [7:0]
#define C01                 0x18   // c01 [15:8]
#define C01_2               0x19   // c01 [7:0]
#define C11                 0x1A   // c11 [15:8]
#define C11_2               0x1B   // c11 [7:0]
#define C20                 0x1C   // c20 [15:8]
#define C20_2               0x1D   // c20 [7:0]
#define C21                 0x1E   // c21 [15:8]
#define C21_2               0x1F   // c21 [7:0]
#define C30                 0x20   // c30 [15:8]
#define C30_2               0x21   // c30 [7:0]

#define COEF_END            0x21   // End of COEF registers



typedef struct{

	I2C_HandleTypeDef *i2cHandle;

	uint8_t address;

	float pressure_hPa;
	float temp_cel;
	float altitude;

	float ALPHA;

	// for DMA
	volatile uint8_t RawTemp[2];
	volatile uint8_t RawPress[3];

	 int16_t c0;
	 int16_t c1;
	    int32_t c00;
	    int32_t c10;
	    int16_t c01;
	    int16_t c11;
	    int16_t c20;
	    int16_t c21;
	    int16_t c30;

}SPL06_t;

typedef struct {
    int16_t c0;
    int16_t c1;
    int32_t c00;
    int32_t c10;
    int16_t c01;
    int16_t c11;
    int16_t c20;
    int16_t c21;
    int16_t c30;
} Coefficients_t;

uint8_t SPL06_INIT(SPL06_t *dev, I2C_HandleTypeDef *i2cHandle, uint8_t address);
HAL_StatusTypeDef SPL06_READ_TEMP_PRESS(SPL06_t *dev);
uint8_t SPL06_PRESS_AND_TEMP_READY(SPL06_t *dev);

#endif /* SPL06_H_ */
