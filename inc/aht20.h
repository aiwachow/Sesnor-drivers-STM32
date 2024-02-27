/*
 * aht20.h
 *
 *  Created on: Aug 22, 2023
 *      Author: Adam Iwachów
 */

#ifndef INC_AHT20_H_
#define INC_AHT20_H_

#define AHT20_I2C_ADDRESS 			(0x38 << 1)

#define AHT20_REG_INIT 				0xBE
#define AHT20_REG_TRIG_MEASURE 		0xAC
#define AHT20_SOFT_RST 				0xBA

#define AHT20_STATUS 				0x71



typedef struct{

	I2C_HandleTypeDef *i2cHandle;

	float temp_C;
	float humidity_pct;

}AHT20_t;


uint8_t AHT20_READ_TEMP_HUMIDITY(AHT20_t *dev);
uint8_t AHT20_INIT(AHT20_t *dev, I2C_HandleTypeDef *i2cHandle);
#endif /* INC_AHT20_H_ */
