#ifndef COIL_H_INCLUDED
#define COIL_H_INCLUDED

#include <avr/io.h>
#include <stdio.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include <string.h>

#define COIL4_OFF PORTB|=0x04
#define COIL4_ON PORTB&=0xFB
#define COIL3_OFF PORTB|=0x02
#define COIL3_ON PORTB&=0xFD
#define COIL1_OFF PORTC|=0x02
#define COIL1_ON PORTC&=0xFD
#define COIL2_OFF PORTC|=0x04
#define COIL2_ON PORTC&=0xFB
#define COIL5_OFF PORTC|=0x08
#define COIL5_ON PORTC&=0xF7
#define COIL6_OFF PORTC|=0x10
#define COIL6_ON PORTC&=0xEF

void update_coil_state(unsigned char*);
#endif // COIL_H_INCLUDED
