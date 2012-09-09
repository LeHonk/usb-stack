/*
Implementation of USB Device Firmware Upgrade Device Class version 1.1
$Id: dfu.c 81 2012-07-10 22:47:59Z tomhol $

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

/**
	On wire firmware blob protocol
	First four byte magic number; 0x48, 0x42, 0x4C, 0x01
	'HBL' for 'Honken BootLoader' and 0x01 for version 1.
	Then two bytes PIC microcontroller DEVID big endian, values as per datasheet,
	two bytes target memory number big endian
	    0 - Internal flash memory
	    1 - Internal eeprom memory - Not implemented
	    2 - External flash memory - Not implemented
	    3 - External eeprom memory - Not implemented
	Then blocks of data as
	4 byte block start address, assumed to be aligned with memory PAGE addresses
	n byte data, where n is equal to the size of a memory PAGE
	The blocks of data must come in non-decending start address order.
	If multiple blocks with the same address exist in a file,
	the last one will overwrite any previous ones.
 */

#include "dfu.h"
#include "usb_stack.h"
//#include <string.h>
#include <limits.h>
#include <assert.h>
#include <libpic30.h>

#define DFU_VID							0xDEAD
#define DFU_PID							0xF007
#define DFU_BCD							0x0100

/* DFU Request Codes */
#define DFU_DETACH					0x00
#define DFU_DNLOAD					0x01
#define DFU_UPLOAD					0x02
#define DFU_GETSTATUS				0x03
#define DFU_CLRSTATUS				0x04
#define DFU_GETSTATE				0x05
#define DFU_ABORT						0x06

/* DFU state machine states */
#define APP_IDLE						0u
#define APP_DETACH					1u
#define DFU_IDLE						2u
#define DFU_DNLOAD_SYNC			3u
#define DFU_DNLOAD_BUSY			4u
#define DFU_DNLOAD_IDLE			5u
#define DFU_MANIFEST_SYNC		6u
#define DFU_MANIFEST				7u
#define DFU_MANIFEST_WAIT		8u
#define DFU_UPLOAD_IDLE			9u
#define DFU_ERROR						10u

/* DFU Status codes */
#define DFU_OK							0x00
#define DFU_errTARGET				0x01
#define DFU_errFILE					0x02
#define DFU_errWRITE				0x03
#define DFU_errERASE				0x04
#define DFU_errCHECK_ERASED	0x05
#define DFU_errPROG					0x06
#define DFU_errVERIFY				0x07
#define DFU_errADDRESS			0x08
#define DFU_errNOTDONE			0x09
#define DFU_errFIRMWARE			0x0A
#define DFU_errVENDOR				0x0B
#define DFU_errUSBR					0x0C
#define DFU_errPOR					0x0D
#define DFU_errUNKNOWN			0x0E
#define DFU_errSTALLED_PKT	0x0F

/* String identifiers */
#define DFU_iManufacturer		1u
#define DFU_iProduct				2u
#define DFU_iSerialNum			3u
#define	DFU_iTarget					4u

/* DFU mode device descriptor */
const struct usb_device_descriptor_st dfu_device_descriptor = {
	sizeof(dfu_device_descriptor),	// bLength
	USB_DEVICE_DESCRIPTOR_TYPE,			// bDescriptorType
	0x0100,													// bcdUSB
	0x00,														// bDeviceClass
	0x00,														// bDeviceSubClass
	0x00,														// bDeviceProtocol
	DFU_XFER_SIZE,									// bMaxPacketSize0
	DFU_VID,												// idVendor
	DFU_PID,												// idProduct
	DFU_BCD,												// bcdDevice
	DFU_iManufacturer,							// iManucaturer
	DFU_iProduct,										// iProduct
	DFU_iSerialNum,									// iSerialNumber
	USB_NUM_CONFIGURATIONS					// bNumConfigurations
};

/* DFU mode configuration descriptor */
#define DFU_CONFIG_LENGTH (9+9+9)

