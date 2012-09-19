/* 
 *
 *	License: creative commons - attribution, share-alike 
 *	Copyright Ian Lesnet 2010
 *	http://dangerousprototypes.com
 *
 */

#ifndef CONFIG_H
#define CONFIG_H

#if defined(__PIC_18F)
#define MAINT_t void
#define MAIN_END Nop()

#include <p18cxxx.h>
#if defined(__18F14K50)
#pragma config CPUDIV	= NOCLKDIV
#pragma config USBDIV	= OFF
#pragma config FOSC		= HS
#pragma config PLLEN	= ON
#pragma config PCLKEN	= OFF
#pragma config FCMEN	= OFF
#pragma config IESO		= OFF
#pragma config PWRTEN	= OFF
#pragma config BOREN	= OFF
#pragma config BORV		= 30
#pragma config WDTEN	= OFF
#pragma config WDTPS	= 32768
#pragma config HFOFST	= ON
#pragma config MCLRE	= ON
#pragma config STVREN	= ON
#pragma config LVP		= OFF
#pragma config XINST	= OFF
#pragma config CP0		= OFF
#pragma config CP1		= OFF
#pragma config CPB		= OFF
#pragma config CPD		= OFF
#pragma config WRT0		= OFF
#pragma config WRT1		= OFF
#pragma config WRTB		= OFF
#pragma config WRTC		= OFF
#pragma config WRTD		= OFF
#pragma config EBTR0	= OFF
#pragma config EBTR1	= OFF
#pragma config EBTRB	= OFF
#elif defined(__18F2450) || defined(__18F2550) || defined(__18F4450) || defined(__18F4550)
#include <p18cxxx.h>
#pragma config PLLDIV	= 5
#pragma config CPUDIV   = OSC1_PLL2
#pragma config USBDIV   = 2
#pragma config FOSC     = HSPLL_HS
#pragma config FCMEN    = OFF
#pragma config IESO     = OFF
#pragma config PWRT     = OFF
#pragma config BOR      = ON
#pragma config BORV     = 3
#pragma config VREGEN   = ON
#pragma config WDT      = OFF
#pragma config WDTPS    = 32768
#pragma config MCLRE    = ON
#pragma config LPT1OSC  = OFF
#pragma config PBADEN   = OFF
//      #pragma config CCP2MX   = ON
#pragma config STVREN   = ON
#pragma config LVP      = OFF
//      #pragma config ICPRT    = OFF       
#pragma config XINST    = OFF
#pragma config CP0      = OFF
#pragma config CP1      = OFF
//      #pragma config CP2      = OFF
//      #pragma config CP3      = OFF
#pragma config CPB      = OFF
//      #pragma config CPD      = OFF
#pragma config WRT0     = OFF
#pragma config WRT1     = OFF
//      #pragma config WRT2     = OFF
//      #pragma config WRT3     = OFF
#pragma config WRTB     = OFF
#pragma config WRTC     = OFF
//      #pragma config WRTD     = OFF
#pragma config EBTR0    = OFF
#pragma config EBTR1    = OFF
//      #pragma config EBTR2    = OFF
//      #pragma config EBTR3    = OFF
#pragma config EBTRB    = OFF
#elif defined(__18F24J50)
#include <p18cxxx.h>
#pragma config WDTEN	= OFF
//      #ifdef XTAL_20MHZ
//      #pragma config PLLDIV   = 5           //Divide by 5 (20 MHz oscillator input)
//      #else
#pragma config PLLDIV	= 4			//Divide by 4 (16 MHz oscillator input)
//      #endif
#pragma config STVREN	= ON
#pragma config XINST	= OFF
#pragma config CPUDIV	= OSC1
#pragma config CP0		= OFF
#pragma config OSC		= HSPLL
#pragma config T1DIG	= ON
#pragma config LPT1OSC	= OFF
#pragma config FCMEN	= OFF
#pragma config IESO		= OFF
#pragma config WDTPS	= 32768
#pragma config DSWDTOSC	= INTOSCREF
#pragma config RTCOSC	= T1OSCREF
#pragma config DSBOREN	= OFF
#pragma config DSWDTEN	= OFF
#pragma config DSWDTPS	= 8192
#pragma config IOL1WAY	= OFF
#pragma config MSSP7B_EN = MSK7
#pragma config WPFP		= PAGE_1
#pragma config WPEND	= PAGE_0
#pragma config WPCFG	= OFF
#pragma config WPDIS	= OFF
#endif

#elif defined(__PIC24F__)

#include <p24Fxxxx.h>
#define MAIN_t int
#define MAIN_END return 0;

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ256GB110__)
_CONFIG1 ( JTAGEN_OFF & GCP_OFF & GWRP_OFF & COE_OFF & FWDTEN_OFF & ICS_PGx2 )
	_CONFIG2 ( IESO_OFF & PLLDIV_DIV3 & PLL_96MHZ_ON & FNOSC_PRIPLL & FCKSM_CSDCMD
	   & OSCIOFNC_ON & IOL1WAY_OFF & POSCMOD_XT )
	_CONFIG3 ( WPDIS_WPDIS )
#endif
#endif	/* PIC_24F */

#endif /* CONFIG_H */
