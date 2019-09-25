# Crimson Dynamic

Hardware files and code for Crimson Dynamic (HAX Shenzhen 2019) sensor system prototyping 

## Objective
Design a sensor system that adds value to the exoskeleton, specifically for use in an overhead assemby line in car factories. Potential insights the sensor system could provide:
* Task compliance: ensure a worker did not forget a bolt/nut on a car
* long term vibration exposure
* long term arm fatigue 
* long term noise exposure 
 * etc. 

## REV 1
Sensor breakout to count the number of overhead drilling actions accomlished. Looked into doing an FFT anaylis on IMU data and microphone data. Decided to settle on matching IMU data peaks to microphone data peaks. The same two sensors could be use to quantify the long term exposure to vibration, noise and fatigue. 

## REV2
Pi HAT with sensors and UI to count the number of overhead drilling actions accomlished. A more efficient rendition of REV1 that is usable for demonstration. 
> Note: Data is post processed and results are not given in real time

## REV3
*(TBD)*