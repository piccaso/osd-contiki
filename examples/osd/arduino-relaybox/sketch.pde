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
#include "dev/relay-sensor.h"

extern resource_t res_led, res_battery, res_cputemp, 
	res_relay1, res_relay2, res_relay3, res_relay4,
	res_relay5, res_relay6, res_relay7, res_relay8;

uint8_t led_pin=4;
uint8_t led_status;

}

void setup (void)
{
    // switch off the led
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, HIGH);
    led_status=0;
    // init relay
    SENSORS_ACTIVATE(relay_sensor);
    // init coap resourcen
    rest_init_engine ();
    #pragma GCC diagnostic ignored "-Wwrite-strings"
    rest_activate_resource (&res_led, "s/led");
    rest_activate_resource (&res_battery, "s/battery");
    rest_activate_resource (&res_cputemp, "s/cputemp");
    rest_activate_resource (&res_relay1, "relay/1");
    rest_activate_resource (&res_relay2, "relay/2");
    rest_activate_resource (&res_relay3, "relay/3");
    rest_activate_resource (&res_relay4, "relay/4");
    rest_activate_resource (&res_relay5, "relay/5");
    rest_activate_resource (&res_relay6, "relay/6");
    rest_activate_resource (&res_relay7, "relay/7");
    rest_activate_resource (&res_relay8, "relay/8");
    #pragma GCC diagnostic pop
    NETSTACK_MAC.off(1);
}

void loop (void)
{

}
