/* Sensor routine */

#include "lib/sensors.h"
#include "dev/button-sensor.h"

#include <avr/interrupt.h>
#include "led.h" // debug

const struct sensors_sensor button_sensor;

static struct timer debouncetimer;
static int status(int type);
static int enabled = 0;
volatile static int bstate = 1; /* initial button state */
struct sensors_sensor *sensors[1];
unsigned char sensors_flags[1];

#define BUTTON_BIT INTF4
#define BUTTON_CHECK_IRQ() (EIFR & BUTTON_BIT) ? 0 : 1

#define PRINTF(...) printf(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
ISR(INT4_vect)
{

//  leds_toggle(LEDS_RED);
  if(BUTTON_CHECK_IRQ()) {
    bstate = (PINE & _BV(PE4) ? 0 : 1);
    if(timer_expired(&debouncetimer) || bstate == 1) {
  //  led1_on();
      timer_reset(&debouncetimer);
      sensors_changed(&button_sensor);
 //   led1_off();
    }
  }

}
/*---------------------------------------------------------------------------*/

static int
value(int type)
{
 //return (PINE & _BV(PE4) ? 0 : 1) || !timer_expired(&debouncetimer);

 return bstate;
}

static int
configure(int type, int c)
{
	switch (type) {
	case SENSORS_ACTIVE:
		if (c) {
			if(!status(SENSORS_ACTIVE)) {
  //  led1_on();
				timer_set(&debouncetimer, CLOCK_SECOND / 8);
				DDRE |= (0<<DDE4); // Set pin as input
				PORTE |= (1<<PORTE4); // Set port PORTE bint 5 with pullup resistor
				EICRB |= (1<<ISC40); // For falling edge
				EIMSK |= (1<<INT4); // Set int
				enabled = 1;
				sei();
  //  led1_off();
			}
		} else {
				enabled = 0;
				EIMSK &= ~(1<<INT4); // clear int
		}
		return 1;
	}
	return 0;
}

static int
status(int type)
{
	switch (type) {
	case SENSORS_ACTIVE:
	case SENSORS_READY:
		return enabled;//(EIMSK & (1<<INT5) ? 0 : 1);//BUTTON_IRQ_ENABLED();
	}
	return 0;
}

SENSORS_SENSOR(button_sensor, BUTTON_SENSOR,
	       value, configure, status);

