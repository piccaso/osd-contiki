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

extern volatile uint8_t mcusleepcycle;  // default 16
extern resource_t res_moisture, res_battery;
uint8_t moisture_pin = A5;
uint16_t moisture_voltage = 0;
#define BUZZER_PIN 3 /* sig pin of the buzzer */
#define LED_PIN 4    /* LED Pin */
}

void setup (void)
{
    // switch off the led
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    // setup Buzzer 
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    // init coap resourcen
    rest_init_engine ();
    rest_activate_resource (&res_moisture, "s/moisture");
    rest_activate_resource (&res_battery, "s/battery");
}

void loop (void)
{
  mcu_sleep_off();
  moisture_voltage = analogRead(moisture_pin);
  if(moisture_voltage < 800){
      digitalWrite(BUZZER_PIN, LOW);
  }else{
      digitalWrite(BUZZER_PIN, HIGH);
  }
  mcu_sleep_on();
}
