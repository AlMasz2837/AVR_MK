#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
uint8_t segments[]=
{
	0b00111111, // 0 - A, B, C, D, E, F
	0b00000110, // 1 - B, C
	0b01011011, // 2 - A, B, D, E, G
	0b01001111, // 3 - A, B, C, D, G
	0b01100110, // 4 - B, C, F, G
	0b01101101, // 5 - A, C, D, F, G
	0b01111101, // 6 - A, C, D, E, F, G
	0b00000111, // 7 - A, B, C
	0b01111111, // 8 - A, B, C, D, E, F, G
	0b01101111, // 9 - A, B, C, D, F, G
};
void InitPorts(void);
void send_data(uint8_t data, uint8_t ind);
void InitTimer0(void);
void Bin2Dec(uint16_t data);
void InitTimer1(void);
void StartTimer1(void);
void StopTimer1(void);
void SendData(uint8_t data);
void DisplayData(uint16_t data);
void InitSPI(void);
volatile uint16_t cnt = 0;
volatile uint8_t switch_state = 0;
volatile uint8_t bcd_buffer[] = {0,0};
int main(void)
{
	InitPorts();
	InitSPI();
	InitTimer1();
	EIMSK |= (1<<INT0); //разрешить прерывание INT0
	EICRA |= (1<<ISC01);//Запуск по заднему фронту INT0
	sei(); //Разрешение прерываний
	PORTB &= ~(1<<PB0); //OE = low (active)
	DisplayData(0);
	while(1)
	{
		
	}
}
//--------------------------------------------
ISR(TIMER1_COMPA_vect)
{
	DisplayData(cnt);
	if(cnt<99)
	{
		cnt++;
	}
	else
	{
		cnt=0;
	}
}
ISR(INT0_vect)
{
	if(switch_state == 0)
	{
		switch_state = 1;
		StartTimer1();
	}
	else
	{
		StopTimer1();
		DisplayData(cnt);
		switch_state = 0;
		cnt = 0;
	}
}
//--------------------------------------------
void InitPorts(void)
{
	DDRB = (1<<PB0|1<<PB1|1<<PB3|1<<PB5);
	DDRD &= ~(1<<PD2);
	PORTD |= (1<<PD2);
}
void InitTimer1(void)
{
	TCCR1A = 0;
	TCCR1B = (1<<CS11|1<<CS10|1<<WGM12);
	TCNT1 = 0;
	OCR1A = 15624;
}
void StartTimer1(void)
{
	TCNT1 = 0;
	TIMSK1 |= (1<<OCIE1A);
}
void StopTimer1(void)
{
	TIMSK1 &= ~(1<<OCIE1A);
}
void Bin2Dec(uint16_t data)
{
	// 	bcd_buffer[3] = (uint8_t)(data/1000);
	// 	data = data % 1000;
	// 	bcd_buffer[2] = (uint8_t)(data/100);
	// 	data = data % 100;
	bcd_buffer[1] = (uint8_t)(data/10);
	data = data % 10;
	bcd_buffer[0] = (uint8_t)(data);
}
void SendData(uint8_t data)
{
// 	for(uint8_t i=0; i<8; i++)
// 	{
// 		PORTB |= (1<<PB5); //CLK low
// 		if(0x80 & (data<<i))
// 		{
// 			PORTB |= 1<<PB3; //DAT high
// 		}
// 		else
// 		{
// 			PORTB &= ~(1<<PB3); //DAT low
// 		}
// 		PORTB &= ~(1<<PB5); //CLK high
// 	}
	SPDR = data;
	while(!(SPSR & (1<<SPIF)));
}
void DisplayData(uint16_t data)
{
	Bin2Dec(data);
	PORTB &= ~(1<<PB1); //clk_out = 0
	SendData(segments[bcd_buffer[0]]);
	SendData(segments[bcd_buffer[1]]);
	// 	SendData(segments[bcd_buffer[2]]);
	// 	SendData(segments[bcd_buffer[3]]);
	PORTB |= (1<<PB1); //clk_out = 1
}
void InitSPI(void)
{
	DDRB |= (1<<PB3 | 1<<PB5);//настроить MOSI и CLK как выходы
	SPSR |= (1<<SPI2X); //Fclk = Fosc/2
	SPCR = (1<<SPE | 1<<MSTR); //SPI включен,мастер,
	//MSB первый, CPOL=0, CPHA=0
	PORTB &= ~(1<<PB3 | 1<<PB5);
	//инициализация: DAT=0, CLK=0
}
