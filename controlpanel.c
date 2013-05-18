#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usb_keyboard.h"

#define LED_CONFIG	(DDRD |= (1<<6))
#define LED_OFF		(PORTD &= ~(1<<6))
#define LED_ON		(PORTD |= (1<<6))
#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))
#define PIN_C6 0b0100000
#define PIN_C7 0b1000000

uint8_t b_keys[8]={KEY_UP,KEY_DOWN,KEY_LEFT,KEY_RIGHT,KEY_Q,KEY_S,KEY_A,KEY_K,KEY_0};
uint8_t d_keys[8]={KEY_R,KEY_F,KEY_D,KEY_G,KEY_I,0,0,KEY_W,KEY_0};
uint8_t f_keys[8]={KEY_P,KEY_O,KEY_0,KEY_0,KEY_SPACE,KEY_L,KEY_Z,KEY_X,KEY_0};

uint8_t alt_b_keys[8]={KEY_TAB,KEY_ESC,KEY_LEFT,KEY_RIGHT,KEY_Q,KEY_S,KEY_A,KEY_K,KEY_0};
uint8_t alt_d_keys[8]={KEY_R,KEY_F,KEY_D,KEY_G,KEY_I,0,0,KEY_W,KEY_0};
uint8_t alt_f_keys[8]={KEY_P,KEY_O,KEY_0,KEY_0,KEY_SPACE,KEY_L,KEY_Z,KEY_X,KEY_0};


void key_action(uint8_t key,int push){
	int j;
	if(push){
		for(j=0;j<32;j++){
			if(keyboard_keys[j]==0){
				keyboard_keys[j]=key;
				break;
			}		
		}
	}
	else{
		for(j=0;j<32;j++){
			if(keyboard_keys[j]==key){
				keyboard_keys[j]=0;
			}		
		}
	}
	usb_keyboard_send();
}

int main(void)
{
	uint8_t b, c, d, f, mask, i;
	uint8_t b_prev=0xFF,c_prev=0xFF,d_prev=0xFF,f_prev=0xFF;
	
	int b_pressed = 0;
	int c_pressed = 0;
	int d_pressed = 0;
	int f_pressed = 0;

	int use_alt = 0;

	// set for 16 MHz clock
	CPU_PRESCALE(0);

	// Configure all input port pins as inputs with pullup resistors.
	DDRB = 0x00;
	PORTB = 0xFF;

	DDRC = 0x00;
	PORTC=0xFF;

	DDRD = 0x00;
	PORTD = 0xFF;

	DDRF = 0x00;
	PORTF = 0xFF;

	LED_CONFIG;
	LED_OFF;

	// Initialize the USB, and then wait for the host to set configuration.
	// If the Teensy is powered without a PC connected to the USB port,
	// this will wait forever.
	usb_init();
	while (!usb_configured()) /* wait */ ;

	// Wait an extra second for the PC's operating system to load drivers
	// and do whatever it does to actually be ready for input
	_delay_ms(1000); 
	while (1) {

		// read all input pins
		b = PINB;
		c = PINC;
		d = PIND;
		f = PINF;
	
		if(use_alt){
			LED_ON;
		}
		else{
			LED_OFF;
		}
		// check if any pins are low, but were high previously 
		mask = 1;
		for (i=0; i<8; i++) {
			//Port B key presses
			if ((b & mask) == 0 && (b_prev & mask) != 0) {
				if(use_alt){
					key_action(alt_b_keys[i], 1);
				}
				else{
					key_action(b_keys[i], 1);
				}
				LED_ON;
			}
			if ((b_prev & mask) == 0 && (b & mask) != 0) {
				if(use_alt){
					key_action(alt_b_keys[i], 0);
				}
				else{
					key_action(b_keys[i], 0);
				}
				LED_ON;
			}

			//PortD key presses
			if ((d & mask) == 0 && (d_prev & mask) != 0 &&  (i!=6)) {
				if(use_alt){
					key_action(alt_d_keys[i], 1);
				}
				else{
					key_action(d_keys[i], 1);
				}
				LED_ON;
			}
			if ((d_prev & mask) == 0 && (d & mask) != 0) {
				if(use_alt){
					key_action(alt_d_keys[i], 0);
				}
				else{
					key_action(d_keys[i], 0);
				}
				LED_ON;
			}

			//PortF key presses
			if ((f & mask) == 0 && (f_prev & mask) != 0 && (i!=2) && (i!=3) && (i!=8)) {
				if(use_alt){
					key_action(alt_f_keys[i], 1);
				}
				else{
					key_action(f_keys[i], 1);
				}
				LED_ON;
			}
			if ((f_prev & mask) == 0 && (f & mask) != 0) {
				if(use_alt){
					key_action(alt_f_keys[i], 0);
				}
				else{
					key_action(f_keys[i], 0);
				}
				LED_ON;
			}

			mask = mask << 1;
		}

		//Start buttons are a special case, both at once changes key modes
		if((((c & 0b01000000) == 0)!=0) && (((c & 0b10000000) == 0)!=0) && ((c_prev & 0b01000000) != 0) && ((c_prev & 0b10000000) != 0)){
				use_alt = ~use_alt;
		}
		else{
			if(((c & 0b01000000) == 0)!=0 && ((c_prev & 0b01000000) != 0)) {
					usb_keyboard_press(KEY_1, 0);
					LED_ON;
			}
			if(((c & 0b10000000) == 0)!=0 && ((c_prev & 0b10000000) != 0)) {
				usb_keyboard_press(KEY_2, 0);
				LED_ON;
			}
		}
		b_prev = b;
		c_prev = c;
		d_prev = d;
		f_prev = f;

		//wait to avoid bounce issues
		_delay_ms(2);
	}
}

