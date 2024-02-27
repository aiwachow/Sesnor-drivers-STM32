/*
 * ADXL345.c
 *
 *  Created on: Aug 9, 2023
 *      Author: Adam
 */

#include "ADXL345.h"
#include "math.h"


 static HAL_StatusTypeDef ADXL345_Read_Reg(ADXL345_t *dev, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, ADXL345_ADRESS, reg, I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY);
}

 static HAL_StatusTypeDef ADXL345_Read_Regs(ADXL345_t *dev, uint8_t reg, uint8_t *data, uint8_t length)
{
	return HAL_I2C_Mem_Read(dev->i2cHandle, ADXL345_ADRESS, reg, I2C_MEMADD_SIZE_8BIT, data, length, HAL_MAX_DELAY);
}


 static HAL_StatusTypeDef ADXL345_Write_Reg(ADXL345_t *dev, uint8_t reg, uint8_t *data)
{
	return HAL_I2C_Mem_Write(dev->i2cHandle, ADXL345_ADRESS, reg, I2C_MEMADD_SIZE_8BIT, data, 1, HAL_MAX_DELAY);
}


uint8_t ADXL_INIT(ADXL345_t *dev, I2C_HandleTypeDef *i2cHandle)
{


	if(dev->INIT_ON != 0x01)
		return 0xFF;

	dev->i2cHandle = i2cHandle ;

	dev ->acc_ms2[0] = 0.0f;
	dev ->acc_ms2[1] = 0.0f;
	dev ->acc_ms2[2] = 0.0f;


	HAL_StatusTypeDef status;
	uint8_t Data   = 0;
	uint8_t errors = 0;

	status = ADXL345_Read_Reg(dev, REG_DEVID, &Data);
	errors += (status != HAL_OK);

	if(Data != ADXL345_DEVID)
		return 0xFF;

	// RESETS

	Data   = 0x0A;
	ADXL345_Write_Reg(dev,REG_BW_RATE, &Data);

	Data   = 0x00;
	status = ADXL345_Write_Reg(dev,REG_INT_ENABLE, &Data);
	status = ADXL345_Write_Reg(dev,REG_INT_MAP, &Data);
	status = ADXL345_Write_Reg(dev,REG_THRESH_ACT, &Data);
	status = ADXL345_Write_Reg(dev,REG_THRESH_INACT, &Data);
	status = ADXL345_Write_Reg(dev,REG_ACT_INACT_CTL, &Data);
	status = ADXL345_Write_Reg(dev,REG_TAP_AXES, &Data);
	status = ADXL345_Write_Reg(dev,REG_POWER_CTL, &Data);
	status = ADXL345_Write_Reg(dev, REG_DATA_FORMAT, &Data);
	status = ADXL345_Write_Reg(dev, REG_ACT_INACT_CTL , &Data);
	// BASIC INITIALIZATION

	//RATE 25HZ and Power mode off
	Data = RATE_6_25Hz;
	status = ADXL345_Write_Reg(dev,REG_BW_RATE,&Data);
	errors += (status != HAL_OK);
	status = ADXL345_Read_Reg(dev,REG_BW_RATE , &Data);



	// Data format by default all options expect range are cleared

	Data = 0x00; // for explanation datasheet p. 27
	status = ADXL345_Write_Reg(dev, REG_DATA_FORMAT, &Data);
	errors += (status != HAL_OK);

	// Power mode by defaul sleep, wakeup are cleard measure is set
	// The wakeup mode means that when device turn into sleep mode measurment freq drops to  0 - 8 hz

	Data   = 0x0B;
	status = ADXL345_Write_Reg(dev,REG_POWER_CTL, &Data);

	ADXL345_readACC(dev); // pre-measurment to avoid gross error


	return errors;
}




