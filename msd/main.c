/*
Main program, implements a MSD MMC Reader.


This work is licensed under the Creative Commons Attribution 3.0 Unported License.
To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
or send a letter to
	Creative Commons,
	171 Second Street,
	Suite 300,
	San Francisco,
	California,
	94105,
	USA.
*/

/* As this is just a test application for the stack this file is intentionally kept cluttered
   with #ifdef's. Better put the efort of facy abstractions to the part of the stack that needs it
   for easier maintanence */

#include <p24Fxxxx.h>
#include "usb_stack.h"
#include "msd.h"

#if defined(__PIC24FJ256GB106__)
// BPv4
#define USB_LED(x)              	(LATBbits.LATB10 = (x))
#define STA_LED(x)              	(LATBbits.LATB8 = (x))
#define STA_LED_TOGGLE  			(LATBbits.LATB8 ^= 1)
#else
// Everyting else
#define USB_LED(x)
#define STA_LED(x)
#define STA_LED_TOGGLE
#endif

void      init ( void );			//hardware init

int
main ( void )
{

	USB_LED ( 0 );
	init (  );
	DINIT (  );
	DPRINTF ( "\n\nMSD MMC Reader test program\n" );
	InitMSD (  );
	USB_LED ( 1 );

	// Infinity local echo
	while ( 1 ) {
	}

	return 0;
}

// hardware init
void
init ( void )
{
	CORCONbits.PSV = 1;			// JTR PIC24 fixup ?? PSV not being initialized. May have been done by c_init though.
	PSVPAG = 0;
	CLKDIV = 0x0000;			// Set PLL prescaler (1:1) 32 Mhz Fcy
	AD1PCFGL = 0x7FD8;			// BPv4 has five analog pins b0, b1, b2, b5, b15
	AD1PCFGH = 0x0002;
	TRISB = 0x8027;				// Analog pins are input
	TRISC = TRISD = TRISE = TRISF = TRISG = 0x0000;
	LATB = LATC = LATD = LATE = LATF = LATG = 0x0000;

//      OSCCONbits.SOSCEN=0;                    // Disable secondary oscillator
	__builtin_write_OSCCONL ( 0x00 );	// Disable secondary oscillator

	//CLKDIVbits.PLLEN = 1;
	unsigned int cnt = 600;

	while ( cnt-- );

	IPC21bits.USB1IP = 7;
	INTCON1bits.NSTDIS = 1;
}

// Interrupt handlers
void __attribute__ ( ( interrupt, auto_psv ) ) _USB1Interrupt ( void )
{
	//USB interrupt
	//IRQ enable IEC5bits.USB1IE
	//IRQ flag      IFS5bits.USB1IF
	//IRQ priority IPC21<10:8>
	STA_LED ( 1 );
	usb_handler (  );
	IFS5bits.USB1IF = 0;
	STA_LED ( 0 );
}

/*
// For debug purposes
void __attribute__((interrupt,no_auto_psv)) _AddressError( void ) {
	register unsigned short int *fp asm("w14");
	DPRINTF("\n\n0x%04X\n", fp[-2]);
	__asm__ volatile ("reset");
}
*/
