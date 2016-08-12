/*
 *  Copyright (c) 2010  harald pichler
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
 *      This file provides Raven KEY support.
 *
 * \author
 *      Harald Pichler harald@the-develop.net
 *
 */

#include <avr/interrupt.h>
#include "dev/led.h"
#include "pcintkey.h"

/*---------------------------------------------------------------------------*/

ISR(PCINT0_vect)
{
//  if(BUTTON_CHECK_IRQ()) {
//    if(timer_expired(&debouncetimer)) {
//    led1_on();
//      timer_set(&debouncetimer, CLOCK_SECOND / 4);
//      sensors_changed(&button_sensor);
//    led1_off();
//    }
//  }
}

/* Compatibility of old vs new definitions in io.h */
#ifndef DDE0
#define DDE0 DDRE0
#define DDE1 DDRE1
#define DDE2 DDRE2
#define DDE3 DDRE3
#define DDE4 DDRE4
#define DDE5 DDRE5
#define DDE6 DDRE6
#define DDE7 DDRE7
#endif
#ifndef DDF0
#define DDF0 DDRF0
#define DDF1 DDRF1
#define DDF2 DDRF2
#define DDF3 DDRF3
#define DDF4 DDRF4
#define DDF5 DDRF5
#define DDF6 DDRF6
#define DDF7 DDRF7
#endif

/**
 *   \brief This will intialize the KEY for button readings.
 */
void
key_init(void)
{
    // Pairing Button
//    PORTB |= (1<<PORTE0); // Set port PORTE pint 0 with pullup resistor
//    DDRB |= (1<<DDE0); // Set pin as input
    // ext1
    PORTF |= (1<<PORTF6); // Set port PORTF pint 6 with pullup resistor
    DDRF |= (1<<DDF6); // Set pin as input
    // ext2
    PORTF |= (1<<PORTF7); // Set port PORTF pint 7 with pullup resistor
    DDRF |= (1<<DDF7); // Set pin as input
    // Interrupt
    //PCICR |= _BV(PCIE0);
    //PCMSK0 |= _BV(PCINT4) | _BV(PCINT5) | _BV(PCINT6);
}

/*---------------------------------------------------------------------------*/

/**
 *   \brief This will poll run key_task() to determine if a button has been pressed.
 *
 *   \retval True if button is pressed
 *   \retval False if button is not pressed
*/
uint8_t
is_button_ext4(void)
{
    /* Return true if button has been pressed. */
    if ( PINE & (1<<PINE0) ) {
        return 0;
    }
    else{
        return 1;
    }
}
uint8_t
is_button_ext5(void)
{
    /* Return true if button has been pressed. */
    if ( PINF & (1<<PINF6) ) {
        return 0;
    }
    else{
        return 1;
    }
}
uint8_t
is_button_ext6(void)
{
    /* Return true if button has been pressed. */
    if ( PINF & (1<<PINF7) ) {
        return 0;
    }
    else{
        return 1;
    }
}