uint8_t ADXL345_SetRate(ADXL345_t *dev, uint8_t Rate)
{
		uint8_t Data = 0, errors= 0;
		HAL_StatusTypeDef status;

		status = ADXL345_Read_Reg(dev,REG_BW_RATE , &Data);
		errors += (status != HAL_OK);

		Data &= 0xF0; Data |= Rate;
		status = ADXL345_Write_Reg(dev,REG_BW_RATE, &Data);
		errors += (status != HAL_OK);

		return errors;

}


uint8_t ADXL345_SetRange(ADXL345_t *dev, uint8_t Range)
{
		uint8_t Data = 0, errors= 0;
		HAL_StatusTypeDef status;

		status = ADXL345_Read_Reg(dev,REG_BW_RATE , &Data);
		errors += (status != HAL_OK);

		Data &= 0xFC; Data |= Range;
		status = ADXL345_Write_Reg(dev,REG_BW_RATE, &Data);
		errors += (status != HAL_OK);

		return errors;
}

uint8_t ADXL345_SetAutoSleep_Sleep_SleepHz(ADXL345_t *dev, uint8_t ONOFF_AS, uint8_t ONOFF_S, uint8_t freq)
{
		uint8_t Data = 0, errors= 0;
		HAL_StatusTypeDef status;

		status = ADXL345_Read_Reg(dev,REG_POWER_CTL, &Data);
		errors += (status != HAL_OK);

		// put into standby mode to ensure properly biased
		Data &= 0xF7;
		status = ADXL345_Write_Reg(dev,REG_POWER_CTL , &Data);
		errors += (status != HAL_OK);

		// AutoSleep Sleep fq

		Data &= 0xE0; Data |= (ONOFF_AS << 4) | (ONOFF_S << 2) | (freq);
		status = ADXL345_Write_Reg(dev,REG_BW_RATE, &Data);
		errors += (status != HAL_OK);

		// Measure on
		Data |= (1 << 3);
		status = ADXL345_Write_Reg(dev,REG_POWER_CTL , &Data);
		errors += (status != HAL_OK);


		return errors;
}


HAL_StatusTypeDef ADXL345_readACC(ADXL345_t *dev)
{
	float scale_factor = 0.0f;
	uint8_t Datareg[6];

	HAL_StatusTypeDef status;


	status = ADXL345_Read_Regs(dev, REG_DATAX0, Datareg, 6);
	int16_t XYZ_ACC_datas[3];


	// 10-bit resolution mode, 2^9 due to twos complement
		switch(dev->Range)
		{
			case RANGE_2g:
				scale_factor = 2 /(float)(1 << 9);
				break;

			case RANGE_4g:
				scale_factor = 4 /(float)(1 << 9);
				break;

			case RANGE_8g:
				scale_factor = 8 /(float)(1 << 9);
				break;

			case RANGE_16g:
				scale_factor = 16 /(float)(1 << 9);
				break;
		}

	// RAW data
	XYZ_ACC_datas[0] = ((int16_t) Datareg[0] | ((int16_t) Datareg[1] << 8));
	XYZ_ACC_datas[1] = ((int16_t) Datareg[2] | ((int16_t) Datareg[3] << 8));
	XYZ_ACC_datas[2] = ((int16_t) Datareg[4] | ((int16_t) Datareg[5] << 8));


	dev->acc_ms2[0] = 9.81f * XYZ_ACC_datas[0] * scale_factor;
	dev->acc_ms2[1] = 9.81f * XYZ_ACC_datas[1] * scale_factor;
	dev->acc_ms2[2] = 9.81f * XYZ_ACC_datas[2] * scale_factor;

	return status;
}

