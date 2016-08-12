#ifndef BH1750FVI_h
#define BH1750FVI_h

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <Wire.h>

//device address options
#define BH_AddrL  0x23 // Device address when addr pin is LOW
#define BH_AddrH  0x5C // Device address when addr pin is HIGH

//power codes 
#define BH_PowOf  0x00 // power-off code
#define BH_PowOn  0x01 // power-on code
#define BH_Reset  0x07 // code to reset the light reading register to zero

//mode codes
#define BH_NoMod  0x02 // No mode specified. Keeps mode the same as before.
#define BH_EasyH  0x03 // 'easy' mode. basically the same as ContH
#define BH_ContH  0x10 // continuous full-scale, high-resolution sampling
#define BH_Conth  0x11 // continuous with twice the resolution but half the max value
#define BH_ContL  0x13 // continuous full-scale, low-resolution sampling. faster.
#define BH_SingH  0x20 // take one high-resolution, full-scale sample then turn off
#define BH_Singh  0x21 // take one high-resolution, half-scale sample then turn off
#define BH_SingL  0x23 // take one low-resolution, full-scale sample then turn off
                       // (happens to be the same as the low address value)
                       
//sensitivity codes
#define BH_MTrHb  0x40 // last 3 bits must be masked to 3 MSB of desired value
#define BH_MTrLb  0x60 // last 5 bits must be masked to 5 LSB of desired value
#define BH_MinSv    31 // minimum sensitivity register value, yields Lux * 0.45
#define BH_MaxSv   254 // maximum sensitivity register value, yields Lux * 3.68
#define BH_DefSv    69 // default sensitivity register value, yields Lux * 1.00

//timing delays. they're actually longer than the datasheet says.
#define BH_FastD    18 // basic delay in microseconds for a fast sample
#define BH_SlowD   125 // basic delay in microseconds for a slow sample

class BH1750FVI {
  public:
  /*BEGIN
    this is intended to be as error-tolerant as possible. if you don't supply
    a mode, the default mode will be 'easy'. the sensitivity obviously defaults
    to 1.0 (byte value of 69). Default address selection is LOW, which is how
    the BH1750FVI should default if you don't connect anything to its ADDR pin.
  */void begin(byte mode=BH_EasyH, byte addr=BH_AddrL, double sens=BH_DefSv, int addrPin=-1);
    
  /*FRESH SAMPLE POLLING
    Check whether you have already looked at the current sample.
  */bool sampleIsFresh();

  /*ACTUALLY GET A READING FROM THE SENSOR
    This is probably all you wanted from this library. Return value is an
    unsigned 16 bit integer with units of Lux. There are three possible modes:
    'r' raw numerical reading, no units, you get what you get
    'c' convert units to Lux or half-Lux, depending on the mode, but don't
        compensate for the sensitivity setting. This allows sensitivity to
        be used as enclosure optics calibration/compensation.
    't' true Lux (or half-Lux) reading, regardless of sensitivity settings.
  */word getLightLevel(char adjustments='c');

    word retrieveSample();

  /*CONTINUOUS SAMPLING
    Return value of this start function is the delay time between refreshes of
    the ambient light reading. After you call this function, you can use getLightLevel()
    to read the most recent ambient light level. If you try to run this ongoing start
    function with a one-time mode argument, it will do nothing and return 0.
  */word startOngoingSamples(char mode=BH_NoMod);

  /*ONE-TIME SAMPLE START
    Return value of this start function is the delay time before the new ambient
    light reading will be available. You can check whether the new sample is ready
    yet using sampleIsFresh(). Retrieve that single sample with getLightLevel().
    If you give this one-time start function a continuous mode argument, it will
    do nothing and return 0.
  */word startSingleSample(char mode=BH_NoMod);

  /*SENSITIVITY SETTINGS
    This function lets you change the 'sensitivity' of the BH1750FVI by changing
    its sampling time up or down. The sensitivity is stored as a number between
    31 and 254, with a default value of 69. This is intended to compensate for
    having the chip installed in an enclosure that blocks some ambient light.
    The floating point versions of the function allow the sensitivity to be
    given as a ratio between ~0.45 and ~3.68. The return value is the number of
    milliseconds that the chip will take to create a new light level reading.
  */word setSensitivity(double sens);
    word setSensitivity(float sens);
    word setSensitivity(int sens);
    word setSensitivity(byte sens=BH_DefSv);
    
  /*POWERDOWN FUNCTION
    In case you want to turn off the BH1750FVI to save power. Other commands will wake
    the chip back up. Single sample modes automatically turn off the chip after sampling,
    but you can still read the value from it without waking it up.
  */void powerDown();
    
  private:
    void init(byte mode, byte addr, byte sens);
    void i2cWrite(byte dataToSend);
    word i2cRead();
    byte address;
    byte currentMode;
    byte currentSensitivity;
    bool sampleUnseen;
    byte MTreg_hiByte;
    byte MTreg_loByte;
    unsigned long int lastSampleStart;
    word currentSamplingTime;
};
#endif