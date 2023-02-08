#define F_CPU 1000000UL

#define D4 PB0
#define D5 PB1
#define D6 PB2
#define D7 PB3
#define RS PB5
#define RW PB6
#define E PB7
#define CMD 0
#define DATA 1

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
uint8_t segments[] =
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
void InitPorts(void);
void InitTimer1(void);
void Bin2Dec(uint16_t data);
void SendData(uint8_t data);
void DisplayData(uint16_t data);
void InitSPI(void);
void InitADC(void);
void InitUSART(void);
void InitLCD(void);
void LCD_Write(uint8_t type,char data);
char LCD_Read(void);
void SendChar(char symbol);
void SendString(char * buffer);
volatile uint8_t bcd_buffer[] = {0,0,0,0};
volatile uint16_t display_val = 0;
int main(void)
{
	InitPorts();
	//InitSPI();
	InitLCD();
	InitTimer1();
	EIMSK |= (1<<INT0); //Enable INT0
	EICRA |= (1<<ISC01); //Trigger on falling edge of INT0
	InitADC();
	InitUSART();
	sei(); //global interrupt enable
	PORTB &= ~(1<<PB0); //OE = low (active)
	//DisplayData(0);
	SendString("Hello\r\n");
	LCD_Write(DATA,'H');
	LCD_Write(DATA,'e');
	LCD_Write(DATA,'l');
	LCD_Write(DATA,'l');
	LCD_Write(DATA,'o');
	LCD_Write(CMD,0x40|0x80); // переход на вторую строку
	LCD_Write(DATA,'V');
	LCD_Write(DATA,'a');
	LCD_Write(DATA,'l');
	LCD_Write(DATA,'u');
	LCD_Write(DATA,'e');
	LCD_Write(DATA,'=');
	LCD_Write(DATA,0x20); // вывод пробела
	while(1)
	{
		//DisplayData(display_val);
		Bin2Dec(display_val);
		LCD_Write(CMD,0x47|0x80); //установка курсора на 7-е
		//знакоместо 2-й строки
		LCD_Write(DATA,0x30+bcd_buffer[3]);
		LCD_Write(DATA,0x30+bcd_buffer[2]);
		LCD_Write(DATA,0x30+bcd_buffer[1]);
		LCD_Write(DATA,0x30+bcd_buffer[0]);
	}
}
//--------------------------------------------
ISR(TIMER1_COMPB_vect)
{
	
}
ISR(INT0_vect)
{
	SendString("Value = ");
	SendChar(0x30 + bcd_buffer[3]);
	SendChar(0x30 + bcd_buffer[2]);
	SendChar(0x30 + bcd_buffer[1]);
	SendChar(0x30 + bcd_buffer[0]);
	SendString("\r\n");
}
ISR(ADC_vect)
{
	display_val = ADC;
}
ISR(USART_RX_vect)
{
	if(UDR0 == 0x20)
	{
		SendString("Roger that\r\n");
	}
}
//--------------------------------------------
void InitPorts(void)
{
	DDRB=0xFF; //настройка выводов управления на выход
	PORTB=0;
	DDRD = (0<<PD2);
	PORTD |= (1<<PD2);
}
void InitTimer1( void)
{
	TCCR1A = 0; //CTC mode - Clear Timer on Compare
	//prescaler = sys_clk/64
	TCCR1B = (1<<CS11 | 1<<CS10 | 1<<WGM12);
	TCNT1 = 0; //start value of counter
	TIMSK1 |= (1<<OCIE1B);
	OCR1A = 1562;
	OCR1B = 1562;
}
void Bin2Dec(uint16_t data)
{
	bcd_buffer[3] = (uint8_t)(data/1000);
	data = data % 1000;
	bcd_buffer[2] = (uint8_t)(data/100);
	data = data % 100;
	bcd_buffer[1] = (uint8_t)(data/10);
	data = data % 10;
	bcd_buffer[0] = (uint8_t)(data);
}
// void SendData (uint8_t data)
// {
// 	SPDR = data;
// 	while(!(SPSR & (1<<SPIF)));
// }
// void DisplayData (uint16_t data)
// {
// 	Bin2Dec(data);
// 	PORTB &= ~(1<<PB1); //clk_out = 0
// 	SendData(segments[bcd_buffer[0]]);
// 	SendData(segments[bcd_buffer[1]]);
// 	SendData(segments[bcd_buffer[2]]);
// 	SendData(segments[bcd_buffer[3]]);
// 	PORTB |= (1<<PB1); //clk_out = 1
// }
// void InitSPI( void)
// {
// 	DDRB |= (1<<PB3 | 1<<PB5);//configure MOSI and CLK as out
// 	SPSR |= (1<<SPI2X); //Fclk = Fosc/2
// 	//SPI enable, master mode, MSB first, CPOL=0, CPHA=0
// 	SPCR = (1<<SPE | 1<<MSTR);
// 	//init values - DAT low, CLK low
// 	PORTB &= ~(1<<PB3 | 1<<PB5);
// }
void InitADC( void)
{
	ADMUX = (1<<MUX0); //Align left, ADC1
	ADCSRB = (1<<ADTS2 | 1<<ADTS0); //Start on Timer1 COMPB
	//Enable, auto update, IRQ enable
	ADCSRA = (1<<ADEN | 1<<ADATE | 1<<ADIE);
}
void InitUSART()
{
	UCSR0B = (1<<RXEN0 | 1<<TXEN0 | 1<<RXCIE0);
	UCSR0C = (1<<UCSZ01 | 1<<UCSZ00);
	UBRR0H = 0;
	UBRR0L = 0x0C;
}
void SendChar(char symbol)
{
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = symbol;
}
void SendString(char * buffer)
{
	while(*buffer != 0)
	{
		SendChar(*buffer++);
	}
}
void InitLCD(void)
{
	uint8_t BF = 0x80; //объявление переменной-флага BF
	_delay_ms(40); //ожидание согласно алгоритму
	PORTB &= ~(1<<RS);
	PORTB = (0x30>>4); //выставление на шину данных
	//старшей тетрады команды 0х30
	PORTB |= (1<<E); // выставление сигнала Е
	asm("nop"); // ожидание в течение 3-х тактов
	asm("nop"); // на частоте 8 МГц 1 такт=125нс
	asm("nop"); //требуется удержания Е не менее 250нс
	PORTB &= ~(1<<E); // сброс Е
	PORTB = 0;
	_delay_ms(5); // ожидание согласно алгоритму
	PORTB &= ~(1<<RS);
	PORTB = (0x30>>4); // вторая отправка команды 0х30
	PORTB |= (1<<E);
	asm("nop");
	asm("nop");
	asm("nop");
	PORTB &= ~(1<<E);
	PORTB = 0;
	_delay_us(150); // ожидание согласно алгоритму
	PORTB &= ~(1<<RS);
	PORTB = (0x30>>4); // третья отправка команды 0х30
	PORTB |= (1<<E);
	asm("nop");
	asm("nop");
	asm("nop");
	PORTB &= ~(1<<E);
	PORTB = 0;
	_delay_ms(5);
	do
	{ // ожидание до тех пор, пока флаг BF
		// не освободится
		BF = (0x80 & LCD_Read());
	}
	while(BF == 0x80);
	PORTB &= ~(1<<RS);
	PORTB = (0x20 >> 4); //установка ширины шины данных
	//в 4 бит
	PORTB |= (1<<E);
	asm("nop");
	asm("nop");
	asm("nop");
	PORTB &= ~(1<<E);
	PORTB = 0;
	do
	{
		BF = (0x80 & LCD_Read());
	}
	while(BF == 0x80);
	// здесь обмен начинает работать по 4- проводной шине
	LCD_Write(CMD,0x28); // установка режима 2 строки,
	// знакоместо 5*8
	LCD_Write(CMD,0x0C); // включение отображения
	LCD_Write(CMD,0x06); // автоинкремент счетчика
}void LCD_Write(uint8_t type,char data)
{
	uint8_t BF = 0x80;
	do
	{ // ожидание завершения предыдущей операции
		BF = 0x80 & LCD_Read();
	}
	while(BF == 0x80);
	PORTB |= (type<<RS); // установка RS в зависимости от
	// типа данных
	PORTB |= (1<<E);
	PORTB &= ~(0x0F);
	PORTB |= (0x0F & (data>>4)); // передача старшей тетрады
	PORTB &= ~(1<<E);
	asm("nop");
	asm("nop");
	asm("nop");
	PORTB |= (1<<E);
	PORTB &= ~(0x0F);
	PORTB |= (0x0F & data); // передача младшей тетрады
	PORTB &= ~(1<<E);
	PORTB = 0;
}char LCD_Read(void)
{
	char retval = 0;
	PORTB &= ~(1<<RS);
	PORTB |= (1<<RW); // переход в режим чтения
	DDRB &= ~(1<<D4|1<<D5|1<<D6|1<<D7); //настройка выводов
	// на вход
	PORTB |= (1<<E);
	asm("nop");
	asm("nop");
	retval = (PINB & 0x0F)<<4; //чтение старшей тетрады байта
	PORTB&=~(1<<E);
	asm("nop");
	asm("nop");
	asm("nop");
	PORTB |= (1<<E);
	asm("nop");
	asm("nop");
	retval |= (PINB & 0x0F); // чтение младшей тетрады
	PORTB&=~(1<<E);
	DDRB |= (1<<D4|1<<D5|1<<D6|1<<D7); //настройка выводов
	// на выход
	PORTB = 0;
	return retval; // возвращаемое значение – прочитанный байт
}