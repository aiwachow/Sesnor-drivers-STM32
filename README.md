## Some simple sensors drivers writes on STM32. 
All of them are written in the same scheme 
- First make object ex. BMI088_t bmi008_device  
- Use initialization function with handle to I2C you currently use ex. 
  ...
  init()
  ...
- Read data with read functions
  that's all 

  
