#include <Wire.h>
#include <BH1750FVI.h>

BH1750FVI eye;
word maxLux = 0;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  //most basic setup with all default options
  eye.begin();
}

void loop() {
  //take a sensor reading, with units of Lux
  word value = eye.getLightLevel();
  byte spaces;
  if(value>maxLux) maxLux=value;
  spaces=map(value, 0, maxLux, 0, 32);
  for(int i=0; i<spaces; i++) Serial.print(' ');
  Serial.println(value);
  delay(100);
}
