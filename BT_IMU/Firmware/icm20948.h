#ifndef _ICM20948_H
#define _ICM20948_H

#include <stdint.h>

typedef enum Userbank {
    USER_BANK_0,
    USER_BANK_1,
    USER_BANK_2,
    USER_BANK_3,
} Userbank;

typedef enum GyroRange {
    GYRO_RANGE_250DPS,
    GYRO_RANGE_500DPS,
    GYRO_RANGE_1000DPS,
    GYRO_RANGE_2000DPS
} GyroRange;

typedef enum AccelRange {
    ACCEL_RANGE_2G,
    ACCEL_RANGE_4G,
    ACCEL_RANGE_8G,
    ACCEL_RANGE_16G    
} AccelRange;

typedef enum AccelDlpfBandwidth {
    ACCEL_DLPF_BANDWIDTH_1209HZ,
    ACCEL_DLPF_BANDWIDTH_246HZ,
    ACCEL_DLPF_BANDWIDTH_111HZ,
    ACCEL_DLPF_BANDWIDTH_50HZ,
    ACCEL_DLPF_BANDWIDTH_24HZ,
    ACCEL_DLPF_BANDWIDTH_12HZ,
    ACCEL_DLPF_BANDWIDTH_6HZ,
    ACCEL_DLPF_BANDWIDTH_473HZ
} AccelDlpfBandwidth;

typedef enum GyroDlpfBandwidth {
    GYRO_DLPF_BANDWIDTH_12106HZ,
    GYRO_DLPF_BANDWIDTH_197HZ,
    GYRO_DLPF_BANDWIDTH_152HZ,
    GYRO_DLPF_BANDWIDTH_120HZ,
    GYRO_DLPF_BANDWIDTH_51HZ,
    GYRO_DLPF_BANDWIDTH_24HZ,
    GYRO_DLPF_BANDWIDTH_12HZ,
    GYRO_DLPF_BANDWIDTH_6HZ,
    GYRO_DLPF_BANDWIDTH_361HZ
} GyroDlpfBandwidth;

typedef enum LpAccelOdr {
    LP_ACCEL_ODR_0_24HZ = 0,
    LP_ACCEL_ODR_0_49HZ = 1,
    LP_ACCEL_ODR_0_98HZ = 2,
    LP_ACCEL_ODR_1_95HZ = 3,
    LP_ACCEL_ODR_3_91HZ = 4,
    LP_ACCEL_ODR_7_81HZ = 5,
    LP_ACCEL_ODR_15_63HZ = 6,
    LP_ACCEL_ODR_31_25HZ = 7,
    LP_ACCEL_ODR_62_50HZ = 8,
    LP_ACCEL_ODR_125HZ = 9,
    LP_ACCEL_ODR_250HZ = 10,
    LP_ACCEL_ODR_500HZ = 11
} LpAccelOdr;


// buffer for reading from sensor
uint8_t _buffer[21];

//configuration variables
uint8_t _gyroSrd;
uint16_t _accelSrd;

float _accelScale;
float _gyroScale;

AccelRange _accelRange;
GyroRange _gyroRange;

AccelDlpfBandwidth _accelBandwidth;
GyroDlpfBandwidth _gyroBandwidth;


//gyro bias estimation 
float _gxb, _gyb, _gzb;

// accel bias and scale factor estimation
float _axmax, _aymax, _azmax;
float _axmin, _aymin, _azmin;
float _axb, _ayb, _azb;

//magnetometer bias and scale factor estimation
uint16_t _counter;
float _framedelta, _delta;
float _hxfilt, _hyfilt, _hzfilt;
float _hxmax, _hymax, _hzmax;
float _hxmin, _hymin, _hzmin;
float _hxb, _hyb, _hzb;
float _avgs;

// wake on motion
uint8_t _womThreshold;

// data counts
int16_t _axcounts,_aycounts,_azcounts;
int16_t _gxcounts,_gycounts,_gzcounts;
int16_t _hxcounts,_hycounts,_hzcounts;
int16_t _tcounts;

// data buffer
float _ax, _ay, _az;
float _gx, _gy, _gz;
float _hx, _hy, _hz;
float _t;

int ICM20948_init();
void ICM20948_read();
void change_userbank(Userbank userbank);
void selectAutoClockSource();
void enableAccelGyro();
void reset();
void enableI2Cmaster();
uint8_t whoAmI();
uint8_t whoAmIMag();
void powerDownMag();
void resetMag();
void enableDataReadyInterrupt();
void disableDataReadyInterrupt();
void writeMagRegister(uint8_t subAddress, uint8_t data);
void readMagRegisters(uint8_t subAddress, uint8_t count, uint8_t* dest);

//configuration functions
void setGyroSrd(uint8_t srd);
void setAccelSrd(uint16_t srd);
void configAccel(AccelRange range, AccelDlpfBandwidth bandwidth);
void configGyro(GyroRange range, GyroDlpfBandwidth bandwidth);
void configMag();

extern void  writeRegister(uint8_t subAddress, uint8_t data);
extern void readRegisters(uint8_t subAddress, uint8_t count, uint8_t *dest);
/*
* KEY REGISTERS
*/

// User bank selection
#define REG_BANK_SEL                0x7F
#define REG_BANK_SEL_USER_BANK_0    0x00
#define REG_BANK_SEL_USER_BANK_1    0x10
#define REG_BANK_SEL_USER_BANK_2    0x20
#define REG_BANK_SEL_USER_BANK_3    0x30

