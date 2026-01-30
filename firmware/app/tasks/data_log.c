/*
 * data_log.c
 * 
 * Copyright The OBDH 2.0 Contributors.
 * 
 * This file is part of OBDH 2.0.
 * 
 * OBDH 2.0 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * OBDH 2.0 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with OBDH 2.0. If not, see <http:/\/www.gnu.org/licenses/>.
 * 
 */

/**
 * \brief Data log task implementation.
 * 
 * \author Gabriel Mariano Marcelino <gabriel.mm8@gmail.com>
 * \author Carlos Augusto Porto Freitas <carlos.portof@hotmail.com>
 * 
 * \version 1.0.0
 * 
 * \date 2021/05/24
 * 
 * \addtogroup data_log
 * \{
 */

#include <stdint.h>
#include <string.h>

#include <system/sys_log/sys_log.h>
#include <devices/media/media.h>
#include <structs/satellite.h>
#include <utils/mem_mng.h>

#include "data_log.h"
#include "startup.h"
#include "lfs.h"
#include "drivers/mt25q/mt25q.h"
#include <app/structs/satellite.h>
#include "semphr.h"

xTaskHandle xTaskDataLogHandle;
SemaphoreHandle_t xSemaphore_File_System_Access = NULL;
// Read a region in a block. Negative error codes are propagated
// to the user.
int _flash_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size){
    return (mt25q_read((block * c->block_size + off), (uint8_t *)buffer, size));
}
// Program a region in a block. The block must have previously
// been erased. Negative error codes are propagated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
int _flash_write(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size){
    return (mt25q_write((block * c->block_size + off), (uint8_t *)buffer, size));
}
// Erase a block. A block must be erased before being programmed.
// The state of an erased block is undefined. Negative error codes
// are propagated to the user.
// May return LFS_ERR_CORRUPT if the block should be considered bad.
int _flash_erase(const struct lfs_config *c, lfs_block_t block){
    return (mt25q_4K_sector_erase((block * c->block_size)));
}
// Sync the state of the underlying block device. Negative error codes
// are propagated to the user.
int _flash_sync(const struct lfs_config *c){
    (void)c;
    return 0;
}
    
// variables used by the filesystem
lfs_t lfs;
lfs_file_t file;
const struct lfs_config cfg = {
    // block device operations
    .read  = _flash_read,
    .prog  = _flash_write,
    .erase = _flash_erase,
    .sync  = _flash_sync,

    // block device configuration
    .read_size = 32,
    .prog_size = 32,
    .block_size = 4096,
    .block_count = 32768,
    .cache_size = 32,
    .lookahead_size = 8,
    .block_cycles = 500,
};


/**
 * @brief Data log task main loop.
 *
 * This FreeRTOS task is responsible for initializing the LittleFS
 * filesystem on NOR flash, maintaining the boot counter file and
 * periodically saving telemetry data to flash.
 *
 * @param p Unused task parameter (pointer passed by FreeRTOS at task create).
 */
