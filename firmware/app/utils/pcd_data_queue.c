/*
 * pcd_data_queue.c
 *
 *  Created on: 20 de out de 2025
 *      Author: ramon.papes
 */

#include "pcd_data_queue.h"

int PCD_Queue_Init()
{
    int err = 0;
    const uint16_t queue_size = sizeof(cimatelite_telemetry_t);
    sat_data_buf.xPCDQueue = xQueueCreate(QUEUE_SIZE, queue_size);

    if (sat_data_buf.xPCDQueue == NULL)
    {
        err = -1;
    }

    return err;
}

int PCD_SendToQueue(cimatelite_telemetry_t *data)
{
    int err = 0;

    if (xQueueCRSend(sat_data_buf.xPCDQueue, data, pdMS_TO_TICKS(100)) != pdPASS)
    {
        err = -1;
    }
    return err;
}

int PCD_ReceiveFromQueue(cimatelite_telemetry_t *data)
{
    int err = 0;
    if (xQueueCRReceive(sat_data_buf.xPCDQueue, data,
                        pdMS_TO_TICKS(100)) != pdPASS)
    {
        err = -1;
    }
    return err;
}

int PCD_QueueIsFull(void)
{
    if (sat_data_buf.xPCDQueue == NULL)
        return -1;
    return (uxQueueSpacesAvailable(sat_data_buf.xPCDQueue) == 0);
}

int PCD_QueueIsEmpty(void)
{
    if (sat_data_buf.xPCDQueue == NULL)
        return -1;
    return (uxQueueMessagesWaiting(sat_data_buf.xPCDQueue) == 0);
}

int PCD_QueueGetCount(void)
{
    if (sat_data_buf.xPCDQueue == NULL)
        return -1;
    return uxQueueMessagesWaiting(sat_data_buf.xPCDQueue);
}

int PCD_QueueGetFreeSlots(void)
{
    if (sat_data_buf.xPCDQueue == NULL)
        return -1;
    return uxQueueSpacesAvailable(sat_data_buf.xPCDQueue);
}

int PCD_QueueReset(void)
{
    int err = 0;

    if (sat_data_buf.xPCDQueue == NULL)
        err = -1;
    xQueueReset(sat_data_buf.xPCDQueue);
    return err;
}