uint8_t ADXL345_SetOFS(ADXL345_t *dev, float OFSX, float OFSY, float OFSZ)
{
	if ( OFSX > 2 || OFSY > 2 || OFSZ > 2 )
		return 255;
	else if ( OFSX < -2 || OFSY < - 2 || OFSZ < -2 )
		return 255;

	uint8_t errors = 0;
	uint8_t OFSs[3];
	HAL_StatusTypeDef status;

	    status = ADXL345_Read_Reg(dev, REG_OFSX , &OFSs[0]);
		errors += (status != HAL_OK);

		status = ADXL345_Read_Reg(dev, REG_OFSY , &OFSs[1]);
		errors += (status != HAL_OK);

		status = ADXL345_Read_Reg(dev, REG_OFSZ , &OFSs[2]);
		errors += (status != HAL_OK);


	OFSs[0] = (uint8_t)(OFSX / .0156f);
	OFSs[1] = (uint8_t)(OFSY / .0156f);
	OFSs[2] = (uint8_t)(OFSZ / .0156f);

	status = ADXL345_Write_Reg(dev, REG_OFSX , &OFSs[0]);
	errors += (status != HAL_OK);

	status = ADXL345_Write_Reg(dev, REG_OFSY , &OFSs[1]);
	errors += (status != HAL_OK);

	status = ADXL345_Write_Reg(dev, REG_OFSZ , &OFSs[2]);
	errors += (status != HAL_OK);

	return errors;

}

uint8_t ADXL345_EnableINTERRUPTS(ADXL345_t *dev, ONOFF Data_Ready, ONOFF Single_Tap, ONOFF Double_Tap,
		ONOFF Activity, ONOFF Inactivity, ONOFF FreeFall, ONOFF Watermark, ONOFF Overrun)
{
	uint8_t errors = 0;
	uint8_t Data = 0x00;
	HAL_StatusTypeDef status;

	// Reset
	status = ADXL345_Write_Reg(dev, REG_INT_ENABLE , &Data);
	errors += (status != HAL_OK);

	Data = 0x50;
	status = ADXL345_Write_Reg(dev, REG_INT_MAP , &Data);
	errors += (status != HAL_OK);

	Data = 0x00;
    Data |= (Data_Ready << 7) | (Single_Tap << 6) | (Double_Tap << 5) |
		(Activity << 4)| (Inactivity << 3) | (FreeFall << 2) | (Watermark << 1) | Overrun ;

	status = ADXL345_Write_Reg(dev, REG_INT_ENABLE , &Data);
	errors += (status != HAL_OK);


	ADXL345_readACC(dev);

	return errors;

}

IntSource_t ADXL_GetSourceOfINT(ADXL345_t *dev)
{
		uint8_t Data = 0, errors = 0;
		HAL_StatusTypeDef status;


		status =  ADXL345_Read_Reg(dev, REG_ACT_TAP_STATUS, &Data);
		errors += (status != HAL_OK);
		Data = 0;
		status =  ADXL345_Read_Reg(dev, REG_INT_SOURCE, &Data);
		errors += (status != HAL_OK);

		for(IntSource_t type = 2; type < 7; type++)
		{
			if(Data & (1 << type))
			{
				return type;
			}
		}
		return NONE_INT;
}

const char *getAxis(ADXL345_t *dev)
{
	char *Axis[] = {"TAP_Z","TAP_Y","TAP_X","","ACT_Z","ACT_Y", "ACT_X"};
	return Axis[ADXL_GetAxisSource(dev)];

}



ActTapStatus_t ADXL_GetAxisSource(ADXL345_t *dev)
{
		uint8_t Data = 0;
		HAL_StatusTypeDef status;

		status =  ADXL345_Read_Reg(dev, REG_ACT_TAP_STATUS, &Data);
		for(ActTapStatus_t type = 0; type < 7; type++)
		{
			if((Data & (1 << type)) && type != ASleep)
			{
				return type;
			}
		}
		return NONE_STAT;
}

