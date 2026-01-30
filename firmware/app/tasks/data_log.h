/*
 * data_log.h
 * 
 * Copyright (C) 2021, SpaceLab.
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
 * along with OBDH 2.0. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

/**
 * \brief Data log task definition.
 * 
 * \author Gabriel Mariano Marcelino <gabriel.mm8@gmail.com>
 * \author Carlos Augusto Porto Freitas <carlos.portof@hotmail.com>
 * 
 * \version 1.0.0
 * 
 * \date 2021/05/24
 * 
 * \defgroup data_log Data Log
 * \ingroup tasks
 * \{
 */

#ifndef DATA_LOG_H_
#define DATA_LOG_H_

#include <FreeRTOS.h>
#include <task.h>
#include <app/structs/satellite.h>

#define TASK_DATA_LOG_NAME                      "Data Log"          /**< Task name. */
#define TASK_DATA_LOG_STACK_SIZE                512                 /**< Stack size in bytes. */
#define TASK_DATA_LOG_PRIORITY                  3                   /**< Task priority. */
#define TASK_DATA_LOG_PERIOD_MS                 (600000UL)          /**< Task period in milliseconds. */
#define TASK_DATA_LOG_INITIAL_DELAY_MS          (300000UL)          /**< Delay, in milliseconds, before the first execution. */
#define TASK_DATA_LOG_INIT_TIMEOUT_MS           5000                /**< Wait time to initialize the task in milliseconds. */

#define DATA_LOG_CIMATELITE_FILE_NAME           "cimatelite_log"
/**
 * \brief Data IDs.
 */
typedef enum
{
    DATA_LOG_CIMATELITE_OK = 0,
    DATA_LOG_CIMATELITE_ERROR = 0,
} data_log_err_t;

/**
 * \brief Data IDs.
 */
typedef enum
{
    DATA_LOG_HK_DATA_ID = 1,
} data_log_id_t;

/**
 * \brief Data log handle.
 */
extern xTaskHandle xTaskDataLogHandle;

/**
 * \brief Data log task.
 *
 * \return None.
 */
void vTaskDataLog(void* p);


/**
 * @brief Initialize the Cimatelite log file in the filesystem.
 *
 * Ensures the log file named by `DATA_LOG_CIMATELITE_FILE_NAME` exists.
 * If the file cannot be created or opened, an error code is returned.
 *
 * @return DATA_LOG_CIMATELITE_OK on success
 * @return DATA_LOG_CIMATELITE_ERROR on failure
 */
data_log_err_t  xTaskDataLog_Initialize_Cimatelite_Log(void);


/**
 * @brief Get the current size (in entries) of the Cimatelite log file.
 *
 * This function reads the size of the file identified by
 * `DATA_LOG_CIMATELITE_FILE_NAME` and places the result in `log_size`.
 *
 * @param[out] log_size Pointer to a uint32_t that will receive the file size in bytes.
 *
 * @return DATA_LOG_CIMATELITE_OK on success and `log_size` is updated
 * @return DATA_LOG_CIMATELITE_ERROR if an error occurred (e.g., filesystem not initialized or file missing)
 */
data_log_err_t  xTaskDataLog_Get_Cimatelite_Log_Size(uint32_t *log_size);

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
data_log_err_t  xTaskDataLog_Insert_Cimatelite_Log_Data(cimatelite_telemetry_t log_data);

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
data_log_err_t  xTaskDataLog_Clear_Cimatelite_Log(void);

/**
 * \brief Reads data from the Cimatelite log file.
 *
 * \param log_data Pointer to the buffer where the read data will be stored.
 * \param start_index The index of the first element to read.
 *
 * \return DATA_LOG_CIMATELITE_OK if successful, otherwise DATA_LOG_CIMATELITE_ERROR.
 */
data_log_err_t  xTaskDataLog_Read_Cimatelite_Log(cimatelite_telemetry_t *log_data, uint32_t start_index);
#endif /* DATA_LOG_H_ */

/** \} End of data_log group */
