/*
 * Copyright (c) 2014, OSDomotics, Institute of Technology.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      DS1820 Sensor Resource
 *
 *      This is a simple GET resource that returns the temperature in Celsius
 *
 * \author
 *      Harald Pichler <harald@the-develop.net>
 */

#include "contiki.h"

#if PLATFORM_HAS_DS1820

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "rest-engine.h"
#include "dev/ds1820.h"

/* A simple getter example. Returns the reading from ds1820 sensor */
#define DS1820_TEMP_LSB                0
#define DS1820_TEMP_MSB                1
#define DS1820_COUNT_REMAIN    6
#define DS1820_COUNT_PER_C     7

//RESOURCE(ds1820, METHOD_GET, "s/temp", "title=\"Temperatur DS1820\";rt=\"temperature-c\"");
static void res_get_ds1820_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* A simple getter example. Returns the reading from light sensor with a simple etag */
RESOURCE(res_ds1820,
         "title=\"Temperature DHTxx\";rt=\"temperature c\"",
         res_get_ds1820_handler,
         NULL,
         NULL,
         NULL);
         
static void
res_get_ds1820_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{

  char message[100];
  int length = 0; /*           |<-------->| */
  union temp_raw {
         int16_t  s_int16;
         uint16_t u_int16;
  } temp_raw;
  double temp_c;
  int temp_integral;
  int temp_centi;

  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);

  // temp = temp_read - 0.25°C + (count_per_c - count_remain) / count_per_c;
  temp_raw.u_int16 = ds1820_ok[DS1820_TEMP_MSB] << 8 | ds1820_ok[DS1820_TEMP_LSB];
  temp_c = temp_raw.s_int16 / 2.0
          - 0.25
          + ((double) ds1820_ok[DS1820_COUNT_PER_C] - (double) ds1820_ok[DS1820_COUNT_REMAIN])
            / (double) ds1820_ok[DS1820_COUNT_PER_C];
  temp_integral = (int) temp_c;
  temp_centi = (int) (fabs (temp_c - (int) temp_c) * 100.0);

  if(accept == -1 || accept == REST.type.TEXT_PLAIN)
  {
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    snprintf(message, REST_MAX_CHUNK_SIZE, "%d.%02d C", temp_integral, temp_centi);

    length = strlen(message);
    memcpy(buffer, message,length );

    REST.set_response_payload(response, buffer, length);
  }
  else if (accept == REST.type.APPLICATION_JSON)
  {
    REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
    snprintf(message, REST_MAX_CHUNK_SIZE, "{\"temp\":\"%d.%02d\"}", temp_integral, temp_centi);

    length = strlen(message);
    memcpy(buffer, message,length );

    REST.set_response_payload(response, buffer, length);
  }
  else
  {
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    REST.set_response_payload(response, (uint8_t *)"Supporting content-types text/plain and application/json", 56);
  }
}
#endif /* PLATFORM_HAS_DS1820 */
