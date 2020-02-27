#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdio.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

#include "lcd.h"
#include "coil.h"
#include "button.h"

#include "MicroMenu.h"
#include "MenuConfig.h"

#include "mb.h"			//Модбас библиотеки
#include "mbport.h"
#include "mbutils.h"

#include "OWIPolled.h"				//1 wire áèáëèîòåêè
#include "OWIHighLevelFunctions.h"
#include "OWIBitFunctions.h"
#include "common_files/OWIcrc.h"

#include "bcd.h" //Мат функции

//прототипы функций
unsigned char DS18B20_ReadTemperature(unsigned char bus, unsigned char * id, unsigned int* temperature);    //Заголовок чтения температуры
void SensorFound();
void timerInit();
char get_temperature();
void temer_poll();
unsigned char* lcd_menuText(int8_t menuShift);
char * utoa_builtin_div(uint32_t value, char *buffer);
void Termostat();

#define AUTO_REG_FLAG ucRegCoilsBuf[0] & 0b01000000
#define ALL_TEN_OFF ucRegCoilsBuf[0]&=0b11110001
#define TARGET_TEMP usHoldingBuf[6]
#define CURRENT_TEMP usHoldingBuf[2]

#define TEN1_ON ucRegCoilsBuf[0]|=0b00000010
#define TEN2_ON ucRegCoilsBuf[0]|=0b00000100
#define TEN3_ON ucRegCoilsBuf[0]|=0b00001000
#define TEN1_OFF ucRegCoilsBuf[0]&=0b11111101
#define TEN2_OFF ucRegCoilsBuf[0]&=0b11111011
#define TEN3_OFF ucRegCoilsBuf[0]&=0b11110111

#define REFRESH_TEMP_DELAY   60  //Переодичность опроса датчиков температуры
#define BUS OWI_PIN_3						//Пин шины 1 wire
#define MAX_DEVICES       0x04				//Количество датчиков на шине
//коды ошибок для функции чтения температуры
#define READ_SUCCESSFUL   0x00
#define READ_CRC_ERROR    0x01
#define READ_WAIT         0x02
//Статусы поиска датчиков 1Wire
#define SEARCH_SENSORS 0x00
#define SENSORS_FOUND 0xff
//коды семейства и коды команд датчика DS18B20
#define DS18B20_FAMILY_ID                0x28
#define DS18B20_CONVERT_T                0x44
#define DS18B20_READ_SCRATCHPAD          0xbe
#define DS18B20_WRITE_SCRATCHPAD         0x4e
#define DS18B20_COPY_SCRATCHPAD          0x48
#define DS18B20_RECALL_E                 0xb8
#define DS18B20_READ_POWER_SUPPLY        0xb4

// Пид регулирование
#define TERMOSTAT_PERIOD usHoldingBuf[11] //период воздействия

#define K_prop usHoldingBuf[9]  //Пропорциональный коэфициетнт
#define K_integ usHoldingBuf[10]// Интегральный коэффициент
#define PWM_intens usRegInputBuf[3]
#define K_dif usHoldingBuf[13]
#define VOZDEISTVIE usHoldingBuf[12]

int i_imbalance=0;  //Интегрирование несоответствия

unsigned int timer_termostat_step=0;  //Таймер термостата
unsigned int timer_termostat_second_step=0;
unsigned int timer_termostat_PWM=0;  //Таймер ШИМ термостата

char thermostat_step_falg=1;    //флаг таймера  температуры

int t_prev=2430;                   // Температура прошлого шага



/*------------------------------Переменные 1 wire-------------------------------*/
static USHORT temperature = 0;
int usTempInputBuf[6][6];                  //Массивы буфера усреднения

unsigned char searchFlag = SEARCH_SENSORS;
unsigned char crcFlag = 0;
unsigned char num = 0;      // Количество найденных датчиков
unsigned char scratchpad[9];
unsigned char index_of_read_ds=0; //индекс датчика
//Объект структуры содержащий id найденных датчиков
OWI_device allDevices[MAX_DEVICES];
/*------------------------------------------------------------------------------*/

//#define F_CPU 1600000UL
//----------------------- Defines MODBUS ------------------------------------------

#define F_CPU 16000000UL

#define REG_DISCRETE_START      10               //Адрес в сети MODBUS
#define REG_DISCRETE_SIZE       32                // Кол-во

#define REG_COILS_START         100
#define REG_COILS_SIZE          32

#define REG_INPUT_START         500
#define REG_INPUT_NREGS        64

#define REG_HOLDING_START       1000
#define REG_HOLDING_NREGS      64

