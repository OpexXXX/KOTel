#ifndef _LCD_H
#define _LCD_H
#include <avr/io.h>
#include <stdio.h>
#define F_CPU 16000000UL
#include <util/delay.h>

/*
 * RS = D 12 = PB4
 * E = D11 = PB3
 * D4 = D5 = PD5
 * D5 = D4 = PD4
 * D6 = D3 = PD3
 * D7 = D2 = PD2
 *
 */
// установка лог. "1" на линии E
#define LCD_E_SET    PORTB|=0x08
/* установка лог. "0" на линии E */
#define LCD_E_CLR    PORTB&=0xF7
/* установка лог. "1" на линии RS */
#define LCD_RS_SET   PORTB|=0x10
/* установка лог. "0" на линии RS */
#define LCD_RS_CLR   PORTB&=0xEF
/* макрос, указывающий функции, что
передаются команды */
#define LCD_COMMAND  0
/* макрос, указывающий функции, что
передаются данные */
#define LCD_DATA     1

#define lcd_putc(x)  lcd_putbyte(x, LCD_DATA)

void lcd_putnibble(char);
void lcd_putchar(char);
 void lcd_putbyte(char, char);
 void lcd_init();
 void lcd_clear();
 void lcd_gotoxy(char, char);
void lcd_putstring (char*);

#endif