ROM const struct {
	struct usb_configuration_descriptor_st	config;
	struct usb_interface_descriptor_st			dfu_interface;
	struct dfu_functional_descriptor_st			dfu_functional;
} dfu_config_descriptor = {
	{
		sizeof(struct usb_configuration_descriptor_st),	// bLength
		USB_CONFIGURATION_DESCRIPTOR_TYPE,							// bDescriptorType
		sizeof(dfu_config_descriptor),									// wTotalLength (low byte),
		0x01,																						// bNumInterfaces
		0x01,																						// bConfigurationValue
		0x00,																						// iConfiguration (0=none)
		0x80,																						// bmAttributes (0x80 = bus powered)
		0x32,																						// bMaxPower (in 2 mA units, 50=100 mA)
	},{
		sizeof(struct usb_interface_descriptor_st),			// bLength
		USB_INTERFACE_DESCRIPTOR_TYPE,									// bDescriptorType
		0x00,																						// bInterfaceNum
		0x00,																						// bAlternateSetting (ie Target memories; Flash, EEPROM...)
		0x00,																						// bNumEndpoints
		0xFE,																						// bInterfaceClass
		0x01,																						// bInterfaceSubClass
		0x02,																						// bInterfaceProtocol
		DFU_iTarget,																		// iInterface
	},
	DFU_FUNCTIONAL_DESC
};

#define DFU_NUM_STRINGS 7
ROM const USB_STRING_DESCRIPTOR(langid, USB_LANGID_English_United_States);
ROM const USB_STRING_DESCRIPTOR(manufacturer, 'H','o','n','k','e','n');
ROM const USB_STRING_DESCRIPTOR(product, 'D','F','U',' ','B','o','o','t','l','o','a','d','e','r'); /* iProduct - "DFU Bootloader" */
ROM const USB_STRING_DESCRIPTOR(serialnum, '0','0','0','0','0','0','0','0','0','0','0','1'); /* iSerialNumber - "000000000001" */
ROM const USB_STRING_DESCRIPTOR(target_main, 'M','a','i','n',' ','f','i','r','m','w','a','r','e',' ','t','a','r','g','e','t'); /* Main firmware target */
ROM const USB_STRING_DESCRIPTOR(target_eeprom, 'M','a','i','n',' ','e','e','p','r','o','m',' ','t','a','r','g','e','t'); /* Main eeprom target */
ROM const USB_STRING_DESCRIPTOR(target_external, 'E','x','t','e','r','n','a','l',' ','f','l','a','s','h',' ','r','o','m'); /* External flash rom */

ROM const struct usb_string_descriptor_st * dfu_string_descriptor[DFU_NUM_STRINGS] = {
	(const struct usb_string_descriptor_st *) &langid,
	(const struct usb_string_descriptor_st *) &manufacturer,
	(const struct usb_string_descriptor_st *) &product,
	(const struct usb_string_descriptor_st *) &serialnum,
	(const struct usb_string_descriptor_st *) &target_main,
	(const struct usb_string_descriptor_st *) &target_eeprom,
	(const struct usb_string_descriptor_st *) &target_external,
};

/* Flash write timeout ~8ms - 20ms to be on the safe side TODO: optimize, make MCU dependent */
#define FLASH_WRITE_TIMEOUT_LOW		0x14
#define FLASH_WRITE_TIMEOUT_MID		0x00
#define FLASH_WRITE_TIMEOUT_HIGH	0x00

// TODO: Improve and relocate
#define START_OF_BOOTLOADER			0x0200u
#define END_OF_BOOTLOADER       (((uint32_t) &_PROGRAM_END) | (DFU_PAGE_SIZE-1))
#define START_OF_USERAPP				((((uint32_t) &_PROGRAM_END) & ~(DFU_PAGE_SIZE-1)) + DFU_PAGE_SIZE)
#define GOTO_USERAPP						asm ("goto 0x001000")
#define	Reset()									asm ("reset")

/* dfu state machine variables */
volatile unsigned char dfuState __attribute__(( persistent, address ( 0x3FFC ) ));
volatile unsigned char dfuStatus __attribute__(( persistent, address ( 0x3FFE ) ));
unsigned char flash_write_pending;

/* Flash programmer */
uint16_t block_num;
uint32_t pgm_ptr;
uint8_t flash_buffer[DFU_PAGE_SIZE + DFU_XFER_SIZE];
uint16_t buffer_index;

//unsigned long rst_vector;

/* Forward declarations */
void dfu_setup( void );
void dfu_reset( void );
void dfu_write_pending( void );
static unsigned int produce_block( uint8_t *dst, uint16_t len, uint32_t src );
static unsigned int consume_block( const uint8_t *src, uint16_t len );
extern unsigned int get_devid( void );
extern void erase_flash( uint32_t dst );
extern void read_flash( uint8_t *dst, uint32_t src );
extern void write_flash( uint32_t dst, const uint8_t *src );
extern void usb_handle_reset( void );