#define RTS_ENABLE							// Активация пина RST


/* ----------------------- Static variables MODBUS---------------------------------*/

static USHORT usRegInputBuf[REG_INPUT_NREGS];                  //Массивы переменных MODBUS
static USHORT usHoldingBuf[REG_HOLDING_NREGS];
static unsigned char ucRegCoilsBuf[REG_COILS_SIZE / 8];
static unsigned char ucRegDiscreteBuf[REG_DISCRETE_SIZE / 8];
/*----------------------------------------------------------------------------------*/
char step_tim=1;            //флаг таймера опроса температуры
unsigned int timer_step=0;  //Таймер опроса температуры



char delay_timer_flag=0;
unsigned int timer_delay_ds=0;

int temperature_first=0; //Температура первый датчик

/*---------------MicroMenu--------------------------------*/
#define strNULL 0x01
//MENU_ITEM(Name, Next, Previous, Parent, Child, SelectFunc, EnterFunc, Text)
MENU_ITEM(Menu_1, Menu_2, Menu_3, NULL_MENU, Menu_1_1 , NULL, NULL, "1");
MENU_ITEM(Menu_2, Menu_3, Menu_1, NULL_MENU, NULL_MENU, NULL, NULL  , "2");
MENU_ITEM(Menu_3, Menu_1, Menu_2, NULL_MENU, NULL_MENU, NULL, NULL   , "3");

MENU_ITEM(Menu_1_1, Menu_1_2, Menu_1_2, NULL_MENU, NULL_MENU, NULL, NULL, "Kotel");
MENU_ITEM(Menu_1_2, Menu_1_1, Menu_1_1, NULL_MENU, NULL_MENU, NULL, NULL, "v1.0");

static void Generic_Write(const char* Text)
{


}

/*------------------------------------------------------------------------------*/



/* инициализация портов,
подключенных к жки */
void init_port()
{
	DDRC= 0b00011110;
	DDRB = 0b00011110;
	PORTB = 0b00011110;
	PORTC = 0b00011110;
	PORTD=0x00;
	DDRD=0b11110000;
}

int main(void)
{
	//инициализация портов
	init_port();
	//инициализация таймера
	timerInit();
	//инициализация 1ваир
	OWI_Init(BUS);
	////Инициализация MODBUS
	const UCHAR ucSlaveID[] = { 0xAA, 0xBB, 0xCC };
	eMBInit(MB_RTU, 0x0A, 0, 9600, MB_PAR_NONE);
	eMBSetSlaveID(0x34, TRUE, ucSlaveID, 3);
	sei( );
	eMBEnable();

	//инициализация АЦП для опроса клавиатуры
	ADC_Init();

	//инициализация дисплея
	lcd_init();
	lcd_gotoxy(0,0);
	/* lcd_putstring(buf);
	int btn_temp=0;
	lcd_gotoxy(0,1);
	lcd_putstring("opex");*/

	/*----MicroMenu------ВРЕМЕННО */
	Menu_SetGenericWriteCallback(Generic_Write);
	Menu_Navigate(&Menu_1_1);
	lcd_menuText(0);
	Menu_Navigate(MENU_NEXT);
	lcd_menuText(0);
	lcd_gotoxy(0,1);
	Menu_Navigate(MENU_NEXT);
	lcd_menuText(0);
	Menu_Navigate(MENU_NEXT);
	lcd_menuText(0);
	/*----MicroMenu------ВРЕМЕННО */
	usHoldingBuf[7]=10;

	TARGET_TEMP=2430;
	K_integ=1;
	K_prop=10;
	K_dif=1;
	TERMOSTAT_PERIOD = 1800;

	/*----------------------*/
	while(1)
	{

		SensorFound();//Поиск датчиков
		temer_poll();//обdновление температуры
		Termostat();// Регулировка температуры
		update_coil_state(ucRegCoilsBuf);//Обновление состояний реле
		(void) eMBPoll();// Основная функция mobbus
	}
	return 0;
}


/*Обработчик INPUTS*/
eMBErrorCode eMBRegInputCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs)
{
	eMBErrorCode eStatus = MB_ENOERR;

	uint16_t i;

	if (usAddress < REG_INPUT_START || usAddress + (usNRegs - 1) > REG_INPUT_START+REG_INPUT_NREGS) return MB_ENOREG;

	usAddress -= REG_INPUT_START;

	for (i = 0; i < usNRegs; i++)
	{

		pucRegBuffer[i * 2 + 0] = (unsigned char) (usRegInputBuf[i + usAddress] >> 8);
		pucRegBuffer[i * 2 + 1] = (unsigned char) (usRegInputBuf[i + usAddress] & 0xFF);


	}


	return eStatus;
}

