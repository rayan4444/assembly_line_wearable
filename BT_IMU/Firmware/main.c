/**
 * Copyright (c) 2014 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup ble_sdk_uart_over_ble_main main.c
 * @{
 * @ingroup  ble_sdk_app_nus_eval
 * @brief    UART over BLE application main file.
 *
 * This file contains the source code for a sample application that uses the Nordic UART service.
 * This application uses the @ref srvlib_conn_params module.
 */


#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "sdk_config.h"
#include "crimson_ble.h"
#include "app_timer.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "nrf_pwr_mgmt.h"

#include "nrf_drv_twi.h"
#include "nrf_delay.h"
#include "custom_board.h"
#include "nrf_queue.h"
#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "icm20948.h"
#include "crimson_sdcard.h"
#include "MadgwickAHRS.h"

#define ICM20948_ADDR 0x68

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */
#define QUEUE_SIZE                      3

typedef enum arm_position{
    UP_POSITION,
    DOWN_POSITION,
    OTHER_POSITION
}arm_position;

arm_position previous_position; 

volatile bool log_available= false; 
volatile bool new_data_ready = false;
volatile uint32_t count=0;
volatile bool m_xfer_done;
#define TWI_INSTANCE_ID     0
nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

#define IMU_SAMPLE_TIMER_INTERVAL     APP_TIMER_TICKS(100)     // 100 ms intervals 
APP_TIMER_DEF(imu_sample_timer_id);

float dt = 0.01; // time between two measurements 
float ax_accumulator, ay_accumulator, az_accumulator, gx_accumulator, gy_accumulator, gz_accumulator; //moving average filter accumulators

float angles[3]; // roll, pitch, yaw
float event_time= 12.3;
//circular buffer to store sensor data before running average filter
//the queue will overflow when it is full 
NRF_QUEUE_DEF(float, m_ax_queue, QUEUE_SIZE, NRF_QUEUE_MODE_OVERFLOW);
NRF_QUEUE_DEF(float, m_ay_queue, QUEUE_SIZE, NRF_QUEUE_MODE_OVERFLOW);
NRF_QUEUE_DEF(float, m_az_queue, QUEUE_SIZE, NRF_QUEUE_MODE_OVERFLOW);
NRF_QUEUE_DEF(float, m_gx_queue, QUEUE_SIZE, NRF_QUEUE_MODE_OVERFLOW);
NRF_QUEUE_DEF(float, m_gy_queue, QUEUE_SIZE, NRF_QUEUE_MODE_OVERFLOW);
NRF_QUEUE_DEF(float, m_gz_queue, QUEUE_SIZE, NRF_QUEUE_MODE_OVERFLOW);

//value to store the filtered sensor data 
float f_ax, f_ay, f_az, f_gx, f_gy, f_gz;
 
// calibration routine
bool calibrate(){
    // Turn on  LED configuration to indicate calibration (pink)
    nrf_gpio_pin_clear(LED_R);
    nrf_gpio_pin_clear(LED_B);

    // initialize running average filter accumulators
    for (int i=0; i<(QUEUE_SIZE-1); i++){
        ICM20948_read();
        ax_accumulator += _ax;
        ay_accumulator += _ay;
        az_accumulator += _az;
        gx_accumulator += _gx;
        gy_accumulator += _gy;
        gz_accumulator += _gz;

        //start filling up the queue
        ret_code_t err_code = nrf_queue_push(&m_ax_queue, &_ax);
        APP_ERROR_CHECK(err_code);
        err_code = nrf_queue_push(&m_ay_queue, &_ay);
        APP_ERROR_CHECK(err_code);
        err_code = nrf_queue_push(&m_az_queue, &_az);
        APP_ERROR_CHECK(err_code);
        err_code = nrf_queue_push(&m_gx_queue, &_gx);
        APP_ERROR_CHECK(err_code);
        err_code = nrf_queue_push(&m_gy_queue, &_gy);
        APP_ERROR_CHECK(err_code);
        err_code = nrf_queue_push(&m_gz_queue, &_gz);
        APP_ERROR_CHECK(err_code);

        //keep a rough 10Hz sampling frequency 
        nrf_delay_ms(10);
    }

    // determine initial gyro offset from average
    _gxb=gx_accumulator/((float)QUEUE_SIZE-1.0);
    _gyb=gy_accumulator/((float)QUEUE_SIZE-1.0);
    _gzb= gz_accumulator/((float)QUEUE_SIZE-1.0);
    
    printf("gyro bias: x: %f, y: %f, z: %f \n", _gxb, _gyb, _gzb);
    
    //init position
    previous_position = OTHER_POSITION;

    //clear LEDs
    nrf_delay_ms(1000);
    nrf_gpio_pin_set(LED_R);
    nrf_gpio_pin_set(LED_B);
    NRF_LOG_INFO("calibration DONE!");

    return 1;
}