void
dfu_runtime_init( uint8_t interface_num )
{
	dfuState = APP_IDLE;
	dfuStatus = DFU_OK;
	usb_register_class_setup_handler( interface_num, dfu_setup );
}

void
dfu_dfumode_main( void )
{

	// Steal the execution environment
	// DisableGlobalInterrupts();
	// Reset the stack and frame pointer
	// TODO: Make generic
	/*
	   _asm
	   LFSR 1, 0x0300
	   LFSR 2, 0x0300
	   _endasm
	 */

	usb_init( &dfu_device_descriptor,
						&dfu_config_descriptor.config,
		   			dfu_string_descriptor, DFU_NUM_STRINGS );
	buffer_index = 0;

//	usb_register_reset_handler( dfu_reset );
	usb_register_class_setup_handler( 0, dfu_setup );
	usb_start();

	// Busy wait at the usb_handler
	while (1) {
		usb_handler();
	}
}

void
dfu_setup( void )
{
	static uint16_t length;

	unsigned char *packet;

	packet = bdp->BDADDR;
	switch ( packet[USB_bmRequestType] &
		 ( USB_bmRequestType_TypeMask |
		   USB_bmRequestType_RecipientMask ) ) {
	case ( USB_bmRequestType_Class | USB_bmRequestType_Interface ):
		switch ( packet[USB_bRequest] ) {
		case DFU_DETACH:
			if ( APP_IDLE == dfuState ) {
				usb_ack_zero ( rbdp );
				// TODO: Delay perhaps?
				usb_stop();
				dfuState = APP_DETACH;
				dfuStatus = DFU_OK;
				Reset();
			}
			usb_ack_zero ( rbdp );
			break;
		case DFU_UPLOAD:
			switch ( dfuState ) {
				uint16_t packet_length;
				uint16_t byte_count;

			case DFU_IDLE:
				*(((uint32_t *) rbdp->BDADDR) + 0) = 0x014C4248;	// TODO: Struct candidate
				*(((uint16_t *) rbdp->BDADDR) + 4) = get_devid();
				*(((uint16_t *) rbdp->BDADDR) + 6) = 0x0000;
				rbdp->BDCNT = 8;
				usb_ack( rbdp );
				dfuState = DFU_UPLOAD_IDLE;
				dfuStatus = DFU_OK;
				pgm_ptr = 0;
				length = UINT_MAX;	// TODO: remove magic
				break;
			case DFU_UPLOAD_IDLE:
				if ( *((uint16_t *) &packet[USB_wLength]) < length )
					packet_length = *((uint16_t *) &packet[USB_wLength]);
				else
					packet_length = length;
				pgm_ptr += byte_count = produce_block( rbdp->BDADDR, packet_length, pgm_ptr );
				rbdp->BDCNT = packet_length;
				if ( 0 == length )
					dfuState = DFU_IDLE;
				length -= byte_count;
				usb_ack( rbdp );
				break;
			default:
				dfuState = DFU_ERROR;
				usb_ack_zero( rbdp );
			}
			break;
		case DFU_DNLOAD:
			switch ( dfuState ) {
			case DFU_IDLE:
				// Check Magic Number
				if ( *((uint32_t *) packet + 8 + 0) != 0x014C4248  ||	// TODO: Confirm
				     *((uint16_t *) packet + 8 + 4) != get_devid() ||
						 *((uint16_t *) packet + 8 + 6) != 0x0000 ) {
					dfuState = DFU_ERROR;
					dfuStatus = DFU_errTARGET;
					break;
				}
				pgm_ptr = 0;
				buffer_index = 0;
			case DFU_DNLOAD_IDLE:
				length = *((int16_t *) &packet[USB_wLength]);
				block_num = packet[USB_wValue];
				if ( length ) {
					dfuState = DFU_DNLOAD_SYNC;
					consume_block( &packet[USB_bData], length );
				} else {
					dfuState = DFU_MANIFEST_SYNC;
				}
				usb_ack_zero( rbdp );
				break;
			default:
				dfuState = DFU_ERROR;
			}
			break;
		case DFU_GETSTATUS:
			rbdp->BDADDR[0] = dfuStatus;
			rbdp->BDADDR[1] = 1;
			rbdp->BDADDR[2] = 0;
			rbdp->BDADDR[3] = 0;
//    rbdp->BDADDR[4] = dfuState;
			rbdp->BDADDR[5] = 0x00;	// No iString
			rbdp->BDCNT = 6;
			switch ( dfuState ) {
			case DFU_DNLOAD_SYNC:
				if ( flash_write_pending ) {
					dfuState = DFU_DNLOAD_BUSY;
					rbdp->BDADDR[1] = FLASH_WRITE_TIMEOUT_LOW;
					rbdp->BDADDR[2] = FLASH_WRITE_TIMEOUT_MID;
					rbdp->BDADDR[3] = FLASH_WRITE_TIMEOUT_HIGH;
					usb_set_in_handler( 0, dfu_write_pending );	// On ack we start writing flash page
				} else {
					dfuState = DFU_DNLOAD_IDLE;
				}
				break;
			case DFU_MANIFEST_SYNC:
				dfuState = DFU_MANIFEST;	// Stay in manifest, host don't know I we don't have to timeout.
				break;
			case DFU_MANIFEST:	// This is actually MANIFEST_SYNC after manifestation.
				dfuState = DFU_IDLE;
				break;
			default:
				break;
			}
			rbdp->BDADDR[4] = dfuState;
			usb_ack( rbdp );
			break;
		case DFU_CLRSTATUS:
			if ( DFU_ERROR == dfuState ) {
				dfuState = DFU_IDLE;
				dfuStatus = DFU_OK;
			}
			usb_ack_zero( rbdp );
			break;
		case DFU_GETSTATE:
			rbdp->BDADDR[0] = dfuState;
			rbdp->BDCNT = 1;
			usb_ack( rbdp );
			break;
		case DFU_ABORT:
			switch ( dfuState ) {
			case DFU_IDLE:
			case DFU_DNLOAD_SYNC:
			case DFU_DNLOAD_IDLE:
			case DFU_MANIFEST_SYNC:
			case DFU_UPLOAD_IDLE:
				dfuStatus = DFU_OK;
				dfuState = DFU_IDLE;
				break;
			default:
				dfuStatus = DFU_errUNKNOWN;
				dfuState = DFU_ERROR;
				break;
			}
			usb_ack_zero( rbdp );
			break;
		default:
			// TODO: Errorhandling - STALL control ep
			usb_RequestError();
			break;
		}
		break;
	default:
		// TODO: Errorhandling - STALL control ep
		usb_RequestError();
		break;
	}
}