void vTaskDataLog(void *p)
{
    (void)p;
     /* Wait startup task to finish */
    (void)xEventGroupWaitBits(task_startup_status, TASK_STARTUP_DONE, pdFALSE, pdTRUE, pdMS_TO_TICKS(TASK_DATA_LOG_INIT_TIMEOUT_MS));

    /* Wait 5 minutes before saving data for the first time */
    // vTaskDelay(pdMS_TO_TICKS(TASK_DATA_LOG_INITIAL_DELAY_MS));


    media_info_t nor_info = media_get_info(MEDIA_NOR);

    // uint8_t page_buf[256] = {0};

    int err = lfs_mount(&lfs, &cfg);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err) {
        sys_log_print_event_from_module(SYS_LOG_INFO, TASK_DATA_LOG_NAME, "Error mounting NOR to littlefs. Formatting...");
        sys_log_new_line();
        err = lfs_format(&lfs, &cfg);
        if(err == 0){
            err = lfs_mount(&lfs, &cfg);
        }else{
            sys_log_print_event_from_module(SYS_LOG_INFO, TASK_DATA_LOG_NAME, "Error formatting NOR to littlefs...");
            sys_log_new_line();
        }        
    }


    if(xTaskDataLog_Initialize_Cimatelite_Log() != DATA_LOG_CIMATELITE_OK){
        /* Print the error */
        sys_log_print_event_from_module(SYS_LOG_INFO, TASK_DATA_LOG_NAME, "Error initializing CIMATELITE log file...");
        sys_log_new_line();

    }

    TickType_t last_cycle = xTaskGetTickCount();

    while(1)
    {
        sys_log_print_event_from_module(SYS_LOG_INFO, TASK_DATA_LOG_NAME, "Saving data to flash memory...");
        sys_log_new_line();

        /* Update OBDH timestamp atomically */
        taskENTER_CRITICAL();
        sat_data_buf.obdh.timestamp = system_get_time();
        taskEXIT_CRITICAL();

        // /* OBDH data */
        // (void)memcpy(&page_buf[0], (void*)&sat_data_buf.obdh, sizeof(obdh_telemetry_t));
        // if (mem_mng_write_data_to_flash_page(page_buf, &sat_data_buf.obdh.data.media.last_page_obdh_data, nor_info.page_size, CONFIG_MEM_OBDH_DATA_START_PAGE, CONFIG_MEM_OBDH_DATA_END_PAGE) == 0)
        // {
        //     sys_log_print_event_from_module(SYS_LOG_INFO, TASK_DATA_LOG_NAME, "Writing to OBDH sector, flash page number: ");
        //     sys_log_print_hex(sat_data_buf.obdh.data.media.last_page_obdh_data);
        //     sys_log_new_line();
        // }
        // else
        // {
        //     sys_log_print_event_from_module(SYS_LOG_ERROR, TASK_DATA_LOG_NAME, "Error writing the OBDH data to the flash memory!");
        //     sys_log_new_line();
        // }

        // (void)memset(&page_buf[0], 0, 256);

        // /* EPS data */
        // (void)memcpy(&page_buf[0], (void*)&sat_data_buf.eps, sizeof(eps_telemetry_t));
        // if (mem_mng_write_data_to_flash_page(page_buf, &sat_data_buf.obdh.data.media.last_page_eps_data, nor_info.page_size, CONFIG_MEM_EPS_DATA_START_PAGE, CONFIG_MEM_EPS_DATA_END_PAGE) == 0)
        // {
        //     sys_log_print_event_from_module(SYS_LOG_INFO, TASK_DATA_LOG_NAME, "Writing to EPS sector, flash page number: ");
        //     sys_log_print_hex(sat_data_buf.obdh.data.media.last_page_eps_data);
        //     sys_log_new_line();
        // }
        // else
        // {
        //     sys_log_print_event_from_module(SYS_LOG_ERROR, TASK_DATA_LOG_NAME, "Error writing the EPS data to the flash memory!");
        //     sys_log_new_line();
        // }

        // (void)memset(&page_buf[0], 0, 256);

        // /* TTC 0 data */
        // (void)memcpy(&page_buf[0], (void*)&sat_data_buf.ttc_0, sizeof(ttc_telemetry_t));
        // if (mem_mng_write_data_to_flash_page(page_buf, &sat_data_buf.obdh.data.media.last_page_ttc_0_data, nor_info.page_size, CONFIG_MEM_TTC_0_DATA_START_PAGE, CONFIG_MEM_TTC_0_DATA_END_PAGE) == 0)
        // {
        //     sys_log_print_event_from_module(SYS_LOG_INFO, TASK_DATA_LOG_NAME, "Writing to TTC 0 sector, flash page number: ");
        //     sys_log_print_hex(sat_data_buf.obdh.data.media.last_page_ttc_0_data);
        //     sys_log_new_line();
        // }
        // else
        // {
        //     sys_log_print_event_from_module(SYS_LOG_ERROR, TASK_DATA_LOG_NAME, "Error writing the TTC 0 data to the flash memory!");
        //     sys_log_new_line();
        // }

        // (void)memset(&page_buf[0], 0, 256);

        // /* TTC 1 data */
        // (void)memcpy(&page_buf[0], (void*)&sat_data_buf.ttc_1, sizeof(ttc_telemetry_t));
        // if (mem_mng_write_data_to_flash_page(page_buf, &sat_data_buf.obdh.data.media.last_page_ttc_1_data, nor_info.page_size, CONFIG_MEM_TTC_1_DATA_START_PAGE, CONFIG_MEM_TTC_1_DATA_END_PAGE) == 0)
        // {
        //     sys_log_print_event_from_module(SYS_LOG_INFO, TASK_DATA_LOG_NAME, "Writing to TTC 1 sector, flash page number: ");
        //     sys_log_print_hex(sat_data_buf.obdh.data.media.last_page_ttc_1_data);
        //     sys_log_new_line();
        // }
        // else
        // {
        //     sys_log_print_event_from_module(SYS_LOG_ERROR, TASK_DATA_LOG_NAME, "Error writing the TTC 1 data to the flash emory!");
        //     sys_log_new_line();
        // }

        // (void)memset(&page_buf[0], 0, 256);

        // /* Antenna data */
        // (void)memcpy(&page_buf[0], (void*)&sat_data_buf.antenna, sizeof(antenna_telemetry_t));
        // if (mem_mng_write_data_to_flash_page(page_buf, &sat_data_buf.obdh.data.media.last_page_ant_data, nor_info.page_size, CONFIG_MEM_ANT_DATA_START_PAGE, CONFIG_MEM_ANT_DATA_END_PAGE) == 0)
        // {
        //     sys_log_print_event_from_module(SYS_LOG_INFO, TASK_DATA_LOG_NAME, "Writing to Antenna sector, flash page number: ");
        //     sys_log_print_hex(sat_data_buf.obdh.data.media.last_page_ant_data);
        //     sys_log_new_line();
        // }
        // else
        // {
        //     sys_log_print_event_from_module(SYS_LOG_ERROR, TASK_DATA_LOG_NAME, "Error writing the antenna data to the flash memory!");
        //     sys_log_new_line();
        // }

        // (void)memset(&page_buf[0], 0, 256);

        // /* EDC data */
        // (void)memcpy(&page_buf[0], (void*)sat_data_buf.state.c_edc, sizeof(payload_telemetry_t));
        // if (mem_mng_write_data_to_flash_page(page_buf, &sat_data_buf.obdh.data.media.last_page_edc_data, nor_info.page_size, CONFIG_MEM_EDC_DATA_START_PAGE, CONFIG_MEM_EDC_DATA_END_PAGE) == 0)
        // {
        //     sys_log_print_event_from_module(SYS_LOG_INFO, TASK_DATA_LOG_NAME, "Writing to EDC sector, flash page number: ");
        //     sys_log_print_hex(sat_data_buf.obdh.data.media.last_page_edc_data);
        //     sys_log_new_line();
        // }
        // else
        // {
        //     sys_log_print_event_from_module(SYS_LOG_ERROR, TASK_DATA_LOG_NAME, "Error writing the EDC data to the flash memory!");
        //     sys_log_new_line();
        // }

        // (void)memset(&page_buf[0], 0, 256);

        // /* Payload-X data */
        // (void)memcpy(&page_buf[0], (void*)&sat_data_buf.payload_x, sizeof(payload_telemetry_t));
        // if (mem_mng_write_data_to_flash_page(page_buf, &sat_data_buf.obdh.data.media.last_page_px_data, nor_info.page_size, CONFIG_MEM_PX_DATA_START_PAGE, CONFIG_MEM_PX_DATA_END_PAGE) == 0)
        // {
        //     sys_log_print_event_from_module(SYS_LOG_INFO, TASK_DATA_LOG_NAME, "Writing to Payload X sector, flash page number: ");
        //     sys_log_print_hex(sat_data_buf.obdh.data.media.last_page_px_data);
        //     sys_log_new_line();
        // }
        // else
        // {
        //     sys_log_print_event_from_module(SYS_LOG_ERROR, TASK_DATA_LOG_NAME, "Error writing the Payload-X data to the flash memory!");
        //     sys_log_new_line();
        // }

        // (void)memset(&page_buf[0], 0, 256);

        // /* Plinio Data */
        // (void)memcpy(&page_buf[0], &sat_data_buf.cimatelite, sizeof(cimatelite_telemetry_t));
        // if (mem_mng_write_data_to_flash_page(page_buf, &sat_data_buf.obdh.data.media.last_page_cimatelite_data, nor_info.page_size, CONFIG_MEM_CIMATELITE_DATA_START_PAGE, CONFIG_MEM_CIMATELITE_DATA_END_PAGE) == 0)
        // {
        //     sys_log_print_event_from_module(SYS_LOG_INFO, TASK_DATA_LOG_NAME, "Writing to Cimatelite X sector, flash page number: ");
        //     sys_log_print_hex(sat_data_buf.obdh.data.media.last_page_cimatelite_data);
        //     sys_log_new_line();
        // }
        // else
        // {
        //     sys_log_print_event_from_module(SYS_LOG_ERROR, TASK_DATA_LOG_NAME, "Error writing the Payload-X data to the flash memory!");
        //     sys_log_new_line();
        // }

        // (void)memset(&page_buf[0], 0, 256);
        // /* END: Plinio Data */


        vTaskDelayUntil(&last_cycle, pdMS_TO_TICKS(TASK_DATA_LOG_PERIOD_MS));
    }
}




