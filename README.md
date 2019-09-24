# Crimson Dynamic

Hardware files and code for Crimson Dynamic (HAX Shenzhen 2019) sensor system prototyping 

## REV 1
In this version:
- 1 x ICM20948 9-Dof IMU (Eagle files for the breakout board are in this folder) 
- 1 x SPH0645LM4H MEMS Microphone breakout (from taobao)
- 1x Raspberry Pi (Raspberry Pi Zero W v1.1 used in my tests)

#### Usage: 
Tries to detect drilling acitons by matching accelerometer peaks to microphone peaks. 
- Attempted doing it purely by placing the IMU on the hand holding the drill and running FFT but as the body acted as a low pass filter and filetered out higher frequencies that could have had information to characterise drilling vibrations. 
- Decided to addd a microphone to provide a second input. 
- In the final iteration of REV1, a simple peak finding algorithm from signal.scipy library is used to identify peaks in accelration and microphone recording. If the peaks happen within a second from each other they are assumed to belong to the same drilling action. Assuming the worker only does drilling tasks, this can be used to count the number of tasks. 

#### Notes:
- detailed assumptions in code
- sigle movements (vs vibrations) are not counted as drilling motion reducing false positives. 
- the code is in different scripts and run very slowly. 

## REV2
Pi HAT with sensors and UI to count the number of overhead drilling actions accomlished:
- 1 x ICM20948 9-Dof IMU 
- 1 x SPH0645LM4H MEMS Microphone
- 3 x pushbuttons
- 2 x LEDs
- 1 x 128x32 I2C OLED Display 
- 1 x Raspberry Pi Zero W v1.1

#### Usage:
- Pressing "start" button records 40s of data from the microphone and the IMU (x,y,z acceleration to be specific). 
- Green LED lights up and stays on after the recording is done if everything went well 
- Pressing "stop" button analyses results, using the latest action counting technique from rev1.
> By default plotting is not enabled because it slows everything down and requires a way to close the plot window to proceed. but in case it is needed, plt.show() can be uncommented from Crimson_action_count.py
- Orange LED lights up and stays on after processing is done if everything went well. The final count will show on the display for a couple seconds. 
- Pressing "BTN" button deletes all data records. If you want to keep the recorded data, ssh into the Rpi and copy it to another directory before deleting it. 
> If you don't delete the data, the next recording will just be appended to the current file and give false action count results. 

#### Notes: 
- Display breakout on the PCB doesn't match the module we purchsed so some reworking had to be done to be able to wire it properly 
- A single python script that can be run from boot. It imports functions from two other python files. These functions are very similar to the REV1 code, a little has been done to optimise it. 