## Some simple sensors drivers writes on STM32. 
All of them are written in the same scheme 
- First make object ex. BMI088_t bmi008_device  
- Use initialization function with handle to I2C you currently use ex. 
``` c
#include "stm32l4xx_hal.h"
```
in [MPU6050.h](/home/fryderyk/MPU6050/mpu6050/MPU6050/Core/Inc/MPU6050.h)  to HAL version matching your microcontroller.

