#include "BH1750FVI.h"
//#if ARDUINO >= 100
 #include "Arduino.h"
//#else
// #include "WProgram.h"
//#endif

#include <Wire.h>

void BH1750FVI::begin(byte mode, byte addr, double sens, int pin){
  if(pin>=0 && pin<55){
    pinMode(pin, OUTPUT);
    if(addr==BH_AddrL) digitalWrite(pin, LOW);
    if(addr==BH_AddrH) digitalWrite(pin, HIGH);
  }
  byte sensitivity;
  if(sens<BH_MinSv){
    sens = constrain(sens, ((double)BH_MinSv/(double)BH_DefSv), ((double)BH_MaxSv/(double)BH_DefSv));
    sensitivity = (byte)round(sens*(double)BH_DefSv);
  }
  else{
    sensitivity = constrain((byte)sens, BH_MinSv, BH_MaxSv);
  }
  address = addr;
  currentSensitivity = sensitivity;
  MTreg_hiByte = BH_MTrHb | (sensitivity >> 5);
  MTreg_loByte = BH_MTrLb | (sensitivity & 0b00011111);
  switch(mode){
    case BH_ContL:
    case BH_ContH:
    case BH_Conth:
      startOngoingSamples(mode);
      break;
    case BH_SingH:
    case BH_Singh:
    case BH_SingL:
      startSingleSample(mode);
      break;
    default:
      mode = BH_EasyH;
      startOngoingSamples(mode);
      break;
  }
  sampleUnseen = false;
  lastSampleStart = 0;
}

word BH1750FVI::startOngoingSamples(char mode){
  if(mode==BH_NoMod) mode=currentMode;
  byte lowLevelMode = mode;
  if(lowLevelMode==BH_EasyH) lowLevelMode=BH_ContH;
  switch(mode){
    case BH_EasyH:
    case BH_ContH:
    case BH_Conth:
    case BH_ContL:
      i2cWrite(BH_PowOn);
      i2cWrite(MTreg_hiByte);
      i2cWrite(MTreg_loByte);
      i2cWrite(lowLevelMode);
      lastSampleStart = millis();
      currentMode = mode;
      return setSensitivity(currentSensitivity);
      break;
    default:
      return 0;
  }
}

word BH1750FVI::startSingleSample(char mode){
  if(mode==BH_NoMod){
    mode=currentMode;
  }
  switch(mode){
    case BH_SingH:
    case BH_Singh:
    case BH_SingL:
      i2cWrite(BH_PowOn);
      i2cWrite(MTreg_hiByte);
      i2cWrite(MTreg_loByte);
      i2cWrite(mode);
      lastSampleStart = millis();
      currentMode = mode;
      return setSensitivity(currentSensitivity);
      break;
    default:
      return 0;
  }
}

bool BH1750FVI::sampleIsFresh(){
  if(sampleUnseen) return true;
  else if((millis() > (lastSampleStart+currentSamplingTime))){
    sampleUnseen = true;
    return true;
  }
  else return false;
}

word BH1750FVI::getLightLevel(char mode){
  unsigned long int Intensity_value;
  sampleUnseen = false;
  Intensity_value = i2cRead();
  if(mode!='r'){
    Intensity_value = (Intensity_value*5)/6;
    if(mode!='c'){
      Intensity_value = (Intensity_value*69)/currentSensitivity;
    }
    /*this next adjustment is not useful in practice. if you're using
      a half-scale mode, you want the extra sensitivity. I'm leaving it
      here for reference.
    switch(currentMode){
      case BH_Singh:
      case BH_Conth:
        Intensity_value /= 2;
    }*/
    Intensity_value = constrain(Intensity_value, 0, 65535);
  }
  switch(currentMode){
    case BH_ContH:
    case BH_Conth:
    case BH_ContL:
    case BH_EasyH:
      //this is a fake previous sample time that will keep pace with
      //the actual BH1750FVI's sampling rate, but isn't necessarily
      //synchronized to the earliest time each new sample is available.
      //if you want true synchronized timing, use a single-sample mode.
      lastSampleStart = millis();
  }
  return (word)Intensity_value;
}

word BH1750FVI::setSensitivity(double sens){
  if(sens<((double)(BH_MinSv/2))){
    sens = constrain(sens,
                    ((double)BH_MinSv/(double)BH_DefSv),
                    ((double)BH_MaxSv/(double)BH_DefSv)
                    );
    sens = sens*(double)BH_DefSv;
  }
  else{
    sens = constrain(sens, (double)BH_MinSv, (double)BH_MaxSv);
  }
  return setSensitivity((byte)round(sens));
}

word BH1750FVI::setSensitivity(float sens){
  return setSensitivity((double)sens);
}

word BH1750FVI::setSensitivity(int sens){
  sens = constrain(sens, 1, BH_MaxSv);
  return setSensitivity((byte)sens);
}

word BH1750FVI::setSensitivity(byte sens){
  if(sens<(BH_MinSv/2)){  
    sens = BH_DefSv*(constrain(sens, (byte)1, (byte)3));
  }
  else{
    sens = constrain(sens, BH_MinSv, BH_MaxSv);
  }
  switch(currentMode){
    case BH_ContL:
    case BH_SingL:
      currentSamplingTime = (word)((BH_FastD*(unsigned long int)sens)/BH_DefSv);
      break;
    default:
      currentSamplingTime = (word)((((unsigned long int)BH_SlowD)*((unsigned long int)sens))/BH_DefSv);
    break;
  }
  MTreg_hiByte = BH_MTrHb | (sens >> 5);
  MTreg_loByte = BH_MTrLb | (sens & 0b00011111);
  currentSensitivity = sens;
  sampleUnseen = false;
  return currentSamplingTime;
}

void BH1750FVI::powerDown(){
  i2cWrite(BH_PowOf);
}

/*I2C INTERFACE
  These two functions were originally based on code written by
  https://github.com/claws/ and
  https://github.com/Genotronex/
*/
void BH1750FVI::i2cWrite(byte dataToSend){
  Wire.beginTransmission(address);
  Wire.write(dataToSend);
  Wire.endTransmission();
}

word BH1750FVI::i2cRead(){
  word value;
  Wire.beginTransmission(address);
  Wire.requestFrom(address, (byte)2);
  if(Wire.available()){
    value = Wire.read();
    if(Wire.available()){
      value <<= 8;
      value |= Wire.read();
    }
    else value=0;
  }
  else value=0;  
  Wire.endTransmission();
  return value;
}
