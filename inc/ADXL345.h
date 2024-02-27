/*
 * ADXL345.h
 *
 *  Created on: Aug 9, 2023
 *      Author: Adam
 */

#ifndef INC_ADXL345_H_
#define INC_ADXL345_H_

#include "stm32l4xx_hal.h"

typedef enum{ OFF =  0, ON = 1} ONOFF;

#define ADXL345_ADRESS    	(0x1D << 1) // SDO = 0 => 0x1D, SDO = 1 => 0x53 (p. 10)
#define ADXL345_DEVID		 0xE5

/* Private df configuration */
// BW_RATE
#define LOW_POWER_ON 	0x10
#define LOW_POWER_Off 	0x00



// RATES
#define RATE_3200Hz		0x0F
#define RATE_1600Hz		0x0E
#define RATE_800Hz		0x0D
#define RATE_400Hz		0x0C
#define RATE_200Hz		0x0B
#define RATE_100Hz		0x0A
#define RATE_50Hz		0x09
#define RATE_25Hz		0x08
#define RATE_12_5Hz		0x07
#define RATE_6_25Hz		0x06
#define RATE_1_55Hz		0x04
#define RATE_0_78Hz		0x03



//POWERCTRL
#define AUTO_SLEEP_ON 	1
#define AUTO_SLEEP_Off 	0
#define LINK_ON 		1
#define LINK_Off 		0


// DATA FORMAT
#define RANGE_2g		0x00
#define RANGE_4g		0x01
#define RANGE_8g		0x02
#define RANGE_16g		0x03



// REGISTER MAP

#define REG_DEVID 				0x00  // Devide ID, R, RST - 11100101

#define REG_THRESH_TAP			0x1D // R/W 		   RST - 0x00
#define REG_DUR					0x21 // Tap threshold, RST - 0x00
#define REG_Latent				0x22 // Tap Latency    RST - 0x00
#define REG_Window				0x23 // Tap window     RST - 0x00
#define REG_THRESH_ACT 			0x24 // Thresh activity RST - 0x00
#define REG_THRESH_INACT		0x25 // Inactivity threshold RST - 0x00
#define REG_TIME_INACT 			0x26 // time for setting inact.
#define REG_ACT_INACT_CTL		0x27 // Axis enable control for activity and inactivity detection.
#define REG_THRESH_FF			0x28 // Free-fall threshold
#define REG_TIME_FF				0x29 // Free-fall time
#define REG_TAP_AXES			0x2A // Axis control for tap/double tap
#define REG_ACT_TAP_STATUS		0x2B // Source of tap/double tap

/* POWER */
#define REG_BW_RATE 			0x2C // Data rate and power mode control
#define REG_POWER_CTL			0x2D // Power-saving features control

/* INTERRUPTS
 *
 * INT PINS ARE WORKING IN PUSH PULL MODE
 * BE CAREFULL FOR SHORT CIRCUITS
 * */


#define REG_INT_ENABLE			0x2E // Interrupt enable control
#define REG_INT_MAP 			0x2F // Interrupt mapping control
#define REG_INT_SOURCE			0x30 // Source of interrupts




/* FIFO */
#define REG_FIFO_CTL			0x38
#define REG_FIFO_STATUS			0x39

/* MEASURMENT */
 // axis offsets
#define REG_OFSX 				0x1E
#define REG_OFSY 				0x1F
#define REG_OFSZ 				0x20

// Data
#define REG_DATA_FORMAT 		0x31 // data format control
#define REG_DATAX0 				0x32
#define REG_DATAX1 				0x33
#define REG_DATAY0 				0x34
#define REG_DATAY1 				0x35
#define REG_DATAZ0 				0x36
#define REG_DATAZ1 				0x37

typedef struct {
	// acceleration data m/s^2
	float acc_ms2[3];


	// init configuration
	uint8_t INIT_ON;
	uint8_t LOW_POW_MOD;
	uint8_t RATE;
	uint8_t LINK_BIT;
	uint8_t AUTO_SLEEP;
	uint8_t Range;

	// handle for i2c
	I2C_HandleTypeDef *i2cHandle;

}ADXL345_t;

// OVERRUN,WATERMARK,DATA_READY are always set
typedef enum{

	OVERRUN,
	WATERMARK,
	FREE_FALL,
	INACTIVITY,
	ACTIVITY,
	DOUBLE_TAP,
	SINGLE_TAP,
	DATA_READY,
	NONE_INT,

}IntSource_t;

typedef enum{

	TAP_Z,
	TAP_Y,
	TAP_X,
	ASleep,
	ACT_Z,
	ACT_Y,
	ACT_X,
	NONE_STAT,

}ActTapStatus_t;


uint8_t ADXL_INIT(ADXL345_t *dev, I2C_HandleTypeDef *i2cHandle);

HAL_StatusTypeDef ADXL345_readACC(ADXL345_t *dev);

uint8_t ADXL345_SetOFS(ADXL345_t *dev, float OFSX, float OFSY, float OFSZ);

uint8_t ADXL345_EnableINTERRUPTS(ADXL345_t *dev, ONOFF Data_Ready, ONOFF Single_Tap, ONOFF Double_Tap,
		ONOFF Activity, ONOFF Inactivity, ONOFF FreeFall, ONOFF Watermark, ONOFF Overrun);

uint8_t ADXL345_Enable_ACT_AXES(ADXL345_t *dev, uint8_t X_ONOFF, uint8_t Y_ONOFF, uint8_t Z_ONOFF);

uint8_t ADXL345_Enable_INACT_AXES(ADXL345_t *dev, uint8_t X_ONOFF, uint8_t Y_ONOFF, uint8_t Z_ONOFF);

uint8_t ADXL345_SET_ACT_PARAMS(ADXL345_t *dev, float Acc_INACT, float Acc_ACT, uint8_t Time);

IntSource_t ADXL_GetSourceOfINT(ADXL345_t *dev);

ActTapStatus_t ADXL_GetAxisSource(ADXL345_t *dev);

const char *getAxis(ADXL345_t *dev);

uint8_t ADXL345_ENABLE_SINGLE_TAP(ADXL345_t *dev, ONOFF onoff,  uint32_t DUR_time, float Thresh_Tap);


#endif /* INC_ADXL345_H_ */
