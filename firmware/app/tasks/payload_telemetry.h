/*
 * general_telemetry.h
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
 * along with OBDH 2.0. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

/**
 * \brief Payload Telemetry task definition.
 * 
 * \author Gabriel Mariano Marcelino <gabriel.mm8@gmail.com>
 * \author Carlos Augusto Porto Freitas <carlos.portof@hotmail.com>
 * 
 * \version 1.0.0
 * 
 * \date 2019/10/27
 * 
 * \defgroup general_telemetry General Telemetry
 * \ingroup tasks
 * \{
 */

#ifndef PAYLOAD_TELEMETRY_H
#define PAYLOAD_TELEMETRY_H

#include <FreeRTOS.h>
#include <task.h>

#define TASK_PAYLOAD_TELEMETRY_NAME                    "Payload Telemetry" /**< Task name. */
#define TASK_PAYLOAD_TELEMETRY_STACK_SIZE              512                 /**< Stack size in bytes. */
#define TASK_PAYLOAD_TELEMETRY_PRIORITY                4                   /**< Task priority. */
#define TASK_PAYLOAD_TELEMETRY_PERIOD_MS               30000               /**< Task period in milliseconds. */
#define TASK_PAYLOAD_TELEMETRY_INITIAL_DELAY_MS        20000               /**< Delay, in milliseconds, before the first execution. */
#define TASK_PAYLOAD_TELEMETRY_INIT_TIMEOUT_MS         10000               /**< Wait time to initialize the task in milliseconds. */

/**
 * \brief General Telemetry handle.
 */
extern xTaskHandle xTaskPayloadTelemetryHandle;

/**
 * \brief Payload Telemetry task.
 *
 * \return None.
 */
void vTaskPayloadTelemetry(void* p);

#endif /* PAYLOAD_TELEMETRY_H_ */

/** \} End of Payload Telemetry group */