/**
 * @brief Initialize the Cimatelite log file in the filesystem.
 *
 * Ensures the log file named by `DATA_LOG_CIMATELITE_FILE_NAME` exists.
 * If the file cannot be created or opened, an error code is returned.
 *
 * @return DATA_LOG_CIMATELITE_OK on success
 * @return DATA_LOG_CIMATELITE_ERROR on failure
 */
data_log_err_t  xTaskDataLog_Initialize_Cimatelite_Log(void){
    int err = 0;
    lfs_file_t File_Handler;
    /* Initialize Geodesic LOG File */
    err = lfs_file_open(&lfs, &File_Handler, (char *)DATA_LOG_CIMATELITE_FILE_NAME, LFS_O_RDWR | LFS_O_CREAT);
    if (err != 0)
        return DATA_LOG_CIMATELITE_ERROR;
    lfs_file_close(&lfs, &File_Handler);

    return DATA_LOG_CIMATELITE_OK;
}

/**
 * @brief Get the current size (in bytes) of the Cimatelite log file.
 *
 * This function reads the size of the file identified by
 * `DATA_LOG_CIMATELITE_FILE_NAME` and places the result in `log_size`.
 *
 * @param[out] log_size Pointer to a uint32_t that will receive the file size in bytes.
 *
 * @return DATA_LOG_CIMATELITE_OK on success and `log_size` is updated
 * @return DATA_LOG_CIMATELITE_ERROR if an error occurred (e.g., filesystem not initialized or file missing)
 */
