
#include "button.h"
char PrevousButton=0;
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


    if(u>900 && PrevousButton)
    {
		char res = PrevousButton;
		PrevousButton =0;
        return res;
    }
    else if ((u<120))
    {
PrevousButton =1; //right
      
    }
    else if ((u<300))
    {

    PrevousButton =2;// up
    }
    else if ((u<380))
    {

      PrevousButton =3;// down
    }
    else if ((u<700))
    {

      PrevousButton =4;//left
    }
    else if ((u<900))
    {

       PrevousButton =5;//select
    }
    return 0;
}
