/*
 * general_telemetry.c
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
 * \brief General Telemetry task implementation.
 * 
 * \author Gabriel Mariano Marcelino <gabriel.mm8@gmail.com>
 * \author Carlos Augusto Porto Freitas <carlos.portof@hotmail.com>
 * 
 * \version 1.0.0
 * 
 * \date 2019/10/27
 * 
 * \addtogroup general_telemetry
 * \{
 */

#include <config/config.h>
#include <fsat_pkt/fsat_pkt.h>

#include <system/sys_log/sys_log.h>
#include <system/system.h>

#include <structs/satellite.h>

#include <devices/ttc/ttc.h>

#include "payload_telemetry.h"
#include "startup.h"



xTaskHandle xTaskPayloadTelemetryHandle;

void vTaskPayloadTelemetry(void *p)
{
    (void) p;

    /* Wait startup task to finish */
    (void) xEventGroupWaitBits(
            task_startup_status, TASK_STARTUP_DONE, pdFALSE, pdTRUE,
            pdMS_TO_TICKS(TASK_PAYLOAD_TELEMETRY_INIT_TIMEOUT_MS));

    /* Delay before the first cycle */
    vTaskDelay(pdMS_TO_TICKS(TASK_PAYLOAD_TELEMETRY_INITIAL_DELAY_MS));

    TickType_t last_cycle = xTaskGetTickCount();

    uint8_t raw_pkt[256];

    while (1)
    {
        fsat_pkt_pl_t gen_tel_pl = { 0 };

        if (sat_data_buf.obdh.data.payload_telemetry_on) // cppcheck-suppress misra-c2012-14.4
        {
            /* Packet ID */
            fsat_pkt_add_id(&gen_tel_pl, PKT_ID_DOWNLINK_PAYLOAD_TELEMETRY);

            /* Source callsign */
            (void) fsat_pkt_add_callsign(&gen_tel_pl,
            CONFIG_SATELLITE_CALLSIGN);

            uint32_t timestamp = system_get_time();

            cimatelite_telemetry_t cimatelite_temelemetry;
            media_read(MEDIA_NOR, (sat_data_buf.obdh.data.media.last_page_cimatelite_data - 1) * PAGE_SIZE, &raw_pkt, sizeof(raw_pkt));
            memcpy(&cimatelite_temelemetry, &raw_pkt, sizeof(cimatelite_temelemetry));

            if (cimatelite_temelemetry.data.pkt_id > 0)
            {

                //timestamp
                gen_tel_pl.payload[0] = (cimatelite_temelemetry.timestamp
                        >> 24U) & 0xFFU;
                gen_tel_pl.payload[1] = (cimatelite_temelemetry.timestamp
                        >> 16U) & 0xFFU;
                gen_tel_pl.payload[2] =
                        (cimatelite_temelemetry.timestamp >> 8U) & 0xFFU;
                gen_tel_pl.payload[3] = cimatelite_temelemetry.timestamp
                        & 0xFFU;

                // pkt_id
                gen_tel_pl.payload[4] = (cimatelite_temelemetry.data.pkt_id
                        >> 24U) & 0xFFU;
                gen_tel_pl.payload[5] = (cimatelite_temelemetry.data.pkt_id
                        >> 16U) & 0xFFU;
                gen_tel_pl.payload[6] = (cimatelite_temelemetry.data.pkt_id
                        >> 8U) & 0xFFU;
                gen_tel_pl.payload[7] = cimatelite_temelemetry.data.pkt_id
                        & 0xFFU;

                // timestamp
                gen_tel_pl.payload[8] = (cimatelite_temelemetry.data.timestamp
                        >> 24U) & 0xFFU;
                gen_tel_pl.payload[9] = (cimatelite_temelemetry.data.timestamp
                        >> 16U) & 0xFFU;
                gen_tel_pl.payload[10] = (cimatelite_temelemetry.data.timestamp
                        >> 8U) & 0xFFU;
                gen_tel_pl.payload[11] = cimatelite_temelemetry.data.timestamp
                        & 0xFFU;

                // battery
                gen_tel_pl.payload[12] = (cimatelite_temelemetry.data.battery
                        >> 8U) & 0xFFU;
                gen_tel_pl.payload[13] = cimatelite_temelemetry.data.battery
                        & 0xFFU;

                // wind_speed (uint32_t por enquanto)
                gen_tel_pl.payload[14] =
                        (cimatelite_temelemetry.data.wind_speed >> 24U)
                                & 0xFFU;
                gen_tel_pl.payload[15] =
                        (cimatelite_temelemetry.data.wind_speed >> 16U)
                                & 0xFFU;
                gen_tel_pl.payload[16] =
                        (cimatelite_temelemetry.data.wind_speed >> 8U) & 0xFFU;
                gen_tel_pl.payload[17] = cimatelite_temelemetry.data.wind_speed
                        & 0xFFU;

                // wind_direction
                gen_tel_pl.payload[18] =
                        (cimatelite_temelemetry.data.wind_direction >> 8U)
                                & 0xFFU;
                gen_tel_pl.payload[19] =
                        cimatelite_temelemetry.data.wind_direction & 0xFFU;

                // rainfall (uint32_t por enquanto)
                gen_tel_pl.payload[20] = (cimatelite_temelemetry.data.rainfall
                        >> 24U) & 0xFFU;
                gen_tel_pl.payload[21] = (cimatelite_temelemetry.data.rainfall
                        >> 16U) & 0xFFU;
                gen_tel_pl.payload[22] = (cimatelite_temelemetry.data.rainfall
                        >> 8U) & 0xFFU;
                gen_tel_pl.payload[23] = cimatelite_temelemetry.data.rainfall
                        & 0xFFU;

                // ground_humidity
                gen_tel_pl.payload[24] =
                        (cimatelite_temelemetry.data.ground_humidity >> 8U)
                                & 0xFFU;
                gen_tel_pl.payload[25] =
                        cimatelite_temelemetry.data.ground_humidity & 0xFFU;

                // humidity
                gen_tel_pl.payload[26] = (cimatelite_temelemetry.data.humidity
                        >> 8U) & 0xFFU;
                gen_tel_pl.payload[27] = cimatelite_temelemetry.data.humidity
                        & 0xFFU;

                // temperature (int16_t)
                gen_tel_pl.payload[28] =
                        ((uint16_t) cimatelite_temelemetry.data.temperature
                                >> 8U) & 0xFFU;
                gen_tel_pl.payload[29] =
                        (uint16_t) cimatelite_temelemetry.data.temperature
                                & 0xFFU;

                // co2
                gen_tel_pl.payload[30] =
                        (cimatelite_temelemetry.data.co2 >> 8U) & 0xFFU;
                gen_tel_pl.payload[31] = cimatelite_temelemetry.data.co2
                        & 0xFFU;

                gen_tel_pl.length = 32U; // tamanho total do payload

                uint8_t gen_tel_pl_raw[120] = { 0 };
                uint16_t gen_tel_pl_raw_len = 0;

                fsat_pkt_encode(&gen_tel_pl, gen_tel_pl_raw,
                                &gen_tel_pl_raw_len);

                if ((!sat_data_buf.obdh.data.hibernation_on)
                        && (sat_data_buf.obdh.data.payload_telemetry_on))
                {
                    if (ttc_send(TTC_1, gen_tel_pl_raw, gen_tel_pl_raw_len)
                            != 0)
                    {
                        sys_log_print_event_from_module(
                                SYS_LOG_ERROR,
                                TASK_PAYLOAD_TELEMETRY_NAME,
                                "Error transmiting the payload telemetry packet!");
                        sys_log_new_line();
                    }
                    else
                    {
                        sys_log_print_event_from_module(
                                SYS_LOG_INFO, TASK_PAYLOAD_TELEMETRY_NAME,
                                "Telemetria da payload enviada com sucesso.");
                        sys_log_new_line();
                    }
                }
            }

        }

        vTaskDelayUntil(&last_cycle,
                        pdMS_TO_TICKS(TASK_PAYLOAD_TELEMETRY_PERIOD_MS));
    }
}

/** \} End of general_telemetry group */
