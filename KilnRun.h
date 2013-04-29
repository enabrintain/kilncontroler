
 #ifndef KILNRUN_H
 #define KILNRUN_H

#include <Time.h>

enum Speed       
{
    fast=0,
    medium = 1,
    slow = 2,
    anneal_slow = 3,
    anneal_medium = 4
};


class KilnRun {
  
   static const int coneMaxNdx = 29;// after this index, the coneNDX is reset to 0
   static const int maxHoldHrs = 9; // reset holdHrs to 0 if its incrimited past this by button push
    
  public:
  
 
    KilnRun(); //int8_t SCLK, int8_t CS, int8_t MISO);

    void candlePressed();
    void conePressed();
    void holdPressed();
    void speedPressed();
    void startPressed();
    void clearPressed();
    
    bool isStarted();
    void firingDone();
    double getTargetTemp(double currentTemp);

  /*double readInternal(void);
  double readInternalF(void);
  double readCelsius(void);
  double readFarenheit(void);
  uint8_t readError();//*/

 private:
  int candleTime; // candle (raise kiln to 200F and hold it there for 2, 4, 6, 8, or 12 hours before firing - cooks out all the water
  int coneNDX; // selected temperature target
  int holdHrs; // hold the kiln at the target temperature for this many hours
  Speed speedMode;
  bool started;
  int segment;
  
  bool adjustTimeForRoomTemp;
  time_t initialSegTime;
  time_t initialSeg0;
  time_t elapsedSeg0;
  time_t initialSeg2;
  time_t elapsedSeg2;
  time_t initialSeg4;
  time_t elapsedSeg4;
  time_t initialSeg6;
  time_t elapsedSeg6;
  time_t initialSeg8;
  time_t elapsedSeg8;
};

 #endif 
