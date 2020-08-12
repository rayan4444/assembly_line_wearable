Crimson Dynamics REV1

## Decription
Tries to detect drilling acitons by matching accelerometer peaks to microphone peaks. 
- Attempted doing it purely by placing the IMU on the hand holding the drill and running FFT but as the body acted as a low pass filter and filetered out higher frequencies that could have had information to characterise drilling vibrations. 
- Decided to addd a microphone to provide a second input. 
- In the final iteration of REV1, a simple peak finding algorithm from signal.scipy library is used to identify peaks in accelration and microphone recording. If the peaks happen within a second from each other they are assumed to belong to the same drilling action. Assuming the worker only does drilling tasks, this can be used to count the number of tasks. 

#### Notes:
- detailed assumptions in code
- sigle movements (vs vibrations) are not counted as drilling motion reducing false positives. 
- the code is in different scripts and run very slowly. 

## Hardware files 
ICM20948 breakout board slightly modified from the [Sparkfun one](https://www.sparkfun.com/products/15335)
## Python scripts 
#### Crimson_data_log.py
Records 10s of IMU and Microphone data, saves it in files "sensor_log.csv" and "output.wav" respectively. Later splits sensor log in "ax.csv", "ay.csv" and "az.csv" for further analysis. 

#### Crimson_action_count.py
- Opens data files and runs peak finding algorithm 
- Plots peaks found 
- For each of ax, ay, az and mic data, groups peaks that are within 0.5 seconds of each other as belonging to one "action" 
- To reduce the occurrance of false positives, only considers "actions" with peaks in at least two of the 3 accelerometer axes. 
- Only Counts "actions" with peaks in both microphone and accelerometer data (peak cluster averages within 1s of each other) 
- Prints total numberof valid "actions"


## OLD CODE
All the other python scripts used to test various things, such as IMU and microphone recording, data hadnling and plotting, FFT analysis etc. 

## TEST DATA
Data recorded with the prototype and used to tweak the peak finidng algorithm. TEST 5 is the best. 
In this version:
- 1 x ICM20948 9-Dof IMU (Eagle files for the breakout board are in this folder) 
- 1 x SPH0645LM4H MEMS Microphone breakout (from taobao)
- 1x Raspberry Pi (Raspberry Pi Zero W v1.1 used in my tests)