/*Обработчик HOLDINGS*/
eMBErrorCode eMBRegHoldingCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode)
{

	uint16_t i;

	if (usAddress < REG_HOLDING_START || usAddress + (usNRegs - 1) > REG_HOLDING_START+REG_HOLDING_NREGS) return MB_ENOREG;

	usAddress -= REG_HOLDING_START;

	for (i = 0; i < usNRegs; i++)
	{

		if (eMode == MB_REG_READ)
		{

			pucRegBuffer[i * 2 + 0] = (unsigned char) (usHoldingBuf[i + usAddress] >> 8);
			pucRegBuffer[i * 2 + 1] = (unsigned char) (usHoldingBuf[i + usAddress] & 0xFF);

		}
		else if (eMode == MB_REG_WRITE)
		{

			usHoldingBuf[i + usAddress] = (pucRegBuffer[i * 2 + 0] << 8) | pucRegBuffer[i * 2 + 1];

		}

	}

	return MB_ENOERR;
}

/*Обрабртчик COILS*/
eMBErrorCode eMBRegCoilsCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode)
{
	eMBErrorCode    eStatus = MB_ENOERR;
	int             iNCoils = ( int )usNCoils;
	unsigned short  usBitOffset;

	/* Check if we have registers mapped at this block. */
	if( ( usAddress >= REG_COILS_START ) &&
	( usAddress + usNCoils <= REG_COILS_START + REG_COILS_SIZE ) )
	{
		usBitOffset = ( unsigned short )( usAddress - REG_COILS_START );
		switch ( eMode )
		{
			/* Read current values and pass to protocol stack. */
			case MB_REG_READ:
			while( iNCoils > 0 )
			{
				*pucRegBuffer++ =
				xMBUtilGetBits( ucRegCoilsBuf, usBitOffset,
				( unsigned char )( iNCoils >
				8 ? 8 :
				iNCoils ) );
				iNCoils -= 8;
				usBitOffset += 8;
			}
			break;

			/* Update current register values. */
			case MB_REG_WRITE:
			while( iNCoils > 0 )
			{
				xMBUtilSetBits( ucRegCoilsBuf, usBitOffset,
				( unsigned char )( iNCoils > 8 ? 8 : iNCoils ),
				*pucRegBuffer++ );
				iNCoils -= 8;
				usBitOffset += 8;
			}
			break;
		}

	}
	else
	{
		eStatus = MB_ENOREG;
	}

	return eStatus;
}

/*Обработчик DISCRETE*/
eMBErrorCode eMBRegDiscreteCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete)
{
	eMBErrorCode    eStatus = MB_ENOERR;
	int             iNDiscrete = ( int )usNDiscrete;
	unsigned short  usBitOffset;


	if( ( usAddress >= REG_DISCRETE_START ) &&
	( usAddress + usNDiscrete <= REG_DISCRETE_START + REG_DISCRETE_SIZE ) )
	{
		usBitOffset = ( unsigned short )( usAddress - REG_DISCRETE_START );
		while( iNDiscrete > 0 )
		{
			*pucRegBuffer++ =
			xMBUtilGetBits( ucRegDiscreteBuf, usBitOffset,
			( unsigned char )( iNDiscrete >
			8 ? 8 :
			iNDiscrete ) );
			iNDiscrete -= 8;
			usBitOffset += 8;
		}



	}
	else
	{
		eStatus = MB_ENOREG;
	}

	return eStatus;
}

unsigned char DS18B20_ReadTemperature(unsigned char bus, unsigned char * id, unsigned int* temperature)
{
	if (delay_timer_flag==0)
	{

		/*подаем сигнал сброса
		команду для адресации 1-Wire устройства на шине
		подаем команду - запук преобразования */
		OWI_DetectPresence(bus);
		OWI_MatchRom(id, bus);
		OWI_SendByte(DS18B20_CONVERT_T ,bus);

		delay_timer_flag=1;
		return READ_WAIT;
	}
	else
	{
		/*ждем, когда датчик завершит преобразование*/

		if (timer_delay_ds>=1500)
		{
			delay_timer_flag=0; //Выключаем инкремент таймера
			timer_delay_ds=0; // Выключаем таймер
			/*подаем сигнал сброса
			команду для адресации 1-Wire устройства на шине
			команду - чтение внутренней памяти
			затем считываем внутреннюю память датчика в массив
			*/
			OWI_DetectPresence(bus);
			OWI_MatchRom(id, bus);
			OWI_SendByte(DS18B20_READ_SCRATCHPAD, bus);
			for (char i = 0; i<=8; i++)
			{
				scratchpad[i] = OWI_ReceiveByte(bus);
			}

			if(OWI_CheckScratchPadCRC(scratchpad) != OWI_CRC_OK)
			{
				return READ_CRC_ERROR;
			}

			*temperature = (unsigned int)scratchpad[0];
			*temperature |= ((unsigned int)scratchpad[1] << 8);

			return READ_SUCCESSFUL;
		}
		else
		{
			return READ_WAIT;
		}
	}
}

