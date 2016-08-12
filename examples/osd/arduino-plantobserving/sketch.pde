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
#include "rest-engine.h"

extern resource_t res_moisture, res_battery;
uint8_t moisture_pin = A5;
uint8_t moisture_vcc = 19;
uint16_t moisture_voltage = 0;

#define LED_PIN 4
}

void setup (void)
{
    // switch off the led
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    // init moisture sensor
    pinMode(moisture_vcc, OUTPUT);
    digitalWrite(moisture_vcc, LOW);
    // init coap resourcen
    rest_init_engine ();
    rest_activate_resource (&res_moisture, "s/moisture");
    rest_activate_resource (&res_battery, "s/battery");
}

void loop (void)
{

}
