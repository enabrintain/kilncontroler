/*************************************************************************
 *  Firing Schedule #1 - Digital Kilns: This Schedule can be followed for kilns with a temperature gauge and timer:
 *  From: http://www.claydemos.com/Kiln_Firing_Charts.php
 *    
 *  80 degrees an hour to 180; hold for 3-4 hours - this step is critical to keep the clay from exploding once you begin to raise the temperature.
 *  150 degrees an hour to 500; hold for ten minutes.
 *  200 degrees an hour to 1000; hold for ten minutes.
 *  300 degrees an hour to the desired temperature is reached; end of firing.
 TODO:FIX THIS COMMENT
 *
/*************************************************************************/




#include "KilnRun.h"
#include <Time.h>
#include <TimeAlarms.h>


 char* coneNames[]={"022", "021", "020", "019", "018", "017", "016", "015", "014", "013", "012", "011", "010", 
    "09", "08", "07", "06", "05", "04", "03", "02", "01",  "1",  "2",  "3",  "4",  "5",  "6",  "7", "8"}; // currenly i'm limiting it to cone 8;

 int coneTemps[] =     {1094,  1143,  1180,  1283,  1353,  1405,  1465,  1504,  1540,  1582,  1620,  1641,  1679, 
    1706, 1753, 1809, 1855, 1911, 1971, 2019, 2052, 2080, 2109, 2127, 2138, 2161, 2205, 2269, 2295, 2320, 2336, 2381}; // i included the temps for cone 9 and 10 just in case.

 char* speedNames[] = {"Fast", "Medium", "Slow", "Slow Annealing", "Medium Annealing"};

KilnRun::KilnRun()
{
  coneNDX = 0;
  candleTime = 0;// candle (raise kiln to 200F and hold it there for 2, 4, 6, 8, or 12 hours before firing - cooks out all the water
  holdHrs = 0; // hold the kiln at the target temperature for this many hours
  speedMode = fast;
  started = false;
  segment = -1; //-1 is NotStarted, 0-3 go through to top target temp
  
  adjustTimeForRoomTemp = true;
  initialSegTime = -1;
  initialSeg0 = -1;
  elapsedSeg0 = -1;
  initialSeg2 = -1;
  elapsedSeg2 = -1;
  initialSeg4 = -1;
  elapsedSeg4 = -1;
  initialSeg6 = -1;
  elapsedSeg6 = -1;
  initialSeg8 = -1;
  elapsedSeg8 = -1;
}//KilnRun

/**
 * called when the candle buton is pressed
 */
void KilnRun::candlePressed()
{
  if(12==candleTime)
  {
    candleTime = 0;
  }
  else
  {
    candleTime += 2;
  }
}// candlePressed

/**
 * called when the cone buton is pressed
 */
void KilnRun::conePressed()
{
  if(coneMaxNdx==coneNDX)
  {
    coneNDX = 0;
  }
  else
  {
    coneNDX++;
  }
}// conePressed

/**
 * called when the hold buton is pressed
 */
void KilnRun::holdPressed()
{
  holdHrs++;
  if(5==holdHrs)
  {
    holdHrs = 0;
  }
}// holdPressed

/**
 * called when the speed buton is pressed
 * "Fast", "Medium", "Slow", "Slow Annealing", "Medium Annealing"
 */
void KilnRun::speedPressed()
{
  if(4==((int)speedMode))
  {
    speedMode = slow;
  }
  else
  {
    speedMode = (Speed)(((int)speedMode)+1);
    //((int)speedMode) += 1;
  }
}// speedPressed

/**
 * called when the start buton is pressed
 */
void KilnRun::startPressed()
{
  if(started)
    return;
  started = true;
  time_t now_0 = 0;
  setTime(now_0); // set timer to 0
  segment = 1;
}// startPressed

/**
 * called when the clear buton is pressed
 */
