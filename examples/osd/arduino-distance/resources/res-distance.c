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
 *      Moisture resource
 * \author
 *      Harald Pichler <harald@the-develop.net>
 */

#include "contiki.h"

#include <string.h>
#include "rest-engine.h"
#include "Arduino.h"
#include "sketch.h"


#define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
#define clockCyclesToMicroseconds(a) ( (a) / clockCyclesPerMicrosecond() )
#define microsecondsToClockCycles(a) ( (a) * clockCyclesPerMicrosecond() )

unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout)
{
        // cache the port and bit of the pin in order to speed up the
        // pulse width measuring loop and achieve finer resolution.  calling
        // digitalRead() instead yields much coarser resolution.
        uint8_t bit = digitalPinToBitMask(pin);
        uint8_t port = digitalPinToPort(pin);
        uint8_t stateMask = (state ? bit : 0);
        unsigned long width = 0; // keep initialization out of time critical area

        // convert the timeout from microseconds to a number of times through
        // the initial loop; it takes 16 clock cycles per iteration.
        unsigned long numloops = 0;
        unsigned long maxloops = microsecondsToClockCycles(timeout) / 16;

        // wait for any previous pulse to end
        while ((*portInputRegister(port) & bit) == stateMask)
                if (numloops++ == maxloops)
                        return 0;
        // wait for the pulse to start
        while ((*portInputRegister(port) & bit) != stateMask)
                if (numloops++ == maxloops)
                        return 0;

        // wait for the pulse to stop
        while ((*portInputRegister(port) & bit) == stateMask)
                width++;

        // convert the reading to microseconds. The loop has been determined
        // to be 10 clock cycles long and have about 16 clocks between the edge
        // and the start of the loop. There will be some error introduced by
        // the interrupt handlers.
        return clockCyclesToMicroseconds(width * 10 + 16);
}




static void res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

/* A simple getter example. Returns the reading from the sensor with a simple etag */
RESOURCE(res_distance,
         "title=\"Distance status\";rt=\"Distance\"",
         res_get_handler,
         NULL,
         NULL,
         NULL);

static void
res_get_handler(void *request, void *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{

  unsigned int accept = -1;
  REST.get_header_accept(request, &accept);

  long duration, distance;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(20);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 500000);

  // found this computation in some arduino examples
  //distance = (duration/2) / 29.1;
  // to get millimeters (duration -20) / 3.18 is a good approach
  distance = (duration-20)/3.18;

  if(accept == -1 || accept == REST.type.TEXT_PLAIN) {
    REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%ld", distance);

    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else if(accept == REST.type.APPLICATION_JSON) {
    REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
    snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{'distance':%ld}", distance);

    REST.set_response_payload(response, buffer, strlen((char *)buffer));
  } else {
    REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
    const char *msg = "Supporting content-types text/plain and application/json";
    REST.set_response_payload(response, msg, strlen(msg));
  }
}

