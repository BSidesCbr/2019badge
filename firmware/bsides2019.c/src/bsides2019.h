#ifndef BSIDES2019_H_INCLUDED
#define BSIDES2019_H_INCLUDED

// useful hardware macros

#define P_CPU_NS (1000000000UL / F_CPU)
#define CALL_CYCLES 26UL
#define CALC_CYCLES 4UL
#define RETURN_CYCLES 4UL
#define CYCLES_PER_LOOP 4UL

// common includes

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <stdio.h>
#include <u8g2.h>
#include <util/delay.h>

// program includes

#include "config.h"
#include "display.h"
#include "menu.h"
#include "power.h"

// logos and graphics

#include "logos/bsidescbr.h"
#include "logos/nopia.h"

// globals
extern u8g2_t u8g2;

#endif /* BSIDES2019_H_INCLUDED */
