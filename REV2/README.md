Crimson Dynamics REV2

## Description
Pi HAT with sensors and UI to count the number of overhead drilling actions accomlished:
- 1 x ICM20948 9-Dof IMU 
- 1 x SPH0645LM4H MEMS Microphone
- 3 x pushbuttons
- 2 x LEDs
- 1 x 128x32 I2C OLED Display 
- 1 x Raspberry Pi Zero W v1.1

#### Python scripts
Crimson_button.py runs at boot and imports functions from Crimson_data_log.py and Crimson_action_count.py

#### Usage:
- Boot up the Rasberry Pi and wait for the OLED display to refresh. This might take up to 3-4 min.  
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
- To view code outputs you can hook up a display to the pi or ssh into it (or use VNC). You can also uncomment print statements in the code to track the action countinfg process. 

## Hardware files
Eagle files and gerber files for the Pi HAT

## Other files
* Notes from meeting with Bunnie on 24.09.2019, discussing how to go about improving the current demo and going about the next version 