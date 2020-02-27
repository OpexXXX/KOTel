#include "lcd.h"
#include <avr/io.h>
#include <stdio.h>
#define F_CPU 16000000UL
#include <util/delay.h>

void lcd_putnibble(char t)
{
    t<<=4;
    PORTD&=0x0F;
    LCD_E_SET;
    _delay_us(1);

    PORTD|=t;

    LCD_E_CLR;
    _delay_us(100);

}

/* функция передачи тетрады в жки */


/* функция передачи байта в жки.
char c - сам байт
char rs - переменная, указывающая что передается:
     rs = 0 - команда (устанавливается линия RS)
	 rs = 1 - данные (сбрасывается линия RS) */
void lcd_putbyte(char c, char rs)
{
    char highc=0;
    highc=c>>4;
	if (rs==LCD_COMMAND) LCD_RS_CLR;
	else                 LCD_RS_SET;
    lcd_putnibble(highc);
    lcd_putnibble(c);
}

void lcd_putchar(char c)
{
    char highc=0;
    highc=c>>4;
	LCD_RS_SET;
    lcd_putnibble(highc);
    lcd_putnibble(c);
}

/* функция инициализации работы жки
в 4-битном режиме, без курсора */
void lcd_init()
{
	lcd_putbyte(0x01, LCD_COMMAND);
    _delay_ms(15);
	lcd_putnibble(0x03);

       _delay_us(4500);
       lcd_putnibble(0x03);
		_delay_us(4500);
       lcd_putnibble(0x03);
       _delay_us(150);
       lcd_putnibble(0x02);
		_delay_ms(5);
		lcd_putbyte(0x01, LCD_COMMAND);
		_delay_us(1500);

      lcd_putbyte(0x28, LCD_COMMAND);
      _delay_ms(5);
        lcd_putbyte(0x0C, LCD_COMMAND);
      _delay_ms(5);
      lcd_putbyte(0x01, LCD_COMMAND);
      _delay_ms(20);
      lcd_putbyte(0x01, LCD_COMMAND);
            _delay_ms(2);


}


/* функция очистки дисплея и возврата
курсора в начальную позицию*/
void lcd_clear()
{
    lcd_putbyte(0x01, LCD_COMMAND);
    _delay_us(1500);
}

/* функция перемещения курсора в заданную позицию
col - номер знакоместа по горизонтальной оси (от 0 до 15)
row - номер строки (0 или 1) */
void lcd_gotoxy(char col, char row)
{
  char adr;
  adr=0x40*row+col;
  adr|=0x80;
  lcd_putbyte(adr, LCD_COMMAND);
}

void lcd_putstring (char* stroka)
{
int i=0;
   for(i=0;stroka[i]!='\0';i++)
   lcd_putchar(stroka[i]);


}


