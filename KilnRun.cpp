
#include "KilnRun.h"


 char* coneNames[]={"022", "021", "020", "019", "018", "017", "016", "015", "014", "013", "012", "011", "010", 
    "09", "08", "07", "06", "05", "04", "03", "02", "01",  "1",  "2",  "3",  "4",  "5",  "6",  "7", "8"}; // currenly i'm limiting it to cone 8;

 int coneTemps[] =     {1094,  1143,  1180,  1283,  1353,  1405,  1465,  1504,  1540,  1582,  1620,  1641,  1679, 
    1706, 1753, 1809, 1855, 1911, 1971, 2019, 2052, 2080, 2109, 2127, 2138, 2161, 2205, 2269, 2295, 2320, 2336, 2381}; // i included the temps for cone 9 and 10 just in case.

 char* speedNames[] = {"Fast", "Medium", "Slow", "Slow Annealing", "Medium Annealing"};

KilnRun::KilnRun()
{
  coneNDX = 0;
  
  holdHrs = 0; // hold the kiln at the target temperature for this many hours
  speedMode = fast;
}//KilnRun

/**
 * called when the candle buton is pressed
 */
void KilnRun::candlePressed()
{
  
}// candlePressed

/**
 * called when the cone buton is pressed
 */
void KilnRun::conePressed()
{
  
}// conePressed

/**
 * called when the hold buton is pressed
 */
void KilnRun::holdPressed()
{
  
}// holdPressed

/**
 * called when the speed buton is pressed
 */
void KilnRun::speedPressed()
{
  
}// speedPressed

/**
 * called when the start buton is pressed
 */
void KilnRun::startPressed()
{
  
}// startPressed

/**
 * called when the clear buton is pressed
 */
void KilnRun::clearPressed()
{
  
}// clearPressed



