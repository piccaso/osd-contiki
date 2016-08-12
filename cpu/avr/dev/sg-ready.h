/*
 *  Copyright (c) 2015 Bernhard Trinnes
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \file
 *
 * \brief
 *     
 *
 * \author
 *      Bernhard Trinnes bernhard.trinnes@guh.guru
 *
 */

#ifndef __SGREADY_H__
#define __SGREADY_H__

#include <avr/io.h>

/* Some other io settings of io include define DDRxx as DDxx */
#ifndef DDRB0
#define DDRB0 DDB0
#define DDRB1 DDB1
#define DDRB2 DDB2
#define DDRB3 DDB3
#define DDRB4 DDB4
#define DDRB5 DDB5
#define DDRB6 DDB6
#define DDRB7 DDB7
#endif

#ifndef DDRD0
#define DDRD0 DDD0
#define DDRD1 DDD1
#define DDRD2 DDD2
#define DDRD3 DDD3
#define DDRD4 DDD4
#define DDRD5 DDD5
#define DDRD6 DDD6
#define DDRD7 DDD7
#endif

#define LATCH_TIME 3000 // time in micro seconds

#define RELAY1_ON     (1<<DDRB4)
#define RELAY1_OFF    (1<<DDRB5)
#define RELAY2_ON     (1<<DDRB6)
#define RELAY2_OFF    (1<<DDRB7)
#define RELAY1_FB     (1<<DDRD5)
#define RELAY2_FB     (1<<DDRD7)


/** @name Smart Grid Ready Functions */
/** @{ */
void sg_relay_init(uint8_t state);
void sg_set_state(uint8_t pin);
uint8_t sg_get_state();
void sg_switch_difference(uint8_t old_state, uint8_t new_state); 

/** @} */

#endif /* __SGREADY_H__ */
