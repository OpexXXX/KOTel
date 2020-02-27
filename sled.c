#include "avr/io.h"
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/pgmspace.h>
#include "sled.h"
#include <avr/interrupt.h>

unsigned char counter_ovf=0;
unsigned int button_counter=0;


char temp_button=0;

char prev_counter_button_find=0;

char button_press=0;
char rep_button=0;
char find_button=0;
char button_long_press=0;

char key_code_num  [5][5] = {
	{ '#', 8, 4, '4',	18},
	{ '5', '*', 7, 3,	19},
	{ '0', 0, 6, 2,		20},
	{ '-', 9, 5, 1,		21},
	{ 1, 2, 3, 4,		22},
};

unsigned char number[] =
{
	0b11000000, //0
	0b11111001, //1
	0b10100100, //2
	0b10110000, //3
	0b10011001, //4
	0b10010010, //5
	0b10000010, //6
	0b11111000, //7
	0b10000000, //8
	0b10010000, //9
	0b11111111, //blank
	0b10111111, //-
	0b10001000, //A  12
	0b10000011, //b
	0b11000110, //c
	0b10100001, //d  15
	0b10000110, //e
	0b10001110, //f
	0b11000111, //L
	0b11001110, //Н
	0b10001001, //Г  20
};



unsigned int delay_button_rep=START_DELAY_REP_BUTTON;


unsigned char data_send[]=
{
	10,
	10,
	10,
	10
};
unsigned char registers[] =
{
	0b11000011,
	0b10100101,
	0b01101001,
	0b11110000,
};


void seven_led_init()
{
	//Конфигурация SPI портов
	SPI_DDR |= (MOSI_PIN | CS_PIN | SCK_PIN);      //Set control pins as outputs
	SPI_PORT &= ~(MOSI_PIN | CS_PIN | SCK_PIN);         //Set control pins low
	SPI_PORT &= ~CS_PIN;
	SPCR = (1<< SPE) | (1<< MSTR)|(0<<SPR1)|(0<<SPR0);  //Start SPI as Master

	//Установка низких уровней на регистрах
	SPDR = 0x00;
	while(!(SPSR & (1<< SPIF)));
	SPDR = 0x00;
	while(!(SPSR & (1<< SPIF)));
	SPI_PORT |= CS_PIN;
	_delay_us(1);
	SPI_PORT &= ~CS_PIN;

	//запуск таймера
	TIMSK0 = (1<<TOIE0);
	TCCR0B = (1<<CS02)|(0<<CS01)|(1<<CS00);
	TCNT0 =  0x10;

}


void set_data_dec(int dat)
{
	if (dat<1000)
	{
		data_send[3]=10;
	}
	else
	{
		data_send[3]=(dat%10000)/1000;
	}

	if (dat<100)
	{
		data_send[2]=10;
	}
	else
	{
		data_send[2]=(dat%1000)/100;
	}

	if (dat<10)
	{
		data_send[1]=10;
	}
	else
	{
		data_send[1]=(dat%100)/10;
	}
	data_send[0]=dat%10;
}

void set_data_bin(unsigned char dat )
{
	char a,b,c,d;
	a=(dat&1);
	data_send[0]=a;
	b=(dat&2);
	data_send[1]=b;
	c=(dat&4);
	data_send[2]=c;
	d=(dat&8);
	data_send[3]=d;
}

unsigned int return_data_dec()
{
	unsigned char array[4];

	for(unsigned int i=0;i<4;i++)
	{

		array[i]=data_send[i];

	}

	unsigned int result=0;

	for(unsigned int i=0;i<4;i++)
	{
		if (array[i]>10)
		{
			return 0;
		}
		if(array[i]==10)array[i]=0;
	}

	result+=(array[0]+(array[1]*10)+(array[2]*100)+(array[3]*1000));
	return result;
}

void SPI_out_shift_reg(char data,char regs)
{
	SPDR = regs;
	while(!(SPSR & (1<< SPIF)));

	SPDR = data;
	while(!(SPSR & (1<< SPIF)));

	SPI_PORT |= CS_PIN;
	_delay_us(1);
	SPI_PORT &= ~CS_PIN;


}

void sled_spi_out()
{
	SPI_out_shift_reg(number[data_send[counter_ovf]],registers[counter_ovf]);
	counter_ovf++;
	if (counter_ovf>3)counter_ovf=0;

}
unsigned char get_data_send(unsigned char index)
{
	return data_send[index];
}


//Опрос клавиатуры

void buttn_poll()
{
	unsigned char key_code=0;
	key_code=PINC;
	key_code&=0b00001111;
	key_code|=((PIND&0b00001000)<<1);

	if(key_code!=0b00011111)
	{
		switch (key_code)
		{	case 0b00011110:
			key_code=0;
			break;
			case 0b00011101:
			key_code=1;
			break;
			case 0b00011011:
			key_code=2;
			break;
			case 0b00010111:
			key_code=3;
			break;
			case 0b00001111:
			key_code=4;
			break;
			default:
			return;
		}

		if ((button_press==0)&&(!find_button))
		{
			temp_button= key_code_num[counter_ovf][key_code];
			prev_counter_button_find=counter_ovf;
			find_button=1;
			rep_button=1;
		}
		button_press=1;
	}
	else
	{
		if ((prev_counter_button_find==counter_ovf)&&button_press)
		{

			temp_button=NULL_BUTTON;
			button_press=0;
			button_long_press=0;
			button_counter=0;
			find_button=0;
			rep_button=0;
			delay_button_rep=START_DELAY_REP_BUTTON;
		}
	}
}
char btn_press()
{
	char result=0;
	result=(button_press&&rep_button);	 //(temp_button!=prev_button)&&(temp_button!=NULL_BUTTON);
	rep_button=0;
	return result;
}
void scan_button()
{


	if (button_counter>DELAY_BEFORE_REP)
	{
		rep_button=1;
		button_counter=0;
		button_long_press=1;
	}

	if (button_long_press&&(button_counter>delay_button_rep))
	{

		if (delay_button_rep<MIN_DELAY_REP_BUTTON)
		{
			delay_button_rep=MIN_DELAY_REP_BUTTON;
		}
		else
		{
			if(delay_button_rep>ACCELERATION_REP_BUTTON)
			{
				delay_button_rep-=ACCELERATION_REP_BUTTON;
			}
			else
			{
				delay_button_rep=MIN_DELAY_REP_BUTTON;
			}
		}

		rep_button=1;
		button_counter=0;
	}

}
char get_key_button()
{
	return temp_button;
}
void set_data_send(char data,unsigned char index)
{
	data_send[index]=data;
}
void inc_button_counter()
{
	buttn_poll();
	if(button_press)
	{
		button_counter++;
	}
}
unsigned int get_delay_button_pep()
{
	return delay_button_rep;
}
