# Programming Guide
Notes and steps to follow to easily program the CRIMSON_IMU_BT board.

## Pinout
| Signal | NRF52 Pin |
|:---:|:---:|
|LED_R|P0.13|
|LED_G|P0.15|
|LED_B|P0.17|
|SCL|P.019|
|SDA| P0.18|
|IMU_INT|P0.25|
|SPI_MOSI|P0.27|
|SPI_MISO|P0.26|
|SPI_SCK|P0.28|
|SPI_CS|P0.29|
|CARD_DETECT|P0.23|
|TX|P0.06|
|RX|P0.08|
|CTS|P0.07|
|RTS|P0.05|
|BAT_SENSE|AIN6/P0.30|
|BUTTON|P0.14|
|RESET|P0.21|

## Flash and Debug setup
Follow the [tutorial](https://gitlab.com/hax_eng/hax-open/vs_code_setup_tutorials/tree/master/NRF52) to setup your development environment if you haven't already.
> I use ARMGCC and GDB to compile and debug, but you can choose an entirely different setup

#### Using the NRF52 Development kit to program a custom board
You can use the onboard J-link adapter of the NRF52 Development Kit to program your custom board.

**Connections**

|NRF52 Dev Kit| Custom PCB|
|:---:|:---:|
|VTG|3V3|
|SWDIO|SWDIO|
|SWCLK|SWCLK|
|GND|GND|
|RESET*|RESET*|

\* Optional
>Make sure you power your Custom board from an external source. The VTG connection  from the Dev Kit does not power the board, it just detects the target voltage.

## Directions of use
* Power on or reset device: a pink LED will stay on until you connect and pair with a Bluetooth host (ex: your smartphone. you can use NRFconnect mobile app).
* Once the device is connected and paired, the light will turn blue. 
* Stand in a neutral position with the arm down. Press the button to calibrate the device: this will sync time and record gyro offsets. 
* Once the device is calibrated, you can go a head and use it: it will log "up" and "down" events in the SD card. 

## Feature implementation
Already implemented:
* BLE connection
* Time sync
* Reading sensor data
* Processing sensor data (Madgwick filter) to determine orientation
* Logging "up" (event type 1) and "down"  (event type 0) events with timestamp
* Writing events to SD card
* Reading battery ADC

To be implemented:
* Read SD card file over UART
* Read SD card file over BLE
* Monitor battery charging 
* Battery level service over BLE

Notes:
* The device updates roll pitch and yaw angles with reference to earth gravity so you can create more event types by defining specific angle thresholds. 

## Errata
