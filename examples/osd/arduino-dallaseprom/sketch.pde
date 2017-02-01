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

#include <OneWire.h>
#include <DallasEPROM.h>

extern "C" {

#include "arduino-process.h"
#include "rest-engine.h"
#include "sketch.h"

extern volatile uint8_t mcusleepcycle;  // default 16

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 3

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
DallasEPROM de(&oneWire);

extern resource_t res_dtemp1, res_dtemp2, res_battery;

#define LED_PIN 4
}


void setup (void)
{
    // switch off the led
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    printf("Dallas Eprom Control Library Demo");

    Serial1.begin(38400);
    // init coap resourcen
    rest_init_engine ();
    rest_activate_resource (&res_battery, "s/batter");
}

// at project-conf.h
// LOOP_INTERVAL		(10 * CLOCK_SECOND)
void loop (void)
{
  byte buffer[32];  // Holds one page of data
  int status;
      
  mcu_sleep_off();

  // Search for the first compatible EPROM/EEPROM on the bus.
  // If you have multiple devices you can use de.setAddress()
  de.search();

  // Print out the 1-wire device's 64-bit address
  Serial1.print("Address=");
  for(int i = 0; i < 8; i++) {
    Serial1.print(de.getAddress()[i], HEX);
    Serial1.print(" ");
  }
  Serial1.println("");

  if (de.getAddress()[0] == 0x00) {
    Serial1.println("No device was found!");
  } else {
    if (de.validAddress(de.getAddress())) {
      Serial1.println("Address CRC is correct.");

      // Uncomment to write to the first page of memory
      //strcpy((char*)buffer, "allthingsgeek.com");
      //if ((status = de.writePage(buffer, 0)) != 0) {
        //sprintf((char*)buffer, "Error writing page! Code: %d", status);
        //Serial1.println((char*)buffer);
      //}

      // Read the first page of memory into buffer
      if ((status = de.readPage(buffer, 0)) == 0) {
        Serial1.println((char*)buffer);
      } else {
        sprintf((char*)buffer, "Error reading page! Code: %d", status);
        Serial1.println((char*)buffer);
      }
    } else {
      Serial1.println("Address CRC is wrong.");
    }
  }
  Serial1.println("");

  mcu_sleep_on();
}