data_log_err_t  xTaskDataLog_Get_Cimatelite_Log_Size(uint32_t *log_size){
    int err = 0;
    lfs_file_t File_Handler;

    err = lfs_file_open(&lfs, &File_Handler, (char *)DATA_LOG_CIMATELITE_FILE_NAME, LFS_O_RDONLY);
    if (err < 0)
        return DATA_LOG_CIMATELITE_ERROR;

    *log_size = lfs_file_size(&lfs, &File_Handler) / sizeof(cimatelite_telemetry_t);
    lfs_file_close(&lfs, &File_Handler);

    return DATA_LOG_CIMATELITE_OK;
}

/**
 * @brief Append a Cimatelite telemetry record to the Cimatelite log file.
 *
 * The function should open the log file, append the provided `log_data`
 * structure and close the file. The exact binary layout written depends on
 * the definition of `cimatelite_telemetry_t`.
 *
 * @param[in] log_data Telemetry structure to append to the log.
 *
 * @return DATA_LOG_CIMATELITE_OK on success
 * @return DATA_LOG_CIMATELITE_ERROR on failure
 */
data_log_err_t  xTaskDataLog_Insert_Cimatelite_Log_Data(cimatelite_telemetry_t log_data){
  int fs_err;
  lfs_file_t File_Handler;


  xSemaphoreTake(xSemaphore_File_System_Access, portMAX_DELAY);
  fs_err = lfs_file_open(&lfs, &File_Handler, (char *)DATA_LOG_CIMATELITE_FILE_NAME, LFS_O_APPEND | LFS_O_WRONLY);
  if (fs_err < 0) {
    xSemaphoreGive(xSemaphore_File_System_Access);
    return DATA_LOG_CIMATELITE_ERROR;
  }

  fs_err = lfs_file_write(&lfs, &File_Handler, (void *)&log_data, sizeof(cimatelite_telemetry_t));
  if (fs_err != sizeof(cimatelite_telemetry_t)) {
    /* File write size is not equal to the file size */
    goto insert_data_error;
  }

  lfs_file_close(&lfs, &File_Handler);
  xSemaphoreGive(xSemaphore_File_System_Access);
  return DATA_LOG_CIMATELITE_OK;
insert_data_error:
  lfs_file_close(&lfs, &File_Handler);
  xSemaphoreGive(xSemaphore_File_System_Access);
  return DATA_LOG_CIMATELITE_ERROR;
}