void SensorFound()
{
	if (searchFlag == SEARCH_SENSORS)
	{
		num = 0;
		crcFlag = OWI_SearchDevices(allDevices, MAX_DEVICES, BUS, &num);
		BCD_1(num);

		if ((num == MAX_DEVICES)&&(crcFlag != SEARCH_CRC_ERROR))
		{
			searchFlag = SENSORS_FOUND;
		}

		usRegInputBuf[0]=num;

	}
}

void timerInit()
{
	TCNT0=0; //Сброс счетчика
	OCR0A=125; //Регистр сравнения
	TCCR0B|=(1<<CS00)|(1<<CS01); //Выбор предделителя 64, запуск таймера
	TIMSK0|=(1<<OCIE0A);         //Прерывание при совпадении
}

ISR(TIMER0_COMPA_vect)
{
	TCNT0=0; //Сброс счетчика

	if(delay_timer_flag)
	{
		timer_delay_ds++;
	}


	if (timer_termostat_step<1000)
	{
		timer_termostat_step++;

	}
	else{
		timer_termostat_step=0;
		timer_step++;
		timer_termostat_second_step++;
	}

}
char get_temperature()
{
	crcFlag = DS18B20_ReadTemperature(BUS, allDevices[index_of_read_ds].id, &temperature);
	if (crcFlag == READ_SUCCESSFUL)
	{
		unsigned int tmp = 0;
		int znak=0;
		/*выводим знак температуры
		*если она отрицательная
		*делаем преобразование*/
		if ((temperature & 0x8000) == 0)
		{
			znak=1;
		}
		else
		{
			znak=-1;
			temperature = ~temperature + 1;
		}
		//
		tmp = (unsigned int)(temperature>>4);
		/*  if (tmp<100)
		{
		BCD_2(tmp);
		}
		else
		{
		BCD_3Int(tmp);
		}*/

		temperature_first=tmp*100;

		//выводим дробную часть знач. температуры
		tmp = (unsigned int)(temperature&15);
		tmp = tmp*100;
		tmp = tmp/16;
		//BCD_2(tmp);


		temperature_first*=znak;
		temperature_first+=tmp;

		for (int i = 0; i < 5; i++ )//Сдвиг массива
		{
			usTempInputBuf[index_of_read_ds][i]=usTempInputBuf[index_of_read_ds][i+1];
		}

		usTempInputBuf[index_of_read_ds][5]=temperature_first; //запись в последний элемент текущей температуры

		unsigned int result_average_temp=0;

		for (int i = 0; i < 6; i++ ) // ссумирование массива
		{

			result_average_temp+=usTempInputBuf[index_of_read_ds][i];

		}

		usHoldingBuf[index_of_read_ds]=result_average_temp/6; //среднее массива
		//usHoldingBuf[index_of_read_ds]=temperature_first;

		index_of_read_ds++;
		return 1;
	}
	else if(crcFlag==READ_CRC_ERROR)
	{
		index_of_read_ds++;
		searchFlag = SEARCH_SENSORS;
		return READ_CRC_ERROR;
	}

}
void temer_poll()
{
	if (timer_step>REFRESH_TEMP_DELAY)
	{
		if(get_temperature()&&(index_of_read_ds>=num))
		{
			//Временно--------------------//
			lcd_clear();
			lcd_gotoxy(0,0);
			char buffer[5];
			lcd_putstring("V:");
			utoa(usHoldingBuf[0]/10,buffer,10);
			lcd_putstring(buffer);
			lcd_putstring(".");
			utoa(usHoldingBuf[0]%10,buffer,10);
			lcd_putstring(buffer);
			lcd_putstring("C ");
			lcd_putstring("O:");
			utoa(usHoldingBuf[1]/10,buffer,10);
			lcd_putstring(buffer);
			lcd_putstring(".");
			utoa(usHoldingBuf[1]%10,buffer,10);
			lcd_putstring(buffer);
			lcd_putstring("C ");
			lcd_gotoxy(0,1);
			lcd_putstring("D:");
			utoa(usHoldingBuf[2]/10,buffer,10);
			lcd_putstring(buffer);
			lcd_putstring(".");
			utoa(usHoldingBuf[2]%10,buffer,10);
			lcd_putstring(buffer);
			lcd_putstring("C ");
			lcd_putstring("P:");
			utoa(usHoldingBuf[3]/10,buffer,10);
			lcd_putstring(buffer);
			lcd_putstring(".");
			utoa(usHoldingBuf[3]%10,buffer,10);
			lcd_putstring(buffer);
			lcd_putstring("C ");
			//--------------------------//
			timer_step=0;
		}
		if(index_of_read_ds>=num) index_of_read_ds=0;
	}
}

