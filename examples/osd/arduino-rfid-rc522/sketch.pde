/*
 * Sample arduino sketch using contiki features.
 * We turn the LED off 
 * Unfortunately sleeping for long times in loop() isn't currently
 * possible, something turns off the CPU (including PWM outputs) if a
 * Proto-Thread is taking too long. We need to find out how to sleep in
 * a Contiki-compatible way.
 * Note that for a normal arduino sketch you won't have to include any
 * of the contiki-specific files here, the sketch should just work.
 */

/*
 * ------------------------------------------------------------------------------------------------------------$
 * Example sketch/program showing how to read new NUID from a PICC to serial.
 * ------------------------------------------------------------------------------------------------------------$
 * This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalb$
 *
 * Example sketch/program showing how to the read data from a PICC (that is: a RFID Tag or Card) using a MFRC52$
 * Reader on the Arduino SPI interface.
 *
 * When the Arduino and the MFRC522 module are connected (see the pin layout below), load this sketch into Ardu$
 * then verify/compile and upload it. To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M)$
 * you present a PICC (that is: a RFID Tag or Card) at reading distance of the MFRC522 Reader/PCD, the serial o$
 * will show the type, and the NUID if a new card has been detected. Note: you may see "Timeout in communicatio$
 * when removing the PICC from reading distance too early.
 *
 * @license Released into the public domain.
 *
 * Typical pin layout used:
 * -------------------------------------
 *             MFRC522      Merkurboard 
 *             Reader/PCD   ATmega      
 * Signal      Pin          Pin         
 * -------------------------------------
 * RST/Reset   RST          D14
 * SPI SS      SDA(SS)      D10
 * SPI MOSI    MOSI         D11
 * SPI MISO    MISO         D13
 * SPI SCK     SCK          D12
 */

extern "C" {
#include "arduino-process.h"
#include "rest-engine.h"
#include "net/netstack.h"

extern resource_t res_led, res_battery, res_cputemp;

uint8_t led_pin=4;
uint8_t led_status;
}

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN  10
#define RST_PIN 14

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;

// Init array that will store new NUID
byte nuidPICC[3];

void setup (void)
{
    // switch off the led
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, HIGH);
    led_status=0;
    Serial1.begin(38400);
    // RFID Init
    SPI.begin(); // Init SPI bus
    rfid.PCD_Init(); // Init MFRC522
    for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
    }
    // init coap resourcen
    rest_init_engine ();
    rest_activate_resource (&res_led, "s/led");
    rest_activate_resource (&res_battery, "s/battery");
    rest_activate_resource (&res_cputemp, "s/cputemp");
    
 //   NETSTACK_MAC.off(1);
}

/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial1.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial1.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial1.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial1.print(buffer[i], DEC);
  }
}

void loop (void)
{
  // Look for new cards
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial1.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial1.println(rfid.PICC_GetTypeName(piccType));
  
  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial1.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }
  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial1.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }
   
    Serial1.println(F("The NUID tag is:"));
    Serial1.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial1.println();
    Serial1.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial1.println();
  }
  else Serial1.println(F("Card read previously."));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}
