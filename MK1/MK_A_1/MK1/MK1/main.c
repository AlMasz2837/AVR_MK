/*
 * MK1.c
 *
 * Created: 16.01.2023 22:09:26
 * Author : Алексей
 */ 
#include <avr/io.h>
#define  F_CPU 1000000UL
#include <util/delay.h>
#include <avr/interrupt.h>

uint8_t segments[]=
{
	// GFEDCBA
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

uint8_t segments1[]=
{
	// GFEDCBA
	0b01111111, // 0 - A, B, C, D, E, F
	0b00001110, // 1 - B, C
	0b10110111, // 2 - A, B, D, E, G
	0b10011111, // 3 - A, B, C, D, G

	0b11001110, // 4 - B, C, F, G
	0b11011101, // 5 - A, C, D, F, G
	0b11111101, // 6 - A, C, D, E, F, G
	0b00001111, // 7 - A, B, C
	0b11111111, // 8 - A, B, C, D, E, F, G
	0b11011111, // 9 - A, B, C, D, F, G
};

volatile int button = 0;
volatile int switch_state = 1;
volatile int counter = 0;
ISR(INT0_vect)
{ // Обработчик прерывания
	if(switch_state == 1)
	{
		switch_state = 0;	
	}
	 else
	{
		switch_state = 0;
		counter = 0;
	}
}

int main(void)
{
	DDRB = 0xFF;
	DDRD = 0xFF;
	PORTD |= (1<<PD2);
	EIMSK |= (1<<INT0); //Включаем INT0
	EICRA |= (1<<ISC01); //Прерывание по спадающему фронту INT0
	sei(); //Глобальное разрешение прерываний
	counter = 0;
	while(1)
	{
		if(switch_state == 0)
		{
			if(counter < 10)
			{
				PORTD = segments1[counter];
				counter += 1;
				_delay_ms(500);
			}
			else
			{
				counter = 0;
				PORTD = segments1[counter];
				counter += 1;
				_delay_ms(500);
			}
			for(int i = 0; i<10; i++)
			{
				i += 0;
				PORTB = segments[i];
				_delay_ms(500);
			}
				PORTB = segments[0];
				PORTD = segments1[counter];	
		}
	}
}

