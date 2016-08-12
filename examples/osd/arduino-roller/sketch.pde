/*
 * Sample arduino sketch using contiki features.
 * We turn the LED off 
 * We allow read the poti sensor
 * Unfortunately sleeping for long times in loop() isn't currently
 * possible, something turns off the CPU (including PWM outputs) if a
 * Proto-Thread is taking too long. We need to find out how to sleep in
 * a Contiki-compatible way.
 * Note that for a normal arduino sketch you won't have to include any
 * of the contiki-specific files here, the sketch should just work.
 */

extern "C" {
#include "rest-engine.h"
#include "dev/servo.h"
#include "kexp.h" // exponential values lookup table

extern resource_t res_poti, res_battery;
uint8_t poti_pin = A5;
uint16_t poti_voltage = 0;

uint16_t servo_min= 1000;
uint16_t servo_max= 2000;

#define LED_PIN 4
}

//**********************scale**********************************

  int scale (int in_min, int in_max, int out_min, int out_max, int wert)
  {
    int abc;

    abc = ((long)out_max - (long)out_min)* ((long)wert-(long)in_min) / ( (long)in_max - (long)in_min);
    abc = abc + out_min;

    return (abc);
  }
  
void setup (void)
{
    // switch off the led
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    // init servo
    servo_init();
        // init coap resourcen
    rest_init_engine ();
    rest_activate_resource (&res_poti, "s/poti");
    rest_activate_resource (&res_battery, "s/battery");
}

void loop (void)
{
  int value;
  int vlookup;
  poti_voltage = analogRead(poti_pin);
  value=scale (345,868,servo_min,servo_max,poti_voltage);
  vlookup=lookup_Daumengas(value-servo_min)+servo_min;
  servo_set(1,vlookup);
//  servo_set(1,value);
  printf("%d,%d,%d\n",poti_voltage,value,vlookup);
}
