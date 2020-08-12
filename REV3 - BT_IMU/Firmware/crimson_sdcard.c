/**
handling read/write to the sdcard
 */

#include <stdio.h>
#include "crimson_sdcard.h"

/**
 * @brief SDC block device definition
 */
NRF_BLOCK_DEV_SDC_DEFINE(
        m_block_dev_sdc,
        NRF_BLOCK_DEV_SDC_CONFIG(
                SDC_SECTOR_SIZE,
                APP_SDCARD_CONFIG(SDC_MOSI_PIN, SDC_MISO_PIN, SDC_SCK_PIN, SDC_CS_PIN)
         ),
         NFR_BLOCK_DEV_INFO_CONFIG("Crimson001", "SDC", "1.00")
);

static volatile bool heading = true;
unsigned char str[100];
static FATFS fs;
static DIR dir;
static FILINFO fno;
static FIL file;

uint32_t bytes_written;
FRESULT ff_result;
DSTATUS disk_state = STA_NOINIT;

/**
 * @brief function to initialize SD card.
 */
void sdcard_init(){
    
     // Initialize FATFS disk I/O interface by providing the block device.
    static diskio_blkdev_t drives[] =
    {
            DISKIO_BLOCKDEV_CONFIG(NRF_BLOCKDEV_BASE_ADDR(m_block_dev_sdc, block_dev), NULL)
    };

    diskio_blockdev_register(drives, ARRAY_SIZE(drives));

    printf("Initializing disk 0 (SDC)...\r\n");
    for (uint32_t retries = 3; retries && disk_state; --retries)
    {
        disk_state = disk_initialize(0);
    }
    if (disk_state)
    {
        printf("Disk initialization failed.\r\n");
        return;
    }

    uint32_t blocks_per_mb = (1024uL * 1024uL) / m_block_dev_sdc.block_dev.p_ops->geometry(&m_block_dev_sdc.block_dev)->blk_size;
    uint32_t capacity = m_block_dev_sdc.block_dev.p_ops->geometry(&m_block_dev_sdc.block_dev)->blk_count / blocks_per_mb;
    printf("Capacity: %ld MB\r\n", capacity);

    printf("Mounting volume...\r\n");
    ff_result = f_mount(&fs, "", 1);
    if (ff_result)
    {
        printf("Mount failed.\r\n");
        return;
    }

    printf("\r\n Listing directory: /\r\n");
    ff_result = f_opendir(&dir, "/");
    if (ff_result)
    {
        printf("Directory listing failed!\r\n");
        return;
    }

    do
    {
        ff_result = f_readdir(&dir, &fno);
        if (ff_result != FR_OK)
        {
            printf("Directory read failed.");
            return;
        }

        if (fno.fname[0])
        {
            if (fno.fattrib & AM_DIR)
            {
                printf("   <DIR>   %ld\r\n",(uint32_t)fno.fname);
            }
            else
            {
                printf("%9lu  %ld\r\n", fno.fsize, (uint32_t)fno.fname);
            }
        }
    }
    while (fno.fname[0]);
    printf("\r\n");


}


/**
 * @brief function to log data in SD card.
 */
void sdcard_sensor_update_data(char *event_time, uint8_t event_type)
{
    printf("Writing to file " FILE_NAME "...\r\n");
    ff_result = f_open(&file, FILE_NAME, FA_READ | FA_WRITE | FA_OPEN_APPEND);
    if (ff_result != FR_OK)
    {
        printf("Unable to open or create file: " FILE_NAME ".\r\n");
        return;
    }

    if(heading) {
      ff_result = f_write(&file, TEST_STRING, sizeof(TEST_STRING) - 1, (UINT *) &bytes_written);
      heading = false;
    }


    sprintf((char *)str, "Time %s \t Event type = %d \r\n", event_time, (uint8_t)event_type);

    ff_result = f_write(&file, str, sizeof(str) - 1, (UINT *) &bytes_written);

    if (ff_result != FR_OK)
    {
        printf("Write failed\r\n.");
    }
    else
    {
        printf("%ld bytes written.\r\n", bytes_written);
    }

    // flush cached information
    ff_result = f_sync(&file);

    (void) f_close(&file);
    return;
}
