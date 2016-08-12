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
#include "Adafruit_HTU21DF.h"

extern "C" {
#include "arduino-process.h"
#include "rest-engine.h"

Adafruit_HTU21DF htu = Adafruit_HTU21DF();

extern resource_t res_htu21dtemp, res_htu21dhum, res_battery;
float htu21d_hum;
float htu21d_temp;
char  htu21d_hum_s[8];
char  htu21d_temp_s[8];

#define LED_PIN 4
}

void setup (void)
{
    // switch off the led
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    // htu21d sensor
    if (!htu.begin()) {
      printf("Couldn't find sensor!");
    }
    // init coap resourcen
    rest_init_engine ();
    rest_activate_resource (&res_htu21dtemp, "s/temp");
    rest_activate_resource (&res_htu21dhum, "s/hum");
    rest_activate_resource (&res_battery, "s/battery");
}

// at project-conf.h
// LOOP_INTERVAL		(10 * CLOCK_SECOND)
void loop (void)
{
	mcu_sleep_off();
    htu21d_temp = htu.readTemperature();
    htu21d_hum = htu.readHumidity();
  	mcu_sleep_on();    
    dtostrf(htu21d_temp , 6, 2, htu21d_temp_s );   
	dtostrf(htu21d_hum , 6, 2, htu21d_hum_s );
    // remove space
    if(htu21d_temp_s[0]==' '){
      memcpy (htu21d_temp_s,htu21d_temp_s+1,strlen(htu21d_temp_s)+1);
    }
    if(htu21d_hum_s[0]==' '){
        memcpy (htu21d_hum_s,htu21d_hum_s+1,strlen(htu21d_hum_s)+1);
    }
      
//  debug only   
//	  printf("Temp: %s",htu21d_temp_s);
//    printf("\t\tHum: %s\n",htu21d_hum_s);
}
