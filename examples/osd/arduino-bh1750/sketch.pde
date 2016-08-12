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

#include <Wire.h>
#include "BH1750FVI.h"

extern "C" {
#include "arduino-process.h"
#include "rest-engine.h"

BH1750FVI lightMeter;

extern resource_t res_bh1750, res_battery;
uint16_t lux;

#define LED_PIN 4
}

void setup (void)
{
    // switch off the led
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    // BH1750 sensor
    Wire.begin();
    lightMeter.begin();
    // init coap resourcen
    rest_init_engine ();
    rest_activate_resource (&res_bh1750, "s/lux");
    rest_activate_resource (&res_battery, "s/battery");
}

// at project-conf.h
// LOOP_INTERVAL		(10 * CLOCK_SECOND)
void loop (void)
{
	mcu_sleep_off();
    lux = lightMeter.getLightLevel();
	printf("Lux: %d\n",lux);
  	mcu_sleep_on();    
}