// TODO: Reset OK except appIDLE or appDETACH
void
dfu_reset( void )
{
	if ( DFU_MANIFEST_WAIT == dfuState /*|| (DFU_IDLE == dfuState && DFU_OK == dfuStatus) */  )
	{
		dfuState = APP_IDLE;
		Reset();
	} else {
		dfuState = DFU_ERROR;
		dfuStatus = DFU_errUSBR;
		usb_handle_reset();
	}
}

// TODO: Assert PAGE alignment
// TODO: Write protect Bootloader, Resetvector
void
dfu_write_pending( void )
{
	uint16_t fragment_length = buffer_index - DFU_PAGE_SIZE;

	usb_unset_in_handler( 0 );
	if ( pgm_ptr == 0 ) {
		/* Protect reset vector */
		flash_buffer[0] = 0x04;                     /* TODO: Linktime argument - START_OF_BOOTLOADER */
		flash_buffer[1] = 0x02;
		flash_buffer[2] = 0x00;
		flash_buffer[3] = 0x00;
		flash_buffer[4] = 0x00;
		flash_buffer[5] = 0x00;
	}
	/* Protect bootloader and config words */
	if ( !( START_OF_BOOTLOADER <= (pgm_ptr & ~(DFU_PAGE_SIZE-1)) && 
				(pgm_ptr | (DFU_PAGE_SIZE-1)) < END_OF_BOOTLOADER )
	     && !( 0x2ABFC <= pgm_ptr ) ) {
		write_flash( pgm_ptr, flash_buffer );
	}
	// consume possible fragment of previous block
	buffer_index = 0;
	consume_block( &flash_buffer[DFU_PAGE_SIZE], fragment_length );
	dfuState = DFU_DNLOAD_SYNC;
}

/* Consume firmware block at "data" of size "length"
 * return number of corresponding flash memory bytes */
