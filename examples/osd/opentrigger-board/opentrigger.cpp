#include <stdlib.h>
#include <string.h>
#include "hw_timer.h"
#include "rest-engine.h"
#include "er-coap-engine.h"
#include "adc.h"
#include "hw-arduino.h"
#include "contiki.h"
//#include "opentrigger.h"
#include "net/netstack.h"
#include "dev/button-sensor.h"

extern "C" {
#include "ChainableLED.h"
}

extern resource_t 
    res_led,
    res_bled, 
    res_battery, 
    res_cputemp,
    res_event,
    res_separate,
    res_server;

uint8_t led_pin=4;
uint8_t led_status;
uint8_t bled_pin=7;
uint8_t bled_status;


#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]", (lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3], (lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif

extern volatile uint8_t mcusleepcycle;
#if PLATFORM_HAS_BUTTON
#include "rest-engine.h"
#include "dev/button-sensor.h"
extern resource_t res_event, res_separate;
#endif /* PLATFORM_HAS_BUTTON */


volatile uint8_t mcusleepcycleval;

#define NUM_LEDS  1

// Merkurboard grove i2c D8, D9
ChainableLED leds(8, 9, NUM_LEDS);

uint8_t color_rgb [3] = {0, 0, 0};
static uint8_t name_to_offset (const char * name)
{
  uint8_t offset = 0;
  if (0 == strcmp (name, "green")) {
    offset = 1;
  } else if (0 == strcmp (name, "blue")) {
    offset = 2;
  }
  return offset;
}

static size_t
color_to_string (const char *name, const char *uri, char *buf, size_t bsize)
{
  return snprintf (buf, bsize, "%d", color_rgb [name_to_offset (name)]);
}

int color_from_string (const char *name, const char *uri, const char *s)
{
    color_rgb [name_to_offset (name)] = atoi (s);
    leds.setColorRGB(0,color_rgb [0], color_rgb [1], color_rgb [2]);
    return 0;
}

uip_ipaddr_t server_ipaddr, tmp_addr;

void setup (void)
{
    // switch off the led
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, HIGH);
    led_status=0;
    // switch off the bled
    pinMode(bled_pin, OUTPUT);
    digitalWrite(bled_pin, LOW);
    bled_status=0;
    // init chainable led
    leds.init();
    leds.setColorRGB(0,color_rgb [0], color_rgb [1], color_rgb [2]);
    // sensors
    SENSORS_ACTIVATE(button_sensor);
    // init coap resourcen
    rest_init_engine ();
    SERVER_NODE (&server_ipaddr);
    #pragma GCC diagnostic ignored "-Wwrite-strings"
    rest_activate_resource (&res_led, "s/led");
    rest_activate_resource (&res_bled, "s/bled");
    rest_activate_resource (&res_battery, "s/battery");
    rest_activate_resource (&res_cputemp, "s/cputemp");
    rest_activate_resource(&res_event, "s/button");
    rest_activate_resource (&res_red,   "led/R");
    rest_activate_resource (&res_green, "led/G");
    rest_activate_resource (&res_blue,  "led/B");         
    #pragma GCC diagnostic pop

//    mcu_sleep_set(64);
    NETSTACK_MAC.off(1);
}


void loop (void)
{
// test caop srever post

   static int buttonstate = 1;
   
   if(buttonstate != button_sensor.value(0)){
     coap_server_post();
     buttonstate=button_sensor.value(0);
   }
// test chainable led
/*
   static byte power=0;
   for (byte i=0; i<NUM_LEDS; i++)
   {
     if (i%2 == 0)
       leds.setColorRGB(i, power, 0, 0);
     else
       leds.setColorRGB(i, 0, 255-power, 0);
   }
   power+= 10;
*/
}


/*-------------- enabled sleep mode ----------------------------------------*/
void
mcu_sleep_init(void)
{
	mcusleepcycleval=mcusleepcycle;
}
void
mcu_sleep_on(void)
{
	mcusleepcycle= mcusleepcycleval;
}
/*--------------- disable sleep mode ---------------------------------------*/
void
mcu_sleep_off(void)
{
	mcusleepcycle=0;
}
/*---------------- set duty cycle value ------------------------------------*/
void
mcu_sleep_set(uint8_t value)
{
	mcusleepcycleval= value;
	mcusleepcycle = mcusleepcycleval;
}


PROCESS(arduino_sketch, "Arduino Sketch Wrapper");

#ifndef LOOP_INTERVAL
#define LOOP_INTERVAL		(1 * CLOCK_SECOND)
#endif


#define REMOTE_PORT UIP_HTONS(COAP_DEFAULT_PORT)
// should be the same :-)
#define UIP_NTOHS(x) UIP_HTONS(x)
#define SERVER_NODE(ip) \
    uip_ip6addr(ip,0xaaaa,0,0,0,0,0,0,0x01)


char         server_resource [20] = "button";

static int32_t levent_counter; 

int coap_server_post(void)
{
    static coap_packet_t request [1]; /* Array: treat as pointer */
    char buf [25];
    
    printf("post\n");
    coap_transaction_t *transaction;
    int buttonstate = button_sensor.value(0);
    sprintf (buf, "state=%d&event=%lu",buttonstate,levent_counter++);
//  printf ("%s\n", buf);
    coap_init_message (request, COAP_TYPE_NON, COAP_PUT, 0);
    coap_set_header_uri_path (request, server_resource);
    coap_set_header_content_format (request, REST.type.TEXT_PLAIN);
    coap_set_payload (request, buf, strlen (buf));
    request->mid = coap_get_mid ();
    transaction = coap_new_transaction
        (request->mid, &server_ipaddr, REMOTE_PORT);
    transaction->packet_len = coap_serialize_message
        (request, transaction->packet);
    coap_send_transaction (transaction);
    return 0;
}

PROCESS_THREAD(arduino_sketch, ev, data)
{
  static struct etimer loop_periodic_timer;

  PROCESS_BEGIN();
  adc_init ();
  mcu_sleep_init ();
  setup ();
  /* Define application-specific events here. */
  etimer_set(&loop_periodic_timer, LOOP_INTERVAL);
  while (1) {
	PROCESS_WAIT_EVENT();
#if PLATFORM_HAS_BUTTON
    if(ev == sensors_event && data == &button_sensor) {
      mcu_sleep_off();
      PRINTF("*******BUTTON*******\n");

      /* Call the event_handler for this application-specific event. */
      res_event.trigger();

      /* Also call the separate response example handler. */
      res_separate.resume();
      mcu_sleep_on();
    }
#endif /* PLATFORM_HAS_BUTTON */

	if(etimer_expired(&loop_periodic_timer)) {
        mcu_sleep_off();
        loop ();
        mcu_sleep_on();
        etimer_reset(&loop_periodic_timer);
    }
  }
  PROCESS_END();
}


/*
 * VI settings, see coding style
 * ex:ts=8:et:sw=2
 */

/** @} */
