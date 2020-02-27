#ifndef __LCD_PC8812
#define __LCD_PC8812



#define LCD_CE_PORT PORTD
#define LCD_CE_PIN  (1<<PD5)
#define LCD_CE_DDR  DDRD

#define LCD_DC_PORT PORTD
#define LCD_DC_PIN  (1<<PD4)
#define LCD_DC_DDR  DDRD

#define LCD_RST_PORT PORTD
#define LCD_RST_PIN  (1<<PD2)
#define LCD_RST_DDR  DDRD

#define LCD_VDD_PORT PORTC
#define LCD_VDD_PIN  (1<<PC5)
#define LCD_VDD_DDR  DDRD

#define LCD_SPI_DDR DDRB
#define LCD_SPI_PORT PORTB

#define LCD_MOSI_PIN (1<< PB3)         //MOS (SI)
#define LCD_CS_PIN (1<< PB2)        //SS   (RCK)
#define LCD_SCK_PIN (1<< PB5)        //SCK  (SCK)


void LCD_setup_io();
void LCDInitC55 ();
void LCD_SPI_out (unsigned char data , char cd);
void LCD_on();
void LCD_reset();
void LCD_clear_RAM();
unsigned char LCD_get_symbol_from_array(unsigned int index_array);
unsigned char LCD_get_symbol_from_array_tahoma(unsigned int index_array);
unsigned char LCD_get_symbol_from_array_tahoma8(unsigned int index_array);

void LCD_print_symbol(unsigned char ch);
void LCD_print_array_link(char *str);

void LCD_print_tahoma_bold(unsigned char ch,unsigned char ramka);

void LCD_set_rows_columns(unsigned char rows, unsigned char columns);
void LCD_set_XY(unsigned char x, unsigned char y);
void LCD_print_array_tahoma_link(char *str,unsigned char ramka);
void LCD_draw_menu();
void LCD_set_marker_pos(int pos);
void LCD_increment_marker_pos();
void LCD_deskrement_marker_pos();
int LCD_get_marker_pos();
#endif