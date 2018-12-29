#include "bsides2019.h"

// returns battery capacity in 1-4 steps
int power_get_battery_level()
{ 
   ADCSRA |= bit( ADSC );
   while (ADCSRA & bit (ADSC))
     { }
   ADCSRA |= bit( ADSC );
   while (ADCSRA & bit (ADSC))
     { }
   // 1062 is a magic number and requires calibration
   // this will be good enough for now
   int voltage = (((1062 * 1024) / ADC) + 5) / 10;

   int level = 0;

   if (voltage > 2500) {
	level = 1;
   } else if (voltage > 2700) {
        level = 2;
   } else if (voltage > 2800) {
        level = 3;
   } else if (voltage > 2900) {
        level = 4;
   }
   return level;
}