/**
 * @brief Clear (truncate/recreate) the Cimatelite log file.
 *
 * Removes the existing log file and recreates it empty. This function
 * acquires the filesystem semaphore while operating to ensure exclusive
 * access.
 *
 * @return DATA_LOG_CIMATELITE_OK on success
 * @return LIB_LOG_ERROR on failure
 */
data_log_err_t  xTaskDataLog_Clear_Cimatelite_Log(void){
    int fs_err;
    lfs_file_t File_Handler;

    xSemaphoreTake(xSemaphore_File_System_Access, portMAX_DELAY);
    fs_err = lfs_remove(&lfs, (char *)DATA_LOG_CIMATELITE_FILE_NAME);
    if (fs_err < 0) {
        xSemaphoreGive(xSemaphore_File_System_Access);
        return DATA_LOG_CIMATELITE_ERROR;
    }

    fs_err = lfs_file_open(&lfs, &File_Handler, (char *)DATA_LOG_CIMATELITE_FILE_NAME, LFS_O_RDWR | LFS_O_CREAT);
    if (fs_err < 0) {
        xSemaphoreGive(xSemaphore_File_System_Access);                                                                                                                       xSemaphoreGive(xSemaphore_File_System_Access);
        return DATA_LOG_CIMATELITE_ERROR;
    }

    lfs_file_close(&lfs, &File_Handler);

    xSemaphoreGive(xSemaphore_File_System_Access);
    return DATA_LOG_CIMATELITE_OK;
}

/**
 * \brief Reads data from the Cimatelite log file.
 *
 * \param log_data Pointer to the buffer where the read data will be stored.
 * \param start_index The index of the first element to read.
 *
 * \return DATA_LOG_CIMATELITE_OK if successful, otherwise DATA_LOG_CIMATELITE_ERROR.
 */
data_log_err_t  xTaskDataLog_Read_Cimatelite_Log(cimatelite_telemetry_t *log_data, uint32_t start_index){
    int fs_err;
    lfs_file_t File_Handler;
    uint32_t offset_bytes;
    uint32_t log_size;
    
    offset_bytes = start_index * sizeof(cimatelite_telemetry_t);

    xSemaphoreTake(xSemaphore_File_System_Access, portMAX_DELAY);
    fs_err = lfs_file_open(&lfs, &File_Handler, (char *)DATA_LOG_CIMATELITE_FILE_NAME, LFS_O_RDONLY);
    if (fs_err < 0) {
        xSemaphoreGive(xSemaphore_File_System_Access);
        return DATA_LOG_CIMATELITE_ERROR;
    }

    log_size = lfs_file_size(&lfs, &File_Handler) / sizeof(cimatelite_telemetry_t);
    if(start_index >= log_size){
        lfs_file_close(&lfs, &File_Handler);
        xSemaphoreGive(xSemaphore_File_System_Access);
        return DATA_LOG_CIMATELITE_ERROR;
    }

    fs_err = lfs_file_seek(&lfs, &File_Handler, offset_bytes, LFS_SEEK_SET);
    if (fs_err < 0) {
        xSemaphoreGive(xSemaphore_File_System_Access);
        return DATA_LOG_CIMATELITE_ERROR;
    }

    fs_err = lfs_file_read(&lfs, &File_Handler, (void *)log_data, sizeof(cimatelite_telemetry_t));
    if (fs_err != sizeof(cimatelite_telemetry_t)) {
        xSemaphoreGive(xSemaphore_File_System_Access);
        return DATA_LOG_CIMATELITE_ERROR;
    }

    lfs_file_close(&lfs, &File_Handler);
    xSemaphoreGive(xSemaphore_File_System_Access);
    return DATA_LOG_CIMATELITE_OK;   
}
/** \} End of file_system group */
