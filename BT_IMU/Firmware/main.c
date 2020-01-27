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

#define ICM20948_ADDR 0x68

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */

volatile bool new_data_ready = false;
volatile bool m_xfer_done;
#define TWI_INSTANCE_ID     0
nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

#define OUR_SENSOR_TIMER_INTERVAL     APP_TIMER_TICKS(1000)     // 1000 ms intervals
APP_TIMER_DEF(m_our_sensor_timer_id);


float event_time = 32.1;
float event_type = 11;

int16_t accX, accY, accZ; // to hold before conversion
int16_t gyroX, gyroY, gyroZ;
float accel_X, accel_Y, accel_Z; // to hold hte data after conversion 
float gyro_X, gyro_Y, gyro_Z;

// timer event handler
static void timer_timeout_sensor_handler(void * p_context)
{
  // Acquire get sensor data
  // toggle flag
  new_data_ready = true;
  NRF_LOG_INFO("TIMER");
}


/**@brief Function for initializing the timer module.
 */
static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_our_sensor_timer_id, APP_TIMER_MODE_REPEATED, timer_timeout_sensor_handler);
    APP_ERROR_CHECK(err_code);

}

/**@brief Function for starting timers.
*/
static void application_timers_start(void)
{
  app_timer_start(m_our_sensor_timer_id, OUR_SENSOR_TIMER_INTERVAL, NULL);
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
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}

static void rgb_led_init(){
    nrf_gpio_cfg_output (LED_G);
    nrf_gpio_cfg_output (LED_R);
    nrf_gpio_cfg_output (LED_B);

    nrf_gpio_pin_set(LED_G);
    nrf_gpio_pin_set(LED_R);
    nrf_gpio_pin_set(LED_B);
}

// static void ping_address(uint8_t address){

//     uint8_t sample_data;
//     ret_code_t err_code = nrf_drv_twi_rx(&m_twi, address, &sample_data, sizeof(sample_data));
//         if (err_code == NRF_SUCCESS)
//         {
//             NRF_LOG_INFO("TWI device detected at address 0x%x.", address);
//         }

// }


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

    // Start execution.
    printf("\r\nUART started.\r\n");
    NRF_LOG_INFO("Debug logging for UART over RTT started.");
   
    advertising_start();

     int val = ICM20948_init();
    if (val==0){
        NRF_LOG_INFO("sensor OK");
    }
    
    // Enter main loop.
    while (1)
    {
        if (new_data_ready){
        nrf_gpio_pin_clear(LED_G);
        nrf_delay_ms(500);
        ICM20948_read();
        new_data_ready=false;
        }
        nrf_gpio_pin_set(LED_G);
        nrf_delay_ms(500);
        idle_state_handle();
    }
}
