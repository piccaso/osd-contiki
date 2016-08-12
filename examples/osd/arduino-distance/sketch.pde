/*
 * Sample arduino sketch using contiki features.
 * We turn the LED off 
 * We allow read the moisture sensor
 * Unfortunately sleeping for long times in loop() isn't currently
 * possible, something turns off the CPU (including PWM outputs) if a
 * Proto-Thread is taking too long. We need to find out how to sleep in
 * a Contiki-compatible way.
 * Note that for a normal arduino sketch you won't have to include any
 * of the contiki-specific files here, the sketch should just work.
 */

extern "C" {
#include "arduino-process.h"
#include "sketch.h"
#include "rest-engine.h"
#include "net/netstack.h"

extern resource_t res_battery, res_distance;
#define LED_PIN 4    /* LED Pin */

}

void setup (void)
{
    NETSTACK_MAC.off(1);

    // switch off the led
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    // init coap resourcen
    rest_init_engine ();
    rest_activate_resource (&res_battery, "s/battery");
    rest_activate_resource (&res_distance, "s/distance");

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
}

void loop (void)
{
	mcu_sleep_off();
	
	mcu_sleep_on();
}

