/*
* heisserdraht.c
*
* Created: 12.06.2016 15:32:19
* Author : Adam Benesh
*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#define F_CPU 8000000




uint8_t count = 0;
uint8_t countdown = 0;		//Zähler für den Countdown
uint8_t LEDTimer = 0;	//Zähler der an LED_out übergeben wird
volatile uint8_t versuch =3;		//Zähler für den Fehler Interrupt
uint8_t test= 0;		//Testen von INT0
volatile static int int1flag = 0;

inline void init_ports()
{
	DDRA |= 0b11111011;
	DDRB |= _BV(PB3) & ~_BV(PB6); //& _BV(PB4); // In DDRB muss DDB4 logisch EINS gesetzt sein, damit Output möglich ist.
	PORTB6 == 1<<PORTB6;
	PORTB4 == 1<<PORTB4;				//Lautsprecherport aktivieren
	//Definition von INT0 und INT1
	GIMSK = 11000000;					// Enable INT0 und INT1
	//MCUCR = 1<<ISC01 | 1<<ISC00;	// Trigger INT0 on rising edge
	//MCUCR = 1<<ISC01 | 1>>ISC00;	//Trigger INT1 on falling Edge
	MCUCR = _BV(ISC00|ISC01);
	sei();				//Enable Global Interrupt

}

void LED_out(uint8_t out)//LED Anzeige
{
	PORTB &= ~_BV(PB3);
	PORTA &= 0b00000111; //wenn countdown=31 sind alle LEDs an. 31 Sekunden Zeit fürs Spiel.
	PORTA |= out<<3;
}

//#define BUZZER_DELAY 80    // Delay for each tick, quasi Frequenz einstellen.

void buzzer (int BUZZER_DELAY)
{
	int count = 100;
	while(count !=0)
	{
		for(int i=0; i <= BUZZER_DELAY; i++)
		{
		
	PORTB |= (1 << PORTB4);
	_delay_us(100);
	PORTB &= ~(1 << PORTB4);
	_delay_us(100);
		}
		
	_delay_ms(10);
	--count;
	
	}
	
}


void seg_out(uint8_t out)
{
	PORTB |= _BV(PB3);
	PORTA &= 0b00000100;

	switch(out)
	{
		case   3:	PORTA |= 0b00000001; break;		//=0
		case   4:	PORTA |= 0b10011111; break;		//=1
		case   5:	PORTA |= 0b00100010; break;		//=2
		case   6:	PORTA |= 0b00001010; break;		//=3
		case   7:	PORTA |= 0b10011100; break;		//=4
		case   8:	PORTA |= 0b01001000; break;		//=5
		case   9:	PORTA |= 0b01000000; break;		//=6
		case   10:	PORTA |= 0b00011101; break;		//=7
		case   11:	PORTA |= 0b00000000; break;		//=8
		case   12:	PORTA |= 0b00001000; break;		//=9
		case   1:	PORTA |= 0b01101010; break; 	// verloren
		case   0:	PORTA |= 0b10010001; break; 	//pre start
		case   2:	PORTA |= 0b11000011; break; 	//gewonnen
		default:;
		
	}

}


ISR(INT1_vect){// Interrupt bei Drahtkontakt
	
	/*
	if ( porthistory >= 1){
	buzzer(80);
	_delay_ms(100);
	versuch = versuch +1;
	//GIMSK = 11000000;	
	porthistory=0;
	}
	versuch--;
	porthistory++;
*/
	
	MCUCR = _BV(~ISC00|~ISC01); //Entprellen fuer Arme, Interrupt 0 Sense Control Seite 51 ATtiny261A Anleitung
	_delay_ms(200);
	int1flag = 1;
	_delay_ms(200);
	
	
	if (int1flag = 1){
		
		buzzer(80);
		versuch++;
		int1flag = 0;
		
		
		
	}
	
	MCUCR = _BV(ISC00|ISC01);
	
	
}





ISR(TIMER0_OVF_vect){// Setzt den Timer zurück nach jeder Sekunde und zählt die vergangene Zeit in countdown
	
	TCNT0H = (0x10000 - (8000000 / 256)+1) / 256;		//Zähler vorladen
	TCNT0L = (0x10000 - (8000000 / 256)+1) % 256;

	
	countdown++; 
}





ISR(INT0_vect){ //Interrupt für den BUtton
	MCUCR = _BV(~ISC00|~ISC01); //Entprellen fuer Arme, Interrupt 0 Sense Control Seite 51 ATtiny261A Anleitung
	buzzer(150);
	buzzer(200);
	buzzer(50);
	
	_delay_ms(100);
	seg_out(0); //Ausgabe des Startsignals
	_delay_ms(1500); //Zeit zum reset geben
	versuch = 3;
	countdown = 0;
	MCUCR = _BV(ISC00|ISC01);
}


int main ()
{
	init_ports();
	//Enable TIMER1
	TCCR0A |= _BV(TCW0);							//Timer auf 16Bit setzen
	TCNT0H = (0x10000 - (8000000 / 256)) / 256;		//Zähler vorladen
	TCNT0L = (0x10000 - (8000000 / 256)) % 256;
	
	TCCR0B |= _BV(CS02);							//Setzen des Prescalers auf 256

	TIMSK |= _BV(TOIE0);							//Overflowbit setzen
	
	
	
	
	
	
	

	while(1)
	{
		/*if (countdown % 4 == 0) //Dafür sorgen das nur Werte durch 4 Teilbar sind an LED_out() übergeben werden
		{
			LEDTimer = countdown;
		}*/
		
		
		
		
		
		
		for (uint8_t i = 0; i<=100; i++)
		{
			
			for (uint8_t j = 0; j<100; j++)
			{
				if (countdown >= 32) //Zeit die für das Spiel vorhanden ist festlegen.
				{
					countdown = 31;
					versuch = 1; //Nach Ablauf der Zeit Wechsel in den Case loose(=1) erzwingen
				}
				if (versuch >12)
				{
					versuch = 1; //Nachdem  zehnten Versuch soll das Board in den loose state gehen.
				}
				_delay_ms(20);
				LED_out(countdown);

				_delay_ms(20);
				seg_out(versuch);
				
			}  
		}
	} 
	
	
	
	

}