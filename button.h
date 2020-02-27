#ifndef BUTTON_H_INCLUDED
#define BUTTON_H_INCLUDED

#include <avr/io.h>
#include <stdio.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include "lcd.h"
#include <string.h>
#include "coil.h"

void ADC_Init();
int button_poll();
#endif // BUTTON_H_INCLUDED
