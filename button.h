#ifndef BUTTON_H_INCLUDED
#define BUTTON_H_INCLUDED

#include <avr/io.h>
#include <stdio.h>
#define F_CPU 16000000UL
#include <string.h>


void ADC_Init();
int button_poll();





#endif // BUTTON_H_INCLUDED