static unsigned int
consume_block( const uint8_t *data, uint16_t length )
{
	unsigned int write_length = 0;

	static unsigned int state = 0;

	switch ( state ) {
	case 0:				// Consume LSB of 32 bit adress
		if ( length ) {
			pgm_ptr = (unsigned long) *data++;
			length--;
			state++;
		} else
			break;
	case 1:
		if ( length ) {
			pgm_ptr |= (unsigned long) *data++ << 8;
			length--;
			state++;
		} else
			break;
	case 2:
		if ( length ) {
			pgm_ptr |= (unsigned long) *data++ << 16;
			length--;
			state++;
		} else
			break;
	case 3:				// Consume MSB of 32 bit adress
		if ( length ) {
			pgm_ptr |= (unsigned long) *data++ << 24;
			length--;
			state++;
		} else
			break;
	default:
		while ( length && DFU_PAGE_SIZE < buffer_index ) {
			flash_buffer[buffer_index++] = *data++;
			buffer_index %= sizeof(flash_buffer);	// Buffer overflow guard
			length--;
			write_length++;
		}
		if ( DFU_PAGE_SIZE == buffer_index ) {
			flash_write_pending = 1;
			// Save rest of packet to the end of the flash_buffer
			while ( length ) {
				flash_buffer[buffer_index++] = *data++;
				length--;
			}
		}
	}
	return write_length;
}

static unsigned int
produce_block( uint8_t *data, uint16_t length, uint32_t src )
{
	unsigned int read_length = 0;

	static unsigned int state = 0;

	switch ( state ) {
	case 0:
		if ( length ) {
			*data++ = src & 0xFF;
			length--;
			state++;
		} else
			break;
	case 1:
		if ( length ) {
			*data++ = ( src >> 8 ) & 0xFF;
			length--;
			state++;
		} else
			break;
	case 2:
		if ( length ) {
			*data++ = ( src >> 16 ) & 0xFF;
			length--;
			state++;
		} else
			break;
	case 3:
		if ( length ) {
			*data++ = ( src >> 24 ) & 0xFF;
			length--;
			state++;
			read_flash( flash_buffer, src );
			src += DFU_PAGE_SIZE;
			buffer_index = 0;
		} else
			break;
	default:
		// TODO: Hide replaced reset vector
		for ( ; length && buffer_index < DFU_PAGE_SIZE; length-- ) {
			*data++ = flash_buffer[buffer_index++];
			read_length++;
		}
		if ( DFU_PAGE_SIZE == buffer_index )	// TODO: Check that this is run in end condition
			state = 0;
	}
	return read_length;
}

#if 1
#include <p24Fxxxx.h>
#include <pps.h>
#include <uart.h>

//extern void USERAPP(void);
int
main ( void )
{
	CORCONbits.PSV = 1;			// PSV not being initialized. May have been done by c_init though.
	PSVPAG = 0;
	CLKDIV = 0x0000;				// Set PLL prescaler (1:1) 32 Mhz Fcy
	AD1PCFGL = 0x7FD8;			// BPv4 has five analog pins b0, b1, b2, b5, b15
	AD1PCFGH = 0x0002;
	TRISB = 0x8026;					// Analog pins are input, RB0 output

	__builtin_write_OSCCONL( 0x00 );	// Disable secondary oscillator

//      CLKDIVbits.PLLEN = 1;
//      unsigned int cnt = 600;
//      while (cnt--);

#if defined(__DEBUG)
	iPPSOutput( OUT_PIN_PPS_RP7, OUT_FN_PPS_U2TX );	// PGD?
	OpenUART2( UART_EN | UART_NO_PAR_8BIT | UART_1STOPBIT, UART_TX_ENABLE, 105 );	// 105 = 19200bps @ 32Mhz ?
#endif

	/* Jumper check test */
	_LATB1 = 0;							// RB1 low
	_CN5PUE = 1;						// Enable pull-up on PGC/CN5/RB1
	_LATB0 = 0;							// RB0 low
	Nop (  );							// Small delay to settle parasitic capacitance
	Nop (  );
	// Possible future TODO: Checksum current FW and stay in bootloader if corrupt
/*   if (!_RB1 || APP_DETACH == dfuState) */
	{
		dfuState = DFU_IDLE;
		dfuStatus = DFU_OK;
		dfu_dfumode_main();
	}
	dfuState = APP_IDLE;
	dfuStatus = DFU_OK;
	GOTO_USERAPP;
	return 0;
}
#endif

