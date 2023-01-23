/*
 * MK1_2.c
 *
 * Created: 17.01.2023 9:51:09
 * Author : Алексей
 */ 


#define F_CPU 1000000UL // директива определяет тактовую частоту работы
// контроллера и необходима для работы функции задержки
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h> // библиотека, в котором содержатся названия векторов
// обработчиков прерывания
uint8_t segments[]=
{ //0b01111111
	// ___GFEDCBA
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
{ //0b01111111
	// ___GFEDCBA
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
volatile int i = 0;
ISR(INT0_vect)
{ // Обработчик прерывания
	if(switch_state == 1)
	{
		switch_state = 0;
		}
		else
		{
		switch_state = 0;
		counter += 1;
		if (i==1 && counter == 6)
		{
			i = 0;
			counter = 0;
		}
		PORTD = segments1[i];
	}
}
int main(void)
{
	DDRD = 0xFF; // регистр режима работы порта D, шестнадцатеричный код
	DDRB = 0xFF; // регистр режима работы порта B, шестнадцатеричный код
	PORTD |= (1<<PD2); // регистр, который зависит от того, что записано в DDR
	// свиг числа 1 на 2 позиции влево
	EIMSK |= (1<<INT0); //Включаем INT0
	EICRA |= (1<<ISC01); //Прерывание по спадающему фронту INT0
	sei(); //Глобальное разрешение прерываний

	while(1) //  бесконечный цикл
	{
		if(switch_state == 0)
		{
			if(counter < 10)
			{
				PORTB = segments[counter];
				_delay_ms(500); // задержка на 500 мс
			}
			else 
			{
			counter = 0;
			i += 1;
			PORTB = segments[counter];
			counter = i;
			PORTD = segments1[counter];
			counter = 0;
			_delay_ms(500); // задержка на 500 мс
			}
		}
	}
}