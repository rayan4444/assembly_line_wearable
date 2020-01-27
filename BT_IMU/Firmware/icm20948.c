#include "icm20948.h"
#include "nrf_delay.h"

// transformation matrix
/* transform the magnetometer values to match the coordinate system of the IMU */
const int16_t tX[3] = {1,  0,  0}; 
const int16_t tY[3] = {0, -1,  0};
const int16_t tZ[3] = {0,  0, -1};
// constants
const float G = 9.807f;
const float _d2r = 3.14159265359f/180.0f;
const float _tempScale = 333.87f;
const float _tempOffset = 21.0f;

const float accRawScaling = 32767.5f; // =(2^16-1)/2 16 bit representation of acc value to cover +/- range
const float gyroRawScaling = 32767.5f; // =(2^16-1)/2 16 bit representation of gyro value to cover +/- range
const float magRawScaling = 32767.5f; // =(2^16-1)/2 16 bit representation of gyro value to cover +/- range 
const float _magScale = 4912.0f / magRawScaling; // micro Tesla, measurement range is +/- 4912 uT.

const uint8_t ICM20948_WHO_AM_I = 0xEA;

//accel bias and scale factor estimation
float _axs = 1.0f;
float _ays = 1.0f;
float _azs = 1.0f;

// magnetometer bias and scale factor estimation
uint16_t _maxCounts = 1000;
float _deltaThresh = 0.3f;
uint8_t _coeff = 8;
float _hxs = 1.0f;
float _hys = 1.0f;
float _hzs = 1.0f;

// select register user bank
void change_userbank(Userbank userbank){    
    uint8_t userBankRegValue = 0x00;
    switch(userbank){
        case USER_BANK_0:
            userBankRegValue = REG_BANK_SEL_USER_BANK_0;
            break;
        case USER_BANK_1:
            userBankRegValue = REG_BANK_SEL_USER_BANK_1;
            break;
        case USER_BANK_2:
            userBankRegValue = REG_BANK_SEL_USER_BANK_2;
            break;
        case USER_BANK_3:
            userBankRegValue = REG_BANK_SEL_USER_BANK_3;
            break;
    }
    writeRegister(REG_BANK_SEL, userBankRegValue);
}

//select the sensor clock source
void selectAutoClockSource(){
    change_userbank(USER_BANK_0);
    writeRegister(UB0_PWR_MGMNT_1, UB0_PWR_MGMNT_1_CLOCK_SEL_AUTO);
}

//enable I2Cmaster
void enableI2Cmaster(){
    change_userbank(USER_BANK_0);
    writeRegister(UB0_USER_CTRL, UB0_USER_CTRL_I2C_MST_EN);
    change_userbank(USER_BANK_3);
    writeRegister(UB3_I2C_MST_CTRL, UB3_I2C_MST_CTRL_CLK_400KHZ);
}

//enable accelerometer and gyroscope 
void enableAccelGyro(){
    change_userbank(USER_BANK_0);
    writeRegister(UB0_PWR_MGMNT_2, UB0_PWR_MGMNT_2_SEN_ENABLE);
}

//reset ICM20948
void reset(){
    change_userbank(USER_BANK_0);
    writeRegister(UB0_PWR_MGMNT_1, UB0_PWR_MGMNT_1_DEV_RESET);
}

//read accel & gyro who am I register
uint8_t whoAmI(){
    change_userbank(USER_BANK_0);
    //read WHO AM I Register
    readRegisters(UB0_WHO_AM_I,1,_buffer);
    // return the register value
    return _buffer[0];
}

//Read magnetormer who am I register
uint8_t whoAmIMag(){
    readMagRegisters(MAG_WHO_AM_I, 2, _buffer);
    return (_buffer[0] << 8) + _buffer[1];
}

// Power down magnetometer
void powerDownMag(){
    writeMagRegister(MAG_CNTL2, MAG_CNTL2_POWER_DOWN);
}

//reset magnetometer
void resetMag(){
    writeMagRegister(MAG_CNTL3, MAG_CNTL3_RESET);
}

