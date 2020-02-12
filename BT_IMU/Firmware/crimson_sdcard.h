#ifndef SD_CARD_
#define SD_CARD_
/**
 *
 * nRF52 SD card interface to log data.
 *
 * Electronut Labs
 * electronut.in
 */
#include "nrf.h"
#include "bsp.h"
#include "ff.h"
#include "diskio_blkdev.h"
#include "nrf_block_dev_sdc.h"
#include "app_sdcard.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define FILE_NAME   "CRIMSON.TXT"
#define TEST_STRING "Sensor Data Logger Example.\r\n\n"
#define TEST_STRING_2 "TEST.\r\n\n"


/**
 * @brief function to log data in SD card.
 */
void sdcard_init();
void sdcard_sensor_update_data(float event_time, uint8_t event_type);

#endif //SD_CARD_