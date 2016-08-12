/*
 * Copyright (c) 2014-15, Ralf Schlatterbeck Open Source Consulting
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
 * \defgroup Hardware independent Arduino compatibility
 *
 * This file is meant to be included into a compatible Arduino.h
 *
 * Arduino calls the combination of an AVR timer and the corresponding
 * channel a timer. So Arduinos definition of timer is different from
 * that of AVR documentation. Arduino defines arbitrary TIMERXX macros
 * that are later parsed in a big switch statement.
 *
 * We use a better representation of timer values than arduino here:
 * The AVRs have max. 6 (numbered 0-5) timers. For representing these we
 * need 3 bits. In addition each timer can have channels A-D (or just
 * one channel in which case there is no alphabetic suffix). We can
 * represent this in 2 bits. We add one bit to each for future
 * compatibility and come up with 7 bits, still easily represented in 8
 * bit with room for a 'NOT_ON_TIMER' value. From these we can easily
 * compute the channel and timer by shifting. No need for a big switch
 * statement, and -- which is the common case -- when initializing with
 * a constant for the pin, the compiler can compute everything at
 * compile-time (that's why the analogWrite below is implemented as a
 * static inline function).
 *
 * Note that Arduino also defines some TIMERX without an alphabetic
 * suffix (e.g. TIMER2). I suspect this is for microcontrollers that
 * only have one channel for a certain timer. So this is currently
 * defined the same as TIMER2A because they are never used together.
 * This may be wrong and may be a bug.
 *
 * Note that the hardware definition still has to define a
 * digitalPinToTimer macro. We suggest to not implement this with a
 * static table in program memory (as currently done by arduino) but
 * instead as an if-cascade (as a C-macro). This allows the compiler to
 * completely compute the if-cascade at compile-time if the used pin is
 * a constant, resulting in *much* smaller code-footprint in the most
 * common use-case.
 *
 * @{
 */

/**
 * \file
 *        Header file for arduino compatibility
 * \author
 *        Ralf Schlatterbeck <rsc@runtux.com>
 *
 */

/* To be included by a compatible Arduino.h */
#ifdef __cplusplus
extern "C" {
#endif

#include "contiki.h"
#include "hw_timer.h"
#include "adc.h"

#ifdef __cplusplus
} // extern "C"
#endif

#define HW_TIMER_SHIFT 3

#define NOT_ON_TIMER 0xFF

#define TIMER0A ((0 << HW_TIMER_SHIFT) | HWT_CHANNEL_A)
#define TIMER0B ((0 << HW_TIMER_SHIFT) | HWT_CHANNEL_B)

#define TIMER1A ((1 << HW_TIMER_SHIFT) | HWT_CHANNEL_A)
#define TIMER1B ((1 << HW_TIMER_SHIFT) | HWT_CHANNEL_B)
#define TIMER1C ((1 << HW_TIMER_SHIFT) | HWT_CHANNEL_C)

#define TIMER2  ((2 << HW_TIMER_SHIFT) | HWT_CHANNEL_A)
#define TIMER2A ((2 << HW_TIMER_SHIFT) | HWT_CHANNEL_A)
#define TIMER2B ((2 << HW_TIMER_SHIFT) | HWT_CHANNEL_B)

#define TIMER3A ((3 << HW_TIMER_SHIFT) | HWT_CHANNEL_A)
#define TIMER3B ((3 << HW_TIMER_SHIFT) | HWT_CHANNEL_B)
#define TIMER3C ((3 << HW_TIMER_SHIFT) | HWT_CHANNEL_C)

#define TIMER4A ((4 << HW_TIMER_SHIFT) | HWT_CHANNEL_A)
#define TIMER4B ((4 << HW_TIMER_SHIFT) | HWT_CHANNEL_B)
#define TIMER4C ((4 << HW_TIMER_SHIFT) | HWT_CHANNEL_C)
#define TIMER4D ((4 << HW_TIMER_SHIFT) | HWT_CHANNEL_D)

#define TIMER5A ((5 << HW_TIMER_SHIFT) | HWT_CHANNEL_A)
#define TIMER5B ((5 << HW_TIMER_SHIFT) | HWT_CHANNEL_B)
#define TIMER5C ((5 << HW_TIMER_SHIFT) | HWT_CHANNEL_C)

#ifdef __cplusplus
extern "C" {
#endif

static inline void analogWrite(uint8_t pin, int val)
{
  /*
   * Note on the timer usage: Arduino has code here that
   * explicitly checks if the given val is 0 or 0xFF.
   * The 16-bit timers on Arduino use the phase correct PWM
   * waveform generation mode which already sets the output to
   * continuous low for 0 or continuous high for 0xFF.  When using
   * an 8-bit timer, Arduino uses fast PWM which creates a tiny
   * spike for 0, so to be Arduino-compatible in this mode we use
   * digitalWrite in this case.
   */
  uint8_t arduino_timer = digitalPinToTimer(pin);
  pinMode(pin, OUTPUT);
  if (val == 0 || arduino_timer == NOT_ON_TIMER) {
    digitalWrite(pin, (val < 128) ? LOW : HIGH);
  } else {
    uint8_t t = arduino_timer >> HW_TIMER_SHIFT;
    uint8_t c = arduino_timer & HWT_CHANNEL_MASK;
    hwtimer_pwm_enable (t, c);
    hwtimer_set_pwm (t, c, val);
  }
}

/*
 * turnOffPWM of arduino is implemented by hw_timer
 */
#define turnOffPWM(atimer)                                   \
  ( (atimer) == NOT_ON_TIMER                                 \
  ? (void)0                                                  \
  : (void)hwtimer_pwm_disable                                \
      (atimer >> HW_TIMER_SHIFT, atimer & HWT_CHANNEL_MASK)  \
  )

/*
 * micros on arduino takes timer overflows into account.
 * We put in the seconds counter. To get a consistent seconds / ticks
 * value we have to disable interrupts.
 */
static inline uint32_t micros (void)
{
  uint32_t ticks;
  uint8_t sreg = SREG;
  cli ();
  ticks = clock_seconds () * 1000000L
        + clock_time () * 1000L / CLOCK_SECOND;
  SREG = sreg;
  return ticks;
}
/*
 * millis counts only internal timer ticks since start, not trying to do
 * something about overflows. Note that we don't try to emulate overflow
 * behaviour of arduino implementation.
 */
#define millis()              (((uint32_t)clock_time())*1000L/CLOCK_SECOND)
#define delay(ms)             clock_delay_msec(ms)
#define delayMicroseconds(us) clock_delay_usec(us)

#define analogRead(analogpin) readADC(analogpin)

#ifdef __cplusplus
} // extern "C"
#endif

/*
 * VI settings, see coding style
 * ex:ts=8:et:sw=2
 */

/** @} */
