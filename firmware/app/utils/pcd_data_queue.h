/*
 * pcd_data_queue.h
 *
 *  Created on: 20 de out de 2025
 *      Author: ramon.papes
 */

#ifndef APP_UTILS_PCD_DATA_QUEUE_H_
#define APP_UTILS_PCD_DATA_QUEUE_H_

#include "FreeRTOS.h"
#include "queue.h"
#include <stdint.h>
#include <structs/satellite.h>

#define QUEUE_SIZE 10

int PCD_Queue_Init();
int PCD_SendToQueue(cimatelite_telemetry_t *data);
int PCD_ReceiveFromQueue(cimatelite_telemetry_t *data);
int PCD_QueueIsFull(void);
int PCD_QueueIsEmpty(void);
int PCD_QueueGetCount(void);
int PCD_QueueGetFreeSlots(void);
int PCD_QueueReset(void);


#endif /* APP_UTILS_PCD_DATA_QUEUE_H_ */