void KilnRun::clearPressed()
{
  if(isStarted())
    return;
  coneNDX = 0;
  candleTime = 0;
  holdHrs = 0;
  speedMode = fast;
}// clearPressed

bool KilnRun::isStarted()
{
  return started;
}// isStarted

void KilnRun::firingDone()
{
  started = false;
  segment = -1;
}// firingDone

/**
 * This method drives the the firing protocol. I expect it to be called every 30sec or so. 
 * Given the current time, it picks the target temperature for that 30 second block. 
 * That temp is then fed into the PID control code and that is how the kiln is run.
 * 
 * We break the firing protocol into 9 segments, some are optional:
 * 	Segment 0 is the ramp up to candling. it runs at 80degF/hr and ends at 200degF
 *	Segment 1 is the candling stage. we hold at 200 for as many hours as the user selected
 *	Segment 2 is the ramp up to 500degF, the rate here is variable, depending on the speed selected by the user
 *	Segment 3 is a rest stage, hold at 500 for 10 min
 *	Segment 4 is the ramp up to 1000degF, the rate here is variable, depending on the speed selected by the user
 *	Segment 5 is a rest stage, hold at 1000 for 10 min
 *	Segment 6 is the ramp up to the user selected Cone temp, the rate here is variable, depending on the speed selected by the user the lowest conetemp, Cone 022, is 1094degF so we dont have to worry about Segment 4 or 5 taking us over the desired temp.
 *	Segment 7 is the Hold stage, the user can choose to hold at the selected temp for a selected number of hours
 *	Segment 8 is the annealing stage, its actually broken into 3 subsegments: cooling down to 1000degF, cooling to 800degF, cooling to 400degF. I havent decided whether to make the annealing stage cooling rates variable or not.
 *
 * This method saves stage start and elapsed times in member variables (initialSeg0, initialSeg1, ...) and (elapsedSeg0, elapsedSeg1, ...) an uses them to calculate the value of "targetTime" for a given stage, given the rate and final temp for that stage.
 * As stages' target temps are reached, the control flow drops through the various if blocks to proceding stages, incrementing "targetTime" with the elapsed times of the previous stages.
 */
