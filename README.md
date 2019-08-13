# Crimson Dynamic

Hardware files and code for Crimson Dynamic (HAX Shenzhen 2019) sensor system prototyping 

## REV 1
In this version:
- 1 x ICM20948 9-Dof IMU (Eagle files for the breakout board are in this folder) 
- 1x Raspberry Pi (Raspberry Pi Zero W v1.1 used in my tests)

Wiring: 
- I2C (SDA: GPIO2, SCL: GPIO3)
- VCC: 3.3V 

Progress: 
- data acquisition: Done
- real time plotting: Done
- recording to csv file: 
- FFT: 


## Files
- Crimson_IMU_plot.py: real time 9-DoF IMU data plotting 
- Crimson_IMU_log.py: save 30s of sensor data in a csv file 
- Crimson_IMU_FFT.py: plot data from csv file and run FFT. Display FFT results 
-  


