#include <avr/io.h>
#include <stdio.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include "lcd.h"
#include <string.h>
#include "coil.h"


void ADC_Init()
{
    ADCSRA |= (1 << ADEN) // Включаем АЦП
              |(1 << ADPS1)|(1 << ADPS0);    // устанавливаем предделитель преобразователя на 8
    ADMUX |= (0 << REFS1)|(1 << REFS0) //выставляем опорное напряжение, как внешний ИОН
             |(0 << MUX0)|(0 << MUX1)|(0 << MUX2)|(0 << MUX3)|(0<<ADLAR); // снимать сигнал будем с  входа PC0
}


int button_poll()
{
    unsigned int u=0;

    ADCSRA |= (1 << ADSC);    // Начинаем преобразование
    while(ADCSRA&0x64){}
    u = (ADCL|ADCH << 8); // Считываем  полученное значение


    if(u>900)
    {

        return 0;

    }
    else if ((u<70))
    {

        return 1;
    }
    else if ((u<300))
    {

        return 2;
    }
    else if ((u<340))
    {

        return 3;
    }
    else if ((u<600))
    {

        return 4;
    }
    else if ((u<900))
    {

        return 5;
    }
    return 0;
}
