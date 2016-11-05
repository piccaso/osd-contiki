/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
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
 *      rgb resource
 * \author
 *      Harald Pichler <harald@the-develop.net>
 */

#include "contiki.h"
#include <string.h>
#include "rest-engine.h"
#include "generic_resource.h"
#include "Arduino.h"
#include "sketch.h"

extern uint8_t color_rgb [3];

static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_post_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* A simple getter example. Returns the reading from the sensor with a simple etag */
RESOURCE(res_rgb,
         "title=\"LED: , POST/PUT r=10&g=11&b=12\";rt=\"Control\"",
         res_get_handler,
         res_post_put_handler,
         res_post_put_handler,
         NULL);

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);

  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "r=%d&g=%d&b=%d",color_rgb [0],color_rgb [1],color_rgb [2] );

    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else if(accept == REST.type.APPLICATION_JSON) {
    REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{'r':%d\n,'g':%d\n,'b':%d}",color_rgb [0],color_rgb [1],color_rgb [2]);

    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else {
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    const char *msg = "Supporting content-types text/plain and application/json";
    REST.set_response_payload(response, msg, strlen(msg));
  }
}

static void
res_post_put_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  size_t len = 0;
  const char *valr = NULL;
  const char *valg = NULL;
  const char *valb = NULL;
  int success = 1;

  if(success && (len = REST.get_post_variable(request, "r", &valr))) {
    if(len != 0) {
 //       printf("r= %s;",valr);
    } else {
      success = 0;
    }
    if(success && (len = REST.get_post_variable(request, "g", &valg))) {
      if(len != 0) {
 //       printf("g= %s;",valg);
      } else {
        success = 0;
      }
    }
    if(success && (len = REST.get_post_variable(request, "b", &valb))) {
      if(len != 0) {
 //       printf("b= %s;\n",valb);
      } else {
        success = 0;
      }
    }
    if((success) && !color_rgb_from_string (valr, valg, valb)) {
      
    }
  } else {
    success = 0;
  } if(!success) {
    REST.set_response_status(response, REST.status.BAD_REQUEST);
  }
}

#pragma GCC diagnostic ignored "-Wwrite-strings"
GENERIC_RESOURCE
  ( red
  , RED_LED
  , s
  , 1
  , color_from_string
  , color_to_string
  );

GENERIC_RESOURCE
  ( green
  , GREEN_LED
  , s
  , 1
  , color_from_string
  , color_to_string
  );

GENERIC_RESOURCE
  ( blue
  , BLUE_LED
  , s
  , 1
  , color_from_string
  , color_to_string
  );
#pragma GCC diagnostic pop
