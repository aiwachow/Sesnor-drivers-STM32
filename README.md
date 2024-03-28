## Some simple sensors I2C drivers writes on STM32. 
All of them are written in the same scheme 
- First make object ex. BMI088_t bmi008_device  
- Use initialization function with handle to I2C you currently use  
``` c
BMI088_INIT(BMI088_t *dev, I2C_HandleTypeDef *i2cHandle)
```
Note* there are no functions to rechange the data rate etc. 

- That's all, read measurments 
``` c
BMI088_ReadAcc(BMI088_t *dev)
```

## Important comments on drivers 
QMC - the measurement procedure includes hard and soft iron calibration. 
I have added a basic matlab script in which to put the raw magnetometer data in csv format. 
