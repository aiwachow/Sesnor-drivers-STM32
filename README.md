## Some simple sensors drivers writes on STM32. 
All of them are written in the same scheme 
- First make object ex. BMI088_t bmi008_device  
- Use initialization function with handle to I2C you currently use ex. 
``` c
BMI088_INIT(BMI088_t *dev, I2C_HandleTypeDef *i2cHandle)
```
Note* there is no function to rechange the data rate etc. 

- Read measurments 