//enable data ready interrupt 
void enableDataReadyInterrupt(){
    change_userbank(USER_BANK_0);
    writeRegister(UB0_INT_PIN_CFG, UB0_INT_PIN_CFG_HIGH_50US); // setup interrupt, 50 us pulse
    writeRegister(UB0_INT_ENABLE_1, UB0_INT_ENABLE_1_RAW_RDY_EN); //sset to data ready
}

//disable data ready interrupt
void disableDataReadyInterrupt(){
    change_userbank(USER_BANK_0);
    writeRegister(UB0_INT_ENABLE_1, UB0_INT_ENABLE_1_DIS); //disable interrupt
}

void setGyroSrd(uint8_t srd){
    change_userbank(USER_BANK_2);
    writeRegister(UB2_GYRO_SMPLRT_DIV, srd);
    _gyroSrd = srd;
}

void setAccelSrd(uint16_t srd){
    change_userbank(USER_BANK_2);
    uint8_t srdHigh = srd >> 8 & 0x0F;
    writeRegister(UB2_ACCEL_SMPLRT_DIV_1, srdHigh);
    uint8_t srdLow = srd & 0x0F;
    writeRegister(UB2_ACCEL_SMPLRT_DIV_2, srdLow);
    _accelSrd = srd;
}

void configAccel(AccelRange range, AccelDlpfBandwidth bandwidth){
    change_userbank(USER_BANK_2);
    uint8_t accelRangeRegValue = 0x00;
    float accelScale = 0.0f;  
    switch(range) {
    case ACCEL_RANGE_2G: 
      accelRangeRegValue = UB2_ACCEL_CONFIG_FS_SEL_2G;
      accelScale = G * 2.0f/accRawScaling; // setting the accel scale to 2G
      break; 
    
    case ACCEL_RANGE_4G: 
      accelRangeRegValue = UB2_ACCEL_CONFIG_FS_SEL_4G;
      accelScale = G * 4.0f/accRawScaling; // setting the accel scale to 4G
      break;
    
    case ACCEL_RANGE_8G: 
      accelRangeRegValue = UB2_ACCEL_CONFIG_FS_SEL_8G;
      accelScale = G * 8.0f/accRawScaling; // setting the accel scale to 8G
      break;
    
    case ACCEL_RANGE_16G: 
      accelRangeRegValue = UB2_ACCEL_CONFIG_FS_SEL_16G;
      accelScale = G * 16.0f/accRawScaling; // setting the accel scale to 16G
      break;
    
    }
  uint8_t dlpfRegValue = 0x00;
  switch(bandwidth) {
  	case ACCEL_DLPF_BANDWIDTH_1209HZ: dlpfRegValue = UB2_ACCEL_CONFIG_DLPFCFG_1209HZ; break;
  	case ACCEL_DLPF_BANDWIDTH_246HZ: dlpfRegValue = UB2_ACCEL_CONFIG_DLPFCFG_246HZ; break;
  	case ACCEL_DLPF_BANDWIDTH_111HZ: dlpfRegValue = UB2_ACCEL_CONFIG_DLPFCFG_111HZ; break;
  	case ACCEL_DLPF_BANDWIDTH_50HZ: dlpfRegValue = UB2_ACCEL_CONFIG_DLPFCFG_50HZ; break;
  	case ACCEL_DLPF_BANDWIDTH_24HZ: dlpfRegValue = UB2_ACCEL_CONFIG_DLPFCFG_24HZ; break;
  	case ACCEL_DLPF_BANDWIDTH_12HZ: dlpfRegValue = UB2_ACCEL_CONFIG_DLPFCFG_12HZ; break;
  	case ACCEL_DLPF_BANDWIDTH_6HZ: dlpfRegValue = UB2_ACCEL_CONFIG_DLPFCFG_6HZ; break;
  	case ACCEL_DLPF_BANDWIDTH_473HZ: dlpfRegValue = UB2_ACCEL_CONFIG_DLPFCFG_473HZ; break;
  }
  writeRegister(UB2_ACCEL_CONFIG, accelRangeRegValue | dlpfRegValue);
  _accelScale = accelScale;
  _accelRange = range;
  _accelBandwidth = bandwidth;
  
}