unsigned char* lcd_menuText(int8_t menuShift)
{
	int8_t i;
	Menu_Item_t* tempMenu;

	if ((void*)Menu_GetCurrentMenu == (void*)&NULL_MENU) return strNULL;

	i = menuShift;
	tempMenu = Menu_GetCurrentMenu();
	if (i>0)
	{
		while( i!=0 )
		{
			if ((void*)tempMenu != (void*)&NULL_MENU)
			{
				tempMenu = (Menu_Item_t*)pgm_read_word(&tempMenu->Next);
			}
			i--;
		}
	}
	else
	{
		while( i!=0 )
		{
			if ((void*)tempMenu != (void*)&NULL_MENU)
			{
				tempMenu = (Menu_Item_t*)pgm_read_word(&tempMenu->Previous);
			}
			i++;
		}
	}

	if ((void*)tempMenu == (void*)&NULL_MENU)
	{
		return strNULL;
	}
	else
	{
		char* str=tempMenu->Text;

		//lcd_putstring(pgm_read_word(&str));
		//strcpy_P(buff, pgm_read_word(&(TempMenu->Text)));
		char c=0;
		char i=0;

		while((c = (const char*)pgm_read_byte(str++)))
		{
			lcd_putchar(c);
			i++;
		}



		return i;
	}
}

char * utoa_builtin_div(uint32_t value, char *buffer)
{
	buffer += 11;
	// 11 байт достаточно для десятичного представления 32-х байтного числа
	// и  завершающего нуля
	*--buffer = 0;
	do
	{
		*--buffer = value % 10 + '0';
		value /= 10;
	}
	while (value != 0);
	return buffer;
}

void Termostat()

{


	if(AUTO_REG_FLAG) //Если разрешено авторегулирование
	{
		usRegInputBuf[4] = timer_termostat_step;//debug

		if(timer_termostat_PWM<timer_termostat_second_step)TEN1_OFF; //По истечении таймера ШИМ вырубить тену №1

		if (timer_termostat_second_step>TERMOSTAT_PERIOD)
		{
			TEN1_OFF;
			TEN2_OFF;
			TEN3_OFF;
			int P=0;                //Переменная воздействия %
			timer_termostat_second_step=0;  //Сброс Таймера
			int nevyzka=0;
			nevyzka = TARGET_TEMP-CURRENT_TEMP;
			int delta_t=0;
			delta_t=t_prev-CURRENT_TEMP;
			t_prev=CURRENT_TEMP;

			i_imbalance=nevyzka+i_imbalance; //Суммирование интегральной части

			usRegInputBuf[5]=K_prop*nevyzka;
			usRegInputBuf[6]=i_imbalance/K_integ;
			usRegInputBuf[7]=K_dif*delta_t;
			usRegInputBuf[8]=nevyzka;

			P = (K_prop*nevyzka+K_dif*delta_t+i_imbalance/K_integ);    //Вычисление воздействия на данном шаге

			PWM_intens = P;

			if (P>=1000)
			{
				timer_termostat_PWM=TERMOSTAT_PERIOD;
				TEN1_ON;
				TEN2_ON;
				TEN3_ON;
			}

			if (P<0)
			{
				TEN1_OFF;
				TEN2_OFF;
				TEN3_OFF;
			}
			else{
				if (P<1000)                 //Если требуется регулирование
				{


					timer_termostat_PWM = (TERMOSTAT_PERIOD/10)*((P%333)*3)/100;

					usRegInputBuf[2] = timer_termostat_PWM; //Debug

					if (P>660)
					{
						TEN1_ON;
						TEN2_ON;
						TEN3_ON;
					}
					else
					{
						if (P>330)
						{
							TEN1_ON;
							TEN2_ON;
							TEN3_OFF;
						}

						else
						{
							TEN1_ON;
							TEN2_OFF;
							TEN3_OFF;
						}

					}
				}
			}

		}


	}
}
