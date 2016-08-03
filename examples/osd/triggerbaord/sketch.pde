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
#include "net/netstack.h"
#include "dev/button-sensor.h"



extern resource_t 
    res_led, 
    res_battery, 
    res_cputemp,
    res_event,
    res_separate,
    res_server;

uint8_t led_pin=4;
uint8_t led_status;
}

void setup (void)
{
    // switch off the led
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, HIGH);
    led_status=0;
    // sensors
    SENSORS_ACTIVATE(button_sensor);
    // init coap resourcen
    rest_init_engine ();
    rest_activate_resource (&res_led, "s/led");
    rest_activate_resource (&res_battery, "s/battery");
    rest_activate_resource (&res_cputemp, "s/cputemp");
    rest_activate_resource(&res_event, "s/button");      
//    rest_activate_resource (&res_battery, "p/server");    

    NETSTACK_MAC.off(1);
}

void loop (void)
{
	mcu_sleep_off();
	
	mcu_sleep_on();
}