void configGyro(GyroRange range, GyroDlpfBandwidth bandwidth){
    change_userbank(USER_BANK_2);
    uint8_t gyroConfigRegValue = 0x00;
     float gyroScale = 0x00;
    switch(range) {
        case GYRO_RANGE_250DPS: 
    	gyroConfigRegValue = UB2_GYRO_CONFIG_1_FS_SEL_250DPS;
        gyroScale = 250.0f/gyroRawScaling * _d2r; // setting the gyro scale to 250DPS
      break;
    
    case GYRO_RANGE_500DPS: 
      gyroConfigRegValue = UB2_GYRO_CONFIG_1_FS_SEL_500DPS;
      gyroScale = 500.0f/gyroRawScaling * _d2r; // setting the gyro scale to 500DPS
      break; 
    
    case GYRO_RANGE_1000DPS: 
      gyroConfigRegValue = UB2_GYRO_CONFIG_1_FS_SEL_1000DPS;
      gyroScale = 1000.0f/gyroRawScaling * _d2r; // setting the gyro scale to 1000DPS
      break;
    
    case GYRO_RANGE_2000DPS: 
      gyroConfigRegValue = UB2_GYRO_CONFIG_1_FS_SEL_2000DPS;
      gyroScale = 2000.0f/gyroRawScaling * _d2r; // setting the gyro scale to 2000DPS
      break;
  }
  uint8_t dlpfRegValue = 0x00;
  switch(bandwidth) {
  	case GYRO_DLPF_BANDWIDTH_12106HZ: dlpfRegValue = UB2_GYRO_CONFIG_1_DLPFCFG_12106HZ; break;
  	case GYRO_DLPF_BANDWIDTH_197HZ: dlpfRegValue = UB2_GYRO_CONFIG_1_DLPFCFG_197HZ; break;
  	case GYRO_DLPF_BANDWIDTH_152HZ: dlpfRegValue = UB2_GYRO_CONFIG_1_DLPFCFG_152HZ; break;
  	case GYRO_DLPF_BANDWIDTH_120HZ: dlpfRegValue = UB2_GYRO_CONFIG_1_DLPFCFG_120HZ; break;
  	case GYRO_DLPF_BANDWIDTH_51HZ: dlpfRegValue = UB2_GYRO_CONFIG_1_DLPFCFG_51HZ; break;
  	case GYRO_DLPF_BANDWIDTH_24HZ: dlpfRegValue = UB2_GYRO_CONFIG_1_DLPFCFG_24HZ; break;
  	case GYRO_DLPF_BANDWIDTH_12HZ: dlpfRegValue = UB2_GYRO_CONFIG_1_DLPFCFG_12HZ; break;
  	case GYRO_DLPF_BANDWIDTH_6HZ: dlpfRegValue = UB2_GYRO_CONFIG_1_DLPFCFG_6HZ; break;
  	case GYRO_DLPF_BANDWIDTH_361HZ: dlpfRegValue = UB2_GYRO_CONFIG_1_DLPFCFG_361HZ; break;
  }
    writeRegister(UB2_GYRO_CONFIG_1, gyroConfigRegValue | dlpfRegValue);
    _gyroScale = gyroScale;
    _gyroRange = range;
    _gyroBandwidth = bandwidth;

}

void configMag(){
    writeMagRegister(MAG_CNTL2, MAG_CNTL2_MODE_100HZ);
}

//sensor initialisation sequence
int ICM20948_init(){
    change_userbank(USER_BANK_0);
    selectAutoClockSource();
    enableI2Cmaster();
    powerDownMag();
    reset();
    nrf_delay_ms(1);
    resetMag();
    selectAutoClockSource();
    if (whoAmI()!=ICM20948_WHO_AM_I){
        return -1;
    }
    enableAccelGyro();
    configAccel(ACCEL_RANGE_16G, ACCEL_DLPF_BANDWIDTH_246HZ);
    configGyro(GYRO_RANGE_2000DPS, GYRO_DLPF_BANDWIDTH_197HZ);
    setGyroSrd(0);
    setAccelSrd(0);
    enableI2Cmaster();
    // if(whoAmIMag() != MAG_AK09916_WHO_AM_I ) {
    // return -2;
	// }
    // configMag();
    selectAutoClockSource();
    // readMagRegisters(MAG_HXL, MAG_DATA_LENGTH, _buffer);
    return 0;
}