uint8_t ADXL345_Enable_ACT_AXES(ADXL345_t *dev, uint8_t X_ONOFF, uint8_t Y_ONOFF, uint8_t Z_ONOFF)
{

	uint8_t errors = 0;
	uint8_t Data = 0x00;
	HAL_StatusTypeDef status;


	status = ADXL345_Read_Reg(dev, REG_ACT_INACT_CTL , &Data);
	errors += (status != HAL_OK);

	Data &= 0x0F; Data |= (X_ONOFF << 6) | (Y_ONOFF << 5) | (Z_ONOFF << 4);
	status = ADXL345_Write_Reg(dev, REG_ACT_INACT_CTL , &Data);
	errors += (status != HAL_OK);

	return errors;
}

uint8_t ADXL345_Enable_INACT_AXES(ADXL345_t *dev, uint8_t X_ONOFF, uint8_t Y_ONOFF, uint8_t Z_ONOFF)
{

	uint8_t errors = 0;
	uint8_t Data = 0x00;
	HAL_StatusTypeDef status;

	// set link and auto_sleep
	status = ADXL345_Read_Reg(dev, REG_POWER_CTL , &Data);
	Data &= 0x0F; Data |= (LINK_ON << 5) | (AUTO_SLEEP_ON << 4);
	status = ADXL345_Write_Reg(dev, REG_POWER_CTL , &Data);
	errors += (status != HAL_OK);


	status = ADXL345_Read_Reg(dev, REG_ACT_INACT_CTL , &Data);
	errors += (status != HAL_OK);

	Data &= 0xF0; Data |= (X_ONOFF << 2) | (Y_ONOFF << 1) | Z_ONOFF;
	status = ADXL345_Write_Reg(dev, REG_ACT_INACT_CTL , &Data);
	errors += (status != HAL_OK);

	return errors;

}

uint8_t ADXL345_SET_ACT_PARAMS(ADXL345_t *dev, float Acc_INACT, float Acc_ACT, uint8_t Time)
{
	if( Time > 255 || Time ==0 || Acc_ACT > 16 || Acc_ACT <-16 || Acc_ACT == 0)
		return 0xff;

	uint8_t errors = 0;
	HAL_StatusTypeDef status;
	uint8_t Acc_ACT_8t = 0;
	uint8_t Acc_INACT_8t = 0;

	Acc_ACT    /= 9.81f; Acc_ACT_8t   = (uint8_t)(Acc_ACT /.0625f);
 	Acc_INACT  /= 9.81f; Acc_INACT_8t = (uint8_t)(Acc_INACT /.0625f);


	status = ADXL345_Write_Reg(dev, REG_TIME_INACT , &Time);
	errors += (status != HAL_OK);

	status = ADXL345_Write_Reg(dev, REG_THRESH_ACT ,&Acc_ACT_8t );
	errors += (status != HAL_OK);

	status = ADXL345_Write_Reg(dev, REG_THRESH_INACT , &Acc_INACT_8t);
	errors += (status != HAL_OK);


	return errors;

}



uint8_t ADXL345_ENABLE_SINGLE_TAP(ADXL345_t *dev, ONOFF onoff,  uint32_t DUR_time, float Thresh_Tap)
{
	if( DUR_time > 160 || DUR_time <= 0 ||Thresh_Tap > 16 || Thresh_Tap <-16 || Thresh_Tap == 0)
		return 0xff;

	uint8_t errors = 0;
	HAL_StatusTypeDef status;
	uint8_t DUR_8t = 0;
	uint8_t TAP_8t = 0;

	Thresh_Tap    /= 9.81f; TAP_8t   = (uint8_t)(Thresh_Tap /.0625f);
	DUR_8t = (uint8_t)(DUR_time /.625f);


	status = ADXL345_Write_Reg(dev, REG_THRESH_TAP , &TAP_8t);
	errors += (status != HAL_OK);

	status = ADXL345_Write_Reg(dev, REG_DUR ,&DUR_8t );
	errors += (status != HAL_OK);

	DUR_8t = 0x01;
	status = ADXL345_Write_Reg(dev, REG_TAP_AXES , &DUR_8t);
	errors += (status != HAL_OK);


	return errors;

}