// Moving Average filter 
float moving_average(nrf_queue_t const * p_queue, float newest_value, float* accumulator){
    //read all values in the queue
    float m_average =0;
    float oldest_value;
    float acc = *accumulator;
    
   
    // when the queue is full
    if (nrf_queue_is_full(p_queue)){
        //add the new value to to the sum 
        
        acc = acc + newest_value;
        //divide by total size of queue
        m_average = acc / ((float)QUEUE_SIZE);
        
        //remove oldest value
        nrf_queue_pop(p_queue, &oldest_value);
        acc = acc - oldest_value;

    }
    //update accumulator value 
    *accumulator = acc;
    return m_average;
}

void process_position(float  roll, float pitch, float yaw){
    arm_position new_position; 

    if (pitch <= -50.0){
        // printf("down \t");
        new_position = DOWN_POSITION;
        // if the posiiton changed, log it 
            if (new_position != previous_position){
                log_available = true;
                printf("DOWN\n");
            }   
    } else {
        if ((pitch>=-25) ||(roll>=0.0)){
            // printf("up \t");
            new_position= UP_POSITION; 
            // if the posiiton changed, log it 
            if (new_position != previous_position){
                log_available = true;
                printf("UP\n");

            }      
        }else{
            // printf("other \t");
            new_position = OTHER_POSITION;
        }
    }
    previous_position = new_position;
}

// timer event handler
static void timer_timeout_sensor_handler(void * p_context)
{
    //update sensor data 
    ICM20948_read();
   
    //store data in a queue
    ret_code_t err_code = nrf_queue_push(&m_ax_queue, &_ax);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_queue_push(&m_ay_queue, &_ay);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_queue_push(&m_az_queue, &_az);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_queue_push(&m_gx_queue, &_gx);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_queue_push(&m_gy_queue, &_gy);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_queue_push(&m_gz_queue, &_gz);
    APP_ERROR_CHECK(err_code);

    //update running average filter
    f_ax=moving_average(&m_ax_queue, _ax, &ax_accumulator);
    f_ay=moving_average(&m_ay_queue, _ay, &ay_accumulator);
    f_az=moving_average(&m_az_queue, _az, &az_accumulator);
    f_gx=moving_average(&m_gx_queue, _gx, &gx_accumulator);
    f_gy=moving_average(&m_gy_queue, _gy, &gy_accumulator);
    f_gz=moving_average(&m_gz_queue, _gz, &gz_accumulator);
    
    // feed filtered data to Madgwick filter
    Madgwick_updateIMU(f_gx,f_gy, f_gz, f_ax, f_ay, f_az);
    computeAngles();

    // toggle flag
    new_data_ready = true;
}

/**@brief Function for initializing the timer module.
 */
static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&imu_sample_timer_id, APP_TIMER_MODE_REPEATED, timer_timeout_sensor_handler);
    APP_ERROR_CHECK(err_code);

}

/**@brief Function for starting timers.
*/
static void application_timers_start(void)
{
    app_timer_start(imu_sample_timer_id, IMU_SAMPLE_TIMER_INTERVAL, NULL);
}


/**
 * @brief TWI events handler.
 */
void twi_handler(nrf_drv_twi_evt_t const* p_event, void* p_context)
{
    switch (p_event->type) {
    case NRF_DRV_TWI_EVT_DONE:
        if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_RX) {
            //things you want to do with the data
        }
        m_xfer_done = true;
        break;
    default:
        break;
    }
}