void ICM20948_read(){
    change_userbank(USER_BANK_0);
    readRegisters(UB0_ACCEL_XOUT_H, 20, _buffer);

    // combine into 16 bit values
  _axcounts = (((int16_t)_buffer[0]) << 8) | _buffer[1];  
  _aycounts = (((int16_t)_buffer[2]) << 8) | _buffer[3];
  _azcounts = (((int16_t)_buffer[4]) << 8) | _buffer[5];
  _gxcounts = (((int16_t)_buffer[6]) << 8) | _buffer[7];
  _gycounts = (((int16_t)_buffer[8]) << 8) | _buffer[9];
  _gzcounts = (((int16_t)_buffer[10]) << 8) | _buffer[11];
  _tcounts = (((int16_t)_buffer[12]) << 8) | _buffer[13];
  _hxcounts = (((int16_t)_buffer[15]) << 8) | _buffer[14];
  _hycounts = (((int16_t)_buffer[17]) << 8) | _buffer[16];
  _hzcounts = (((int16_t)_buffer[19]) << 8) | _buffer[18];

  // transform and convert to float values
  _ax = (((float)_axcounts * _accelScale) - _axb)*_axs;
  _ay = (((float)_aycounts * _accelScale) - _ayb)*_ays;
  _az = (((float)_azcounts * _accelScale) - _azb)*_azs;
  _gx = ((float)_gxcounts * _gyroScale) - _gxb;
  _gy = ((float)_gycounts * _gyroScale) - _gyb;
  _gz = ((float)_gzcounts * _gyroScale) - _gzb;
  _t = ((((float) _tcounts) - _tempOffset)/_tempScale) + _tempOffset;
  _hx = (((float)(tX[0]*_hxcounts + tX[1]*_hycounts + tX[2]*_hzcounts) * _magScale) - _hxb)*_hxs;
  _hy = (((float)(tY[0]*_hxcounts + tY[1]*_hycounts + tY[2]*_hzcounts) * _magScale) - _hyb)*_hys;
  _hz = (((float)(tZ[0]*_hxcounts + tZ[1]*_hycounts + tZ[2]*_hzcounts) * _magScale) - _hzb)*_hzs;
}

void writeMagRegister(uint8_t subAddress, uint8_t data){
    change_userbank(USER_BANK_3);
    writeRegister(UB3_I2C_SLV0_ADDR, MAG_AK09916_I2C_ADDR);
    writeRegister(UB3_I2C_SLV0_REG, subAddress); // set the register to the desired magnetometer sub address
    writeRegister(UB3_I2C_SLV0_DO, data); //store data for write
    writeRegister(UB3_I2C_SLV0_CTRL, UB3_I2C_SLV0_CTRL_EN | (uint8_t)1); // enable I2C and send 1 byte
}

void readMagRegisters(uint8_t subAddress, uint8_t count, uint8_t* dest){
    change_userbank(USER_BANK_3);
    writeRegister(UB3_I2C_SLV0_ADDR, MAG_AK09916_I2C_ADDR | UB3_I2C_SLV0_ADDR_READ_FLAG);
    writeRegister(UB3_I2C_SLV0_REG, subAddress); // set register to the desired magnetometer subaddress
    writeRegister(UB3_I2C_SLV0_CTRL, UB3_I2C_SLV0_CTRL_EN | count); //enableI2C and request bytes
    nrf_delay_ms(1); //wait for registers to fill 
    
    // read the bytes off the ICM-20948 EXT_SLV_SENS_DATA registers
    change_userbank(USER_BANK_0);
    readRegisters(UB0_EXT_SLV_SENS_DATA_00, count, dest);
}