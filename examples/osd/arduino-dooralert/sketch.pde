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
#include "rest-engine.h"

extern resource_t res_door, res_battery;
uint8_t door_pin = 3;
uint8_t door_status = 0;

#define LED_PIN 4
}

void setup (void)
{
    // switch off the led
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    // init coap resourcen
    rest_init_engine ();
    rest_activate_resource (&res_door, "s/door");
    rest_activate_resource (&res_battery, "s/battery");
 //   NETSTACK_MAC.off(1);
}

void loop (void)
{
	mcu_sleep_off();
	
	mcu_sleep_on();
}
