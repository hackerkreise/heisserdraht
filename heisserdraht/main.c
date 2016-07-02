/*
* heisserdraht.c
*
* Created: 15.06.2016 20:56:18
* Author : Max
*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#define F_CPU 8000000

uint8_t count = 0;
uint8_t wasted = 0;		//Zähler für den Timer
uint8_t timerem = 0;	//Zähler der an LED_out übergeben wird
uint8_t trys =3;		//Zähler für den Fehler Interrupt
uint8_t test= 0;		//Testen von INT0
inline void init_ports()
{
	DDRA |= 0b11111011;
	DDRB |= _BV(PB3) & ~_BV(PB6);
	PORTB6 == 1<<PORTB6;
	//PORTB4 == 1<<PORTB4;
	//Definition von INT0 und INT1
	GIMSK = 11000000;					// Enable INT0 und INT1
	MCUCR = 1<<ISC01 | 1<<ISC00;	// Trigger INT0 on rising edge
	MCUCR = 1<<ISC01 | 1>>ISC00;	//Trigger INT1 on falling Edge
	
	sei();				//Enable Global Interrupt

}

void LED_out(uint8_t out)//Hier werden die LEDs aufgesetzt, ein Integer im TimerOverflow soll hochgezählt werden nach jeweils 4s wird eine weiter LED angeschaltet...
{
	PORTB &= ~_BV(PB3);
	PORTA &= 0b00000111; //wenn wasted=31 sind alle LEDs an. 31 Sekunden Zeit fürs Spiel.
	PORTA |= out<<3;
}



int main ()
{
	init_ports();
	//Enable TIMER1
	TCCR0A |= _BV(TCW0);							//Timer auf 16Bit setzen avr Manual S.83
	TCNT0H = (0x10000 - (8000000 / 256)) / 256;		//Zähler vorladen
	TCNT0L = (0x10000 - (8000000 / 256)) % 256;
	
	TCCR0B |= _BV(CS02);							//Setzen des Prescalers auf 256 (S.84)

	TIMSK |= _BV(TOIE0);							//Overflowbit setzen
		
		

	while(1)
	{	
		if (wasted % 4 == 0) //Dafür sorgen das nur Werte durch 4 Teilbar sind an LED_out() übergeben werden
		{
			timerem = wasted;
		}
		
		for (uint8_t i = 0; i<=100; i++)
		{
			
			for (uint8_t j = 0; j<100; j++)
			{
				if (wasted >= 32) //Zeit die für das Spiel vorhanden ist festlegen.
				{
					wasted = 31;
					trys = 1; //Nach Ablauf der Zeit Wechsel in den Case loose(=10) erzwingen
				}
				if (trys >9)
				{
					trys = 1; //Nachdem zehnten Versuch soll das Board in den loose state gehen.
				}
				_delay_ms(20);
				LED_out(wasted);

				_delay_ms(20);
				seg_out(trys);
				
			}  //for j
		}// for i
	} // while

}

void seg_out(uint8_t out)//Ansteuerung des 7 Segment Displays
{
	PORTB |= _BV(PB3);
	PORTA &= 0b00000100;

	switch(out)
	{
		case   3:	PORTA |= 0b00000001; break;		//=0
		case   4:	PORTA |= 0b10011111; break;		//=1
		case   5:	PORTA |= 0b00100010; break;		//=2
		case   6:	PORTA |= 0b00001110; break;		//=3
		case   7:	PORTA |= 0b10011000; break;		//=4
		case   8:	PORTA |= 0b01001000; break;		//=5
		case   9:	PORTA |= 0b01000000; break;		//=6
		case   10:	PORTA |= 0b00011101; break;		//=7
		case   11:	PORTA |= 0b00000000; break;		//=8
		case   12:	PORTA |= 0b00001000; break;		//=9
		case   1:	PORTA |= 0b01101010; break; // loose
		case   0:	PORTA |= 0b10010001; break; //pre start
		case   2:	PORTA |= 0b11000011; break; //win
		default:;
		//An dieser Stelle hat mir das hier sehr geholfen: http://www.mikrocontroller.net/articles/AVR-Tutorial:_7-Segment-Anzeige
	}

}


ISR(INT1_vect){// Interrupt für Berührung der Drähte.
	_delay_ms(100);
	trys++;
	
}


ISR(TIMER0_OVF_vect){// Setzt den Timer zurück nach jeder Sekunde und zählt die vergangene Zeit in wasted
	TCNT0H = (0x10000 - (8000000 / 256)+1) / 256;		//Zähler vorladen
	TCNT0L = (0x10000 - (8000000 / 256)+1) % 256;
	wasted++; 
}


ISR(INT0_vect){ //Interrupt für den BUtton
	_delay_ms(100);
	seg_out(0); //Ausgabe des Startsignals
	_delay_ms(10000); //Zeit zum reset geben
	trys = 3;
	wasted = 0;
}