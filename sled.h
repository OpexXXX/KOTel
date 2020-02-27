#ifndef __SLED_BUTTUN_LIB
#define __SLED_BUTTUN_LIB


#define SPI_DDR DDRB
#define SPI_PORT PORTB

#define MOSI_PIN (1<< PB3)         //MOS (SI)
#define CS_PIN (1<< PB2)        //SS   (RCK)
#define SCK_PIN (1<< PB5)        //SCK  (SCK)

#define NULL_BUTTON 195
#define START_DELAY_REP_BUTTON 60
#define MIN_DELAY_REP_BUTTON 4
#define ACCELERATION_REP_BUTTON  15
#define DELAY_BEFORE_REP 350


void set_data_reg();
void set_data_send(char data,unsigned char index);
void SPI_out_shift_reg(char data,char regs);
void set_data_dec(int dat);
unsigned int return_data_dec();
void set_data_bin(unsigned char dat );
void seven_led_init();
void sled_spi_out();
unsigned char get_data_send(unsigned char index);


//Опрос клавиатуры
	char get_key_button();
	void buttn_poll();
	void scan_button();
	char btn_press();
    void inc_button_counter();
	unsigned int get_delay_button_pep();
#endif