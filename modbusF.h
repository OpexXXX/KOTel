/*
 *
 *
 * Created: 14.12.2015 19:40:39
 *  Author: Opex
 */ 
#include <avr/io.h>



#ifndef _MBF_H
#define _MBF_H

#define REG_DISCRETE_START      10               //Адрес в сети MODBUS
#define REG_DISCRETE_SIZE       32                // Кол-во

#define REG_COILS_START         100
#define REG_COILS_SIZE          32

#define REG_INPUT_START         500
#define REG_INPUT_NREGS        32

#define REG_HOLDING_START       1000
#define REG_HOLDING_NREGS      32






#endif  // ST_H