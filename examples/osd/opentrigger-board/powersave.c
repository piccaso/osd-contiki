#include "powersave.h"
#include "net/netstack.h"

#ifndef POWERSAVE_COUNTER_MAX
#define POWERSAVE_COUNTER_MAX (1000u)
#endif

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

#define COUNTER_MAX ((unsigned short) POWERSAVE_COUNTER_MAX)

static unsigned short _counter = 0;
static unsigned short _low_power = 0;

void powersave_loop(void){
  if(_low_power == 1) return;
  _counter++;
  if(DEBUG >= 2) {
    PRINTF("powersave _counter = %i/%i\n", _counter, COUNTER_MAX);
  }
  if(_counter >= COUNTER_MAX){
    // 2 minutes since COUNTER_MAX cycles, go to low power mode;
    // TODO: how to do that? `NETSTACK_MAC.off(0);` kills network
    _low_power = 1;
    PRINTF("powersave _low_power = %i\n",_low_power);
  }
}

void powersave_reset(void){
  _counter = 0;
  _low_power = 0;
  PRINTF("powersave _low_power = %i\n",_low_power);
  // TODO: is this the right call?
  NETSTACK_MAC.on();
}
