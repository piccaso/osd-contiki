#include <Wire.h>
#include <BH1750FVI.h>

BH1750FVI eye;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  //setup with continuous sampling, using the low address and double sensitivity
  eye.begin(BH_ContH, BH_AddrL, 2.0);
}

void loop() {
  //test if the sensor has had time to produce a new reading
  if(eye.sampleIsFresh()){
    //read the calibration-adjusted value, according to the sensitivity setting
    word value = eye.getLightLevel('c');
    logarithmicGraphing(value);
    Serial.println(value);
  }
}




void logarithmicGraphing(word value){
  int i=0;
  while(value>>i){
    i++;
    Serial.print("  ");
  }
  if(i>=2){
    i=i-2;
    if(value&(1<<i))Serial.print(' ');
  }
}
