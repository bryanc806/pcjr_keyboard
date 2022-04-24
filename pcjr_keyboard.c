/* Keyboard example for Teensy USB Development Board
 * http://www.pjrc.com/teensy/usb_keyboard.html
 * Copyright (c) 2008 PJRC.COM, LLC
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usb_keyboard.h"

#define LED_OFF		(PORTB &= ~(1<<PB2))
#define LED_ON		(PORTB |= (1<<PB2))
#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

uint16_t idle_count=0;
struct keytable
{
	unsigned char	key;
	unsigned char	fnkey;
	unsigned char	fnkey_modifier;
	unsigned char	modifier;
};

struct keytable	pcjr_key_to_usb[] =
{
	{0,		0,		0,		0},		// 0x00
	{KEY_ESC,	0,		0,		0},		// 0x01
	{KEY_1,		KEY_F1,		0,		0},		// 0x02
	{KEY_2,		KEY_F2,		0,		0},		// 0x03
	{KEY_3,		KEY_F3,		0,		0},		// 0x04
	{KEY_4,		KEY_F4,		0,		0},		// 0x05
	{KEY_5,		KEY_F5,		0,		0},		// 0x06
	{KEY_6,		KEY_F6,		0,		0},		// 0x07
	{KEY_7,		KEY_F7,		0,		0},		// 0x08
	{KEY_8,		KEY_F8,		0,		0},		// 0x09
	{KEY_9,		KEY_F9,		0,		0},		// 0x0a
	{KEY_0,		KEY_F10,	0,		0},		// 0x0b
	{KEY_MINUS,	0,		0,		0},		// 0x0c
	{KEY_EQUAL,	0,		0,		0},		// 0x0d
	{KEY_BACKSPACE,	0,		0,		0},		// 0x0e
	{KEY_TAB,	0,		0,		0},		// 0x0f
	{KEY_Q,		KEY_PAUSE,	0,		0},		// 0x10
	{KEY_W,		0,		0,		0},		// 0x11
	{KEY_E,		0,		0,		0},		// 0x12	KEY_ECHO???
	{KEY_R,		0,		0,		0},		// 0x13
	{KEY_T,		0,		0,		0},		// 0x14
	{KEY_Y,		0,		0,		0},		// 0x15
	{KEY_U,		0,		0,		0},		// 0x16
	{KEY_I,		0,		0,		0},		// 0x17
	{KEY_O,		0,		0,		0},		// 0x18
	{KEY_P,		KEY_PRINTSCREEN,0,		0},		// 0x19
	{KEY_LEFT_BRACE,KEY_BACKSLASH,	KEY_SHIFT,	0},		// 0x1A
	{KEY_RIGHT_BRACE,KEY_TILDE,	KEY_SHIFT,	0},		// 0x1B
	{KEY_ENTER,	0,		0,		0},		// 0x1C
	{0,		0,		0,		KEY_CTRL},	// 0x1D
	{KEY_A,		0,		0,		0},		// 0x1E
	{KEY_S,		KEY_SCROLL_LOCK,0,		0},		// 0x1F
	{KEY_D,		0,		0,		0},		// 0x20
	{KEY_F,		0,		0,		0},		// 0x21
	{KEY_G,		0,		0,		0},		// 0x22
	{KEY_H,		0,		0,		0},		// 0x23
	{KEY_J,		0,		0,		0},		// 0x24
	{KEY_K,		0,		0,		0},		// 0x25
	{KEY_L,		0,		0,		0},		// 0x26
	{KEY_SEMICOLON,	0,		0,		0},		// 0x27
	{KEY_QUOTE,	KEY_TILDE,	0,		0},		// 0x28	// backquote
	{0,		0,		0,		0},		// 0x29 not used
	{0,		0,		0,		KEY_LEFT_SHIFT},// 0x2A
	{0,		0,		0,		0},		// 0x2B not used
	{KEY_Z,		0,		0,		0},		// 0x2C
	{KEY_X,		0,		0,		0},		// 0x2D
	{KEY_C,		0,		0,		0},		// 0x2E
	{KEY_V,		0,		0,		0},		// 0x2F
	{KEY_B,		0,		0,		0},		// 0x30	break
	{KEY_N,		0,		0,		0},		// 0x31
	{KEY_M,		0,		0,		0},		// 0x32
	{KEY_COMMA,	0,		0,		0},		// 0x33
	{KEY_PERIOD,	0,		0,		0},		// 0x34
	{KEY_SLASH,	KEY_BACKSLASH,	0,		0},		// 0x35
	{0,		0,		0,		KEY_RIGHT_SHIFT},// 0x36
	{0,		0,		0,		0},		// 0x37
	{0,		0,		0,		KEY_ALT},		// 0x38
	{KEY_SPACE,	0,		0,		0},		// 0x39
	{KEY_CAPS_LOCK,	0,		0,		0},		// 0x3A
	{0,		0,		0,		0},		// 0x3B
	{0,		0,		0,		0},		// 0x3C
	{0,		0,		0,		0},		// 0x3D
	{0,		0,		0,		0},		// 0x3E
	{0,		0,		0,		0},		// 0x3F
	{0,		0,		0,		0},		// 0x40
	{0,		0,		0,		0},		// 0x41
	{0,		0,		0,		0},		// 0x42
	{0,		0,		0,		0},		// 0x43
	{0,		0,		0,		0},		// 0x44
	{0,		0,		0,		0},		// 0x45
	{0,		0,		0,		0},		// 0x46
	{0,		0,		0,		0},		// 0x47
	{KEY_UP,	KEY_HOME,	0,		0},		// 0x48
	{0,		0,		0,		0},		// 0x49
	{0,		0,		0,		0},		// 0x4A
	{KEY_LEFT,	KEY_PAGE_UP,	0,		0},		// 0x4B
	{0,		0,		0,		0},		// 0x4C
	{KEY_RIGHT,	KEY_PAGE_DOWN,	0,		0},		// 0x4D
	{0,		0,		0,		0},		// 0x4E
	{0,		0,		0,		0},		// 0x4F
	{KEY_DOWN,	KEY_END,	0,		0},		// 0x50
	{0,		0,		0,		0},		// 0x51
	{KEY_INSERT,	0,		0,		0},		// 0x52
	{KEY_DELETE,	0,		0,		0},		// 0x53
	{0,		0,		0,		KEY_GUI},	// 0x54 - FN
	{0,		0,		0,		0}		// 0x55
};
//PB2 led
// PD7	hwb
// PD3 input from pcjr
#define	MYPIN	(PIND & (1 << PD3))
int main(void)
{
	// set for 8 MHz clock
	CPU_PRESCALE(0);

	// Configure all port B and port D pins as inputs with pullup resistors.
	// See the "Using I/O Pins" page for details.
	// http://www.pjrc.com/teensy/pins.html
	DDRD = 1 << PB2;
	DDRB = 0x00;
	PORTB = 0x00;
	PORTD = 0x00;

	LED_OFF;
	// Initialize the USB, and then wait for the host to set configuration.
	// If the Teensy is powered without a PC connected to the USB port,
	// this will wait forever.
	usb_init();
	while (!usb_configured()) /* wait */ ;

	// Wait an extra second for the PC's operating system to load drivers
	// and do whatever it does to actually be ready for input
	_delay_ms(1000);

	// Configure timer 0 to generate a timer overflow interrupt every
	// 256*1024 clock cycles, or approx 61 Hz when using 16 MHz clock
	// This demonstrates how to use interrupts to implement a simple
	// inactivity timeout.
	TCCR0A = 0x00;
	TCCR0B = 0x05;
	TIMSK0 = (1<<TOIE0);

	unsigned char	local_modifier = 0;
	while (1)
	{
		if (!MYPIN)	// transition to low...
		{
			_delay_us(440+110);	// 1.25 bits, e.g. go .25bits into the first bit
			unsigned short c;
			unsigned char i;
			c = 0;
			unsigned char d;
			for (i = 0; i < 9; i++)
			{
				d = MYPIN;
				if (!d) c |= (1 << i);
				_delay_us(439);  // 1 bit ish
			}
			d = 0;
			for (i =0 ; i < 8; i++)	// parity
				if (c & (1 << i))  d++;
			if ((c & 0x100) ^ (d & 0x01))	// parity good
			{
				c &=0xff;	// remove stop bit
				if (c & 0x80)	// release key
				{
					c &= 0x7f;
					if (c < 0x55)
					{
						if (pcjr_key_to_usb[c].modifier == KEY_GUI)
						{
							local_modifier = 0;
						}
						else if (pcjr_key_to_usb[c].modifier)
						{
							keyboard_modifier_keys &= ~pcjr_key_to_usb[c].modifier;
						}
						else
						{
							unsigned char key;
							if (local_modifier == 0)
								key = pcjr_key_to_usb[c].key;
							else
							{
								key = pcjr_key_to_usb[c].fnkey;
								keyboard_modifier_keys &= ~(local_modifier & ~KEY_GUI);
							}
							for (i = 0; i < 6; i++)
							{
								if (keyboard_keys[i] == key)
								{
									keyboard_keys[i] = 0;
									break;
								}
							}
						}
					}
				}
				else
				{
					if (c < 0x55)
					{
						if (pcjr_key_to_usb[c].modifier == KEY_GUI)
						{
							local_modifier = KEY_GUI;	// reset all local modifiers on press
						}
						else if (pcjr_key_to_usb[c].modifier)
						{
							keyboard_modifier_keys  |= pcjr_key_to_usb[c].modifier;
						}
						else
						{
							unsigned char key;
							if ((local_modifier & KEY_GUI) == 0)
								key = pcjr_key_to_usb[c].key;
							else
							{
								key = pcjr_key_to_usb[c].fnkey;
								local_modifier |= pcjr_key_to_usb[c].fnkey_modifier;
								keyboard_modifier_keys |= (local_modifier & ~KEY_GUI);
							}
							for (i = 0; i < 6; i++)
							{
								if (keyboard_keys[i] == 0)
								{
									keyboard_keys[i] = key;
									break;
								}
								if (keyboard_keys[i] == key)
									break;
							}
						}
					}
				}
				usb_keyboard_send();
			}
		}
		if (keyboard_leds & 0x02)	// caps lock
			LED_ON;
		else
			LED_OFF;
	}
}

// This interrupt routine is run approx 61 times per second.
// A very simple inactivity timeout is implemented, where we
// will send a space character.
ISR(TIMER0_OVF_vect)
{
	idle_count++;
	if (idle_count > 61) {
		idle_count = 0;
	}
}