double KilnRun::getTargetTemp(double currentTemp)
{ 
  if(!started)
    return -1;
  
  time_t now_ = now();
  time_t targetTime = 0;
  
  /******* Segment 0 *********/
  if(initialSeg0==-1)
    initialSeg0 = now_;
  if(initialSegTime==-1)
    initialSegTime = now_;
  int segmentTargetTemp_0 = 200;
  int segmentRate_0 = 80;
  time_t segment0 = segmentTargetTemp_0/segmentRate_0 * 3600; // number of secs it will take to raise to target temp, given the segment rate
  targetTime = initialSegTime + segment0;
  if(now_ <= targetTime)
  {
    if(adjustTimeForRoomTemp)
    {
      //adjust current time to account for room temperature
      time_t equivalentTime = (time_t)currentTemp * (3600/segmentRate_0); 
      setTime(equivalentTime);
      adjustTimeForRoomTemp = false; //do this only once
      return currentTemp+1;// 80F/hr is .66F/30sec
    }
    return now_*segmentRate_0/3600;
  }// handle heating up to candle segment
  
  if(currentTemp < segmentTargetTemp_0)
    return segmentTargetTemp_0;
  else if(elapsedSeg0==-1)
    elapsedSeg0 = now_-initialSeg0;
  targetTime = initialSegTime + elapsedSeg0; //set targetTime to the time at the end of seg0
  
  /******* Segment 1 *********/
  time_t segment1 = (candleTime * 3600); //number of seconds to hold at 200
  targetTime += segment1;
  int segmentTargetTemp_1 = segmentTargetTemp_0;
  if(now_ <= targetTime) //now is between segment0 and totalTime...
  {
    return segmentTargetTemp_1;
  }// handle the candle hold
  
  
  /******* Segment 2 *********/
  if(initialSeg2==-1)
    initialSeg2 = now_;
  int segmentTargetTemp_2 = 500;
  int segmentRate_2 = -1;
  switch(speedMode)
  {
    case fast:
      segmentRate_2 = 450; //450 F/hr *this is arbitrary-pms
      break;
    case medium:
      segmentRate_2 = 180; //180 F/hr 
      break;
    case slow:
      segmentRate_2 = 150; //150 F/hr 
      break;
    case anneal_slow:
      segmentRate_2 = 150; //150 F/hr 
      break;
    case anneal_medium:
      segmentRate_2 = 180; //150 F/hr 
      break;
    default:
      segmentRate_2 = 450; //450F/hr *this is arbitrary-pms
      break;
  }
  time_t segment2 = ((segmentTargetTemp_2-segmentTargetTemp_1)/segmentRate_2 * 3600); // number of secs it will take to raise to target temp, given the segment rate
  targetTime += segment2;
  if(now_ <= targetTime)
  {
    return (now_*(segmentRate_2/3600))+segmentTargetTemp_1;
  }// handle heating up to segment 2
  
  if(currentTemp < segmentTargetTemp_2)
    return segmentTargetTemp_2;
  else if(elapsedSeg2==-1)
    elapsedSeg2 = now_-initialSeg2;
  targetTime = initialSegTime + elapsedSeg0 + segment1 + elapsedSeg2; //set targetTime to the time at the end of seg0
  
  
  /******* Segment 3 *********/
  time_t segment3 = 600; //hold for ten minutes.
  targetTime += segment3;
  int segmentTargetTemp_3 = segmentTargetTemp_2;
  if(now_ <= targetTime) //now is between segment0 and totalTime...
  {
    return segmentTargetTemp_3;
  }// hold for ten minutes.
  
  
  /******* Segment 4 *********/
  if(initialSeg4==-1)
    initialSeg4 = now_;
  int segmentTargetTemp_4 = 1000;
  int segmentRate_4 = -1;
  switch(speedMode)
  {
    case fast:
      segmentRate_4 = 450; //450 F/hr *this is arbitrary-pms
      break;
    case medium:
      segmentRate_4 = 250; //250 F/hr 
      break;
    case slow:
      segmentRate_4 = 200; //200 F/hr 
      break;
    case anneal_slow:
      segmentRate_4 = 200; //200 F/hr 
      break;
    case anneal_medium:
      segmentRate_4 = 250; //250 F/hr 
      break;
    default:
      segmentRate_4 = 450; //450F/hr *this is arbitrary-pms
      break;
  }
  time_t segment4 = ((segmentTargetTemp_4-segmentTargetTemp_3)/segmentRate_4 * 3600); // number of secs it will take to raise to target temp, given the segment rate
  targetTime += segment4;
  if(now() <= targetTime)
  {
    return (now_*(segmentRate_4/3600))+segmentTargetTemp_3;
  }// handle heating up to segment 4
  
  if(currentTemp < segmentTargetTemp_4)
    return segmentTargetTemp_4;
  else if(elapsedSeg4==-1)
    elapsedSeg4 = now_-initialSeg4;
  targetTime = initialSegTime + elapsedSeg0 + segment1 + elapsedSeg2 + segment3 + elapsedSeg4; //set targetTime to the time at the end of seg0
  
  
  /******* Segment 5 *********/  
  time_t segment5 = 600; //hold at 1000 for ten minutes.
  targetTime += segment5;
  int segmentTargetTemp_5 = segmentTargetTemp_4;
  if(now_ <= targetTime) //now is in time segment 5...
  {
    return segmentTargetTemp_5;
  }// hold for ten minutes.
  
  
  /******* Segment 6 *********/
  if(initialSeg6==-1)
    initialSeg6 = now_;
  int segmentTargetTemp_6 = coneTemps[coneNDX];
  int segmentRate_6 = -1;
  switch(speedMode)
  {
    case fast:
      segmentRate_6 = 500; //450 F/hr *this is arbitrary-pms
      break;
    case medium:
      segmentRate_6 = 400; //250 F/hr 
      break;
    case slow:
      segmentRate_6 = 300; //200 F/hr 
      break;
    case anneal_slow:
      segmentRate_6 = 300; //200 F/hr 
      break;
    case anneal_medium:
      segmentRate_6 = 400; //250 F/hr 
      break;
    default:
      segmentRate_6 = 500; //450F/hr *this is arbitrary-pms
      break;
  }
  time_t segment6 = ((segmentTargetTemp_6-segmentTargetTemp_5)/segmentRate_6 * 3600); // number of secs it will take to raise to target temp, given the segment rate
  targetTime += segment6;
  if(now_ <= targetTime)
  {
    return (now_*(segmentRate_6/3600))+segmentTargetTemp_5;
  }// handle heating up to segment 6
  
  if(currentTemp < segmentTargetTemp_6)
    return segmentTargetTemp_6;
  else if(elapsedSeg6==-1)
    elapsedSeg6= now_-initialSeg6;
  targetTime = initialSegTime + elapsedSeg0 + segment1 + elapsedSeg2 + segment3 + elapsedSeg4 +
               segment5 + elapsedSeg6; //set targetTime to the time at the end of seg6
  
  
  /******* Segment 7 (hold at temp) *********/
  time_t segment7 = holdHrs * 3600; //hold at cone temp for ten minutes.
  targetTime += segment7;
  int segmentTargetTemp_7 = segmentTargetTemp_6;
  if(now_ <= targetTime) //now is in time segment 6...
  {
    return segmentTargetTemp_7;
  }// hold for holdHrs.
  
  /******* Segment 8 (anneal stage) *********/
  //TODO:Decide if anneal needs 2 speeds like in enum
  if(initialSeg8==-1)
    initialSeg8 = now_;
  
  if(speedMode==anneal_slow || speedMode==anneal_medium)
  {
    int segmentTargetTemp_8a = 1000;
    //Anneal I: Fast ramp down then hold to thoroughly equalize temperatures. (let cool to 1000) hold for an hour
    if(currentTemp >= segmentTargetTemp_8a)
    {
      return segmentTargetTemp_8a;
    }
    
    time_t segment8 = 3600; //hold at cone temp for 60 minutes.
    targetTime += segment8;
    if(now_ <= targetTime) //now is in time segment 8...
    {
      return segmentTargetTemp_8a;
    }// hold for holdHrs.
    if(elapsedSeg8==-1)
      elapsedSeg8= now_-initialSeg8;
    targetTime = initialSegTime + elapsedSeg0 + segment1 + elapsedSeg2 + segment3 + elapsedSeg4 +
               segment5 + elapsedSeg6; //set targetTime to the time at the end of seg6
  
    //Anneal II: Slow cool through sensitive zone, then hold to equalize. (cool to 800@150) hold for 10 min
    int segmentTargetTemp_8b = 800;
    int segmentRate_8 = -150;
    segment8 = (segmentTargetTemp_8b-segmentTargetTemp_8a)/segmentRate_8 * 3600; // drop temp to 800 @ 150 F/hr
    targetTime += segment8;
    if(now_ <= targetTime)
    {
      return (now_*(segmentRate_8/3600))+segmentTargetTemp_8a;
    }// handle cooling down to 800
    
    //Cool: Moderate ramp down to minimize thermal shock. cool to 400@300)
    int segmentTargetTemp_8c = 400;
    segmentRate_8 = -300;
    segment8 = (segmentTargetTemp_8c-segmentTargetTemp_8b)/segmentRate_8 * 3600; // drop temp to 800 @ 150 F/hr
    targetTime += segment8;
    if(now_ <= targetTime)
    {
      return (now_*(segmentRate_8/3600))+segmentTargetTemp_8b;
    }// handle cooling down to 400
  }
  
  firingDone();
  return -1.0;
}// getTargetTemp