// User bank 0
#define UB0_WHO_AM_I                0x00
#define UB0_USER_CTRL               0x03
#define UB0_USER_CTRL_I2C_MST_EN    0x20

#define UB0_PWR_MGMNT_1             0x06
#define UB0_PWR_MGMNT_1_CLOCK_SEL_AUTO 0x01
#define UB0_PWR_MGMNT_1_DEV_RESET   0x80
#define UB0_PWR_MGMNT_2             0x07
#define UB0_PWR_MGMNT_2_SEN_ENABLE  0x00    

#define UB0_INT_PIN_CFG             0x0F  
#define UB0_INT_PIN_CFG_HIGH_50US   0x00
#define UB0_INT_ENABLE_1            0x11 
#define UB0_INT_ENABLE_1_RAW_RDY_EN 0x01
#define UB0_INT_ENABLE_1_DIS        0x00

#define UB0_ACCEL_XOUT_H            0x2D
#define UB0_EXT_SLV_SENS_DATA_00    0x3B

// User bank 2
#define UB2_GYRO_SMPLRT_DIV 0x00
#define UB2_GYRO_CONFIG_1  0x01
#define UB2_GYRO_CONFIG_1_FS_SEL_250DPS 0x00
#define UB2_GYRO_CONFIG_1_FS_SEL_500DPS 0x02
#define UB2_GYRO_CONFIG_1_FS_SEL_1000DPS 0x04
#define UB2_GYRO_CONFIG_1_FS_SEL_2000DPS 0x06
#define UB2_GYRO_CONFIG_1_DLPFCFG_12106HZ  0x00
#define UB2_GYRO_CONFIG_1_DLPFCFG_197HZ 0x00 | 0x01
#define UB2_GYRO_CONFIG_1_DLPFCFG_152HZ 0b00001000 | 0x01
#define UB2_GYRO_CONFIG_1_DLPFCFG_120HZ 0b00010000 | 0x01
#define UB2_GYRO_CONFIG_1_DLPFCFG_51HZ 0b00011000 | 0x01
#define UB2_GYRO_CONFIG_1_DLPFCFG_24HZ 0b00100000 | 0x01
#define UB2_GYRO_CONFIG_1_DLPFCFG_12HZ 0b00101000 | 0x01
#define UB2_GYRO_CONFIG_1_DLPFCFG_6HZ 0b00110000 | 0x01
#define UB2_GYRO_CONFIG_1_DLPFCFG_361HZ 0b00111000 | 0x01

#define UB2_ACCEL_SMPLRT_DIV_1          0x10
#define UB2_ACCEL_SMPLRT_DIV_2          0x11

#define UB2_ACCEL_CONFIG                0x14
#define UB2_ACCEL_CONFIG_FS_SEL_2G      0x00
#define UB2_ACCEL_CONFIG_FS_SEL_4G      0x02
#define UB2_ACCEL_CONFIG_FS_SEL_8G      0x04
#define UB2_ACCEL_CONFIG_FS_SEL_16G     0x06

#define UB2_ACCEL_CONFIG_DLPFCFG_1209HZ     0x00
#define UB2_ACCEL_CONFIG_DLPFCFG_246HZ      0x00 | 0x01
#define UB2_ACCEL_CONFIG_DLPFCFG_111HZ      0b00010000 | 0x01
#define UB2_ACCEL_CONFIG_DLPFCFG_50HZ       0b00011000 | 0x01
#define UB2_ACCEL_CONFIG_DLPFCFG_24HZ       0b00100000 | 0x01
#define UB2_ACCEL_CONFIG_DLPFCFG_12HZ       0b00101000 | 0x01
#define UB2_ACCEL_CONFIG_DLPFCFG_6HZ        0b00110000 | 0x01
#define UB2_ACCEL_CONFIG_DLPFCFG_473HZ      0b00111000 | 0x01

//user bank 3
#define UB3_I2C_MST_CTRL             0x01
#define UB3_I2C_MST_CTRL_CLK_400KHZ  0x07 // Gives 345.6kHz and is recommended to achieve max 400kHz
#define UB3_I2C_SLV0_ADDR            0x03
#define UB3_I2C_SLV0_ADDR_READ_FLAG  0x80
#define UB3_I2C_SLV0_REG             0x04
#define UB3_I2C_SLV0_CTRL            0x05
#define UB3_I2C_SLV0_CTRL_EN         0x80
#define UB3_I2C_SLV0_DO              0x06

// Magnetometer constants
#define MAG_AK09916_I2C_ADDR 0x0C
#define MAG_AK09916_WHO_AM_I 0x4809
#define MAG_DATA_LENGTH      8 // Bytes

// Magnetometer (AK09916) registers
#define MAG_WHO_AM_I            0x00
#define MAG_HXL                 0x11
#define MAG_CNTL2               0x31
#define MAG_CNTL2_POWER_DOWN    0x00
#define MAG_CNTL2_MODE_10HZ     0x02
#define MAG_CNTL2_MODE_50HZ     0x06
#define MAG_CNTL2_MODE_100HZ    0x08
#define MAG_CNTL3               0x32
#define MAG_CNTL3_RESET         0x01    

#endif //_ICM20948_H