void twi_init()
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_icm20948_config = {
        .scl = SCL_PIN,
        .sda = SDA_PIN,
        .frequency = NRF_DRV_TWI_FREQ_100K,
        .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
        .clear_bus_init = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_icm20948_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

/*
* Intermediary function to read registers
*/
ret_code_t read_register(nrf_drv_twi_t twi_instance, uint8_t device_addr, uint8_t register_addr, uint8_t* p_data, uint8_t bytes, bool no_stop)
{
    ret_code_t err_code;

    err_code = nrf_drv_twi_tx(&twi_instance, device_addr, &register_addr, 1, no_stop);
    APP_ERROR_CHECK(err_code);

    if (err_code != NRF_SUCCESS) {
        return err_code;
    }

    err_code = nrf_drv_twi_rx(&twi_instance, device_addr, p_data, bytes);
    return err_code;
}

/*@brief Function to read i2C sensor registers 
 * reads registers from ICM20948 given a starting register address, number of bytes, and a pointer to store data
*/
void readRegisters(uint8_t subAddress, uint8_t count, uint8_t *dest){
    ret_code_t err_code = read_register(m_twi, ICM20948_ADDR, subAddress, dest, count, true);
    APP_ERROR_CHECK(err_code);
}


/* writes a byte to ICM20948 register given a register address and data */
void  writeRegister(uint8_t subAddress, uint8_t data)
{
    /* write data to device */
    ret_code_t err_code;
    uint8_t reg[2] = {subAddress, data};
    err_code = nrf_drv_twi_tx(&m_twi, ICM20948_ADDR, reg, sizeof(reg), false);
    nrf_delay_ms(1);
    APP_ERROR_CHECK(err_code);
    // while (m_xfer_done == false);
}

/**@brief  Function for initializing the UART module.
 */
/**@snippet [UART Initialization] */
static void uart_init(void)
{
    uint32_t                     err_code;
    app_uart_comm_params_t const comm_params =
    {
        .rx_pin_no    = RX_PIN_NUMBER,
        .tx_pin_no    = TX_PIN_NUMBER,
        .rts_pin_no   = RTS_PIN_NUMBER,
        .cts_pin_no   = CTS_PIN_NUMBER,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity   = false,
#if defined (UART_PRESENT)
        .baud_rate    = NRF_UART_BAUDRATE_115200
#else
        .baud_rate    = NRF_UARTE_BAUDRATE_115200
#endif
    };

    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_event_handle,
                       APP_IRQ_PRIORITY_LOWEST,
                       err_code);
    APP_ERROR_CHECK(err_code);
}
/**@snippet [UART Initialization] */


/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
// static void idle_state_handle(void)
// {
//     if (NRF_LOG_PROCESS() == false)
//     {
//         nrf_pwr_mgmt_run();
//     }
// }

static void rgb_led_init(){
    nrf_gpio_cfg_output (LED_G);
    nrf_gpio_cfg_output (LED_R);
    nrf_gpio_cfg_output (LED_B);

    nrf_gpio_pin_set(LED_G);
    nrf_gpio_pin_set(LED_R);
    nrf_gpio_pin_set(LED_B);
}

/**@brief Application main function.
 */
int main(void)
{
    // Initialize.
    uart_init();
    log_init();
    timers_init();
    application_timers_start();
    twi_init();

    power_management_init();
    crimson_ble_init();
    rgb_led_init();

    advertising_start();
    // Start execution.
    printf("\r\nUART started.\r\n");
    NRF_LOG_INFO("Debug logging for UART over RTT started.");
       
    int val = ICM20948_init();
    if (val==0){
        printf("sensor OK \n");
    }

    sdcard_init();
    printf("yo\r\n");
    calibrate();
    
    
    Madgwick_init();
    Madgwick_setfreq(10.0); // frequency is 10Hz
    
    // Enter main loop.
    while (1)
    {
        if (new_data_ready){
            
            angles[0]= getRoll();
            angles[1]= getPitch();
            angles[2]= getYaw();
            // printf("gx: %f, gy: %f, gz: %f,  \n", _gx, _gy, _gz);
            // printf("-10, %f,  %f, %f, %f,  %f, %f, 10\n", _gx, _gy, _gz, f_gx, f_gy, f_gz);
            printf("Roll: %f , Pitch: %f , Yaw: %f \n ", angles[0], angles [1], angles[2]);
           
            process_position(angles[0], angles[1], angles[2]);
            new_data_ready=false; 
        }
        // if (log_available){
        //     if (previous_position == UP_POSITION){
        //     sdcard_sensor_update_data( event_time, 1);
        //     }
        //     if (previous_position == DOWN_POSITION){
        //     sdcard_sensor_update_data( event_time,  0);
        //     }
        //     log_available = false;
        // }
        
        nrf_gpio_pin_clear(LED_R);
        nrf_delay_ms(500);
        nrf_gpio_pin_set(LED_R);
        nrf_delay_ms(500);

        // sdcard_sensor_update_data(event_time, event_type)
        // idle_state_handle();
    }
}
