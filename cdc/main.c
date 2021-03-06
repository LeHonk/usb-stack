/*
Main program, implements an echo service on a USB CDC ACM connection.


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

#include "config.h"
#include "usb_stack.h"
#include "cdc.h"

#if defined(__18f2550)
// IRtoy
#define USB_LED(x)
#define STA_LED(x)
#define STA_LED_TOGGLE
#elif defined(__18F24J50)
// OLS
#define USB_LED(x)
#define STA_LED(x)
#define STA_LED_TOGGLE
#elif defined(__18F14K50)
// Someones something
#define USB_LED(x)
#define STA_LED(x)			(LATBbits.LATB6 = (x))
#define STA_LED_TOGGLE			(LATBbits.LATB6 ^= 1)
#elif defined(__PIC24FJ256GB106__)
// BPv4
#define USB_LED(x)              	(LATBbits.LATB10 = (x))
#define STA_LED(x)              	(LATBbits.LATB8 = (x))
#define STA_LED_TOGGLE  		(LATBbits.LATB8 ^= 1)
#else
// Everyting else
#define USB_LED(x)
#define STA_LED(x)
#define STA_LED_TOGGLE
#endif

void init( void );                              //hardware init

MAIN_t
main ( void )
{
	unsigned char c;

	USB_LED( 0 );
	init();
	DINIT();
	DPRINTF( "\n\nCDC-ACM Echo test program\n" );
	InitCDC();
	USB_LED( 1 );

	// Infinity local echo
	while ( 1 ) {
		while ( !DataRdyCDC() )
			/* usb_handler() */ ;
		c = getcCDC();
		while ( BusyCDC() )
			/* usb_handler() */ ;
		putcCDC( c );
	}
	MAIN_END;
}

// hardware init
void
init( void )
{
// init of pic18 platforms
#if defined(PIC_18F)
#if defined(__18F14K50)
	PORTA = 0x00;
        PORTB = 0x00;                           // init port
	TRISA = 0xFF;
        TRISB = 0x00;                           // RB6 = debug LED(tm)
	LATB = 0xFF;
#elif defined(__18F2450) || defined(__18F2550) || defined(__18F4450) || defined(__18F4550)
	//disable some defaults
	ADCON1 |= 0 b1111;			//all pins digital
	CVRCON = 0 b00000000;
#elif defined(__18F24J50)                       //OLS
	//all pins digital
	ANCON0 = 0xFF;
        ANCON1 = 0 b00011111;                   // updated for lower power consumption. See datasheet page 343                  
	//make sure everything is input (should be on startup, but just in case)
	TRISA = 0xff;
	TRISB = 0xff;
	TRISC = 0xff;
	//on 18f24j50 we must manually enable PLL and wait at least 2ms for a lock
        OSCTUNEbits.PLLEN = 1;                  //enable PLL
	unsigned int cnt = 2048;

        while ( cnt-- );                        //wait for lock
#endif //defined(__18F24J50)
	// pic18 interrupts
	//setup USB as interrupt service
        RCONbits.IPEN = 1;                      // enable interrupt priorities
	PIR1 = PIR2 = 0;
        PIE2bits.USBIE = 1;                     // Enable USB interrupts
        IPR2bits.USBIP = 0;                     // USB interrupt low priority
        INTCONbits.GIEL = 1;                    // enable peripheral interrupts
        INTCONbits.GIE = 1;                     // enable interrupts
#endif //defined(PIC_18F)

// pic24 inits
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ256GB110__)
        CORCONbits.PSV = 1;                     // JTR PIC24 fixup ?? PSV not being initialized. May have been done by c_init though.
	PSVPAG = 0;
        CLKDIV = 0x0000;                        // Set PLL prescaler (1:1) 32 Mhz Fcy
        AD1PCFGL = 0x7FD8;                      // BPv4 has five analog pins b0, b1, b2, b5, b15
	AD1PCFGH = 0x0002;
        TRISB = 0x8027;                         // Analog pins are input
	TRISC = TRISD = TRISE = TRISF = TRISG = 0x0000;
	LATB = LATC = LATD = LATE = LATF = LATG = 0x0000;



//      OSCCONbits.SOSCEN=0;                    // Disable secondary oscillator
        __builtin_write_OSCCONL( 0x00 );        // Disable secondary oscillator and unlock registers
// Configure Output Functions
        _RP7R = 5;                              // Assign U2TX To Pin RP2
	_TRISB7 = 0;																	// RP7 output

	//CLKDIVbits.PLLEN = 1;
	unsigned int cnt = 600;

	while ( cnt-- );

        _NSTDIS = 1;                            // No nested interrupts
        _USB1IP = 7;                            // USB interrupt priority 7
	_USB1IF = 0;																	// Clear USB Interrupt Flag
	_USB1IE = 1;																	// Enable USB Interrupts

        __builtin_write_OSCCONL(OSCCON | 0x40); // Lock Registers 
#endif //defined(__PI24FJ...)
}

// Interrupt handlers
#if defined(PIC_18F)

#pragma interrupt High_ISR
void
High_ISR( void )
{
}

#pragma interrupt Low_ISR
void
Low_ISR( void )
{
	if ( PIR2bits.USBIF ) {
		usb_handler (  );
		PIR2bits.USBIF = 0;
	}
}

#pragma code HIGH_INTERRUPT_VECTOR = 0x08
void
interrupt_high( void )
{
	_asm GOTO High_ISR _endasm
}

#pragma code LOW_INTERRUPT_VECTOR = 0x18
void
interrupt_low( void )
{
	_asm GOTO Low_ISR _endasm
}

#elif defined(PIC_24F)

void __attribute__(( interrupt, auto_psv )) _USB1Interrupt( void )
{
	//USB interrupt
	//IRQ enable IEC5bits.USB1IE
	//IRQ flag      IFS5bits.USB1IF
	//IRQ priority IPC21<10:8>
	STA_LED( 1 );
	usb_handler();
	IFS5bits.USB1IF = 0;
	STA_LED( 0 );
}

#if 0
// For debug purposes
void __attribute__ ( ( interrupt, no_auto_psv ) ) _AddressError ( void )
{
	register unsigned short int *fp asm ( "w14" );

	DPRINTF ( "\n\n0x%04X\n", fp[-2] );
	__asm__ volatile ( "reset" );
}
#endif

#endif
