/**
 * \defgroup LED PWM example
 *
 * Resource definition for LED PWM module
 *
 * @{
 */

/**
 * \file
 *         Resource definitions for the LED PWM module
 *      
 * \author
 *         Ralf Schlatterbeck <rsc@tux.runtux.com>
 */

#ifndef led_pwm_h
#define led_pwm_h
#include "contiki.h"

extern resource_t res_led_pwm;
extern void led_pwm_init (void);

#endif // led_pwm_h
/** @} */
