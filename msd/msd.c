/*
Implementation of USB MSD


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

#include <stdint.h>
#include <string.h>
#include <libpic30.h>
#include "usb_stack.h"
#include "msd.h"
#include "scsi.h"
#include "descriptors.h"

#define MSD_MAX_LUN 1

static void msd_setup( void );
static void receive_cbw( void );
static void data_out( void );
static void data_in( void );
static void send_csw_ack( void );
static void reset_csw( void );

#pragma udata usb_data
uint8_t   msd_rx_buffer[MSD_RXTX_BUFFER_SIZE];
uint8_t   msd_tx_buffer[MSD_RXTX_BUFFER_SIZE];
#pragma udata

BDentry  *rxbdp, *txbdp;
CBW_t     cbw;
CSW_t     csw;
uint8_t   msd_block_buffer[512];

static uint8_t  *msd_subblock_ptr;

void
InitMSD( void )
{
	// Honken: JTR makes compelling argument to move this into user app, but I'm not convinced yet
	usb_init( &msd_device_descriptor, &msd_config_descriptor.config, msd_str_descs, USB_NUM_STRINGS );
	rxbdp = &usb_bdt[USB_CALC_BD( 1, USB_DIR_OUT, USB_PP_EVEN )];
	txbdp = &usb_bdt[USB_CALC_BD( 1, USB_DIR_IN, USB_PP_EVEN )];
	usb_register_endpoint( 1, USB_EP_INOUT,
				MSD_RXTX_BUFFER_SIZE, msd_rx_buffer, msd_tx_buffer,
				receive_cbw, NULL );
	usb_register_class_setup_handler( 0, msd_setup );

	// Should follow the usb_init...
	usb_start (  );
}

void
msd_send_buffer( void )
{
	msd_subblock_ptr = msd_block_buffer;
	usb_set_in_handler( 1, data_in );
	data_in();
}

void
msd_recv_buffer( void ) {
	msd_subblock_ptr = msd_block_buffer;
	usb_set_out_handler( 1, data_out );
}

void
msd_send_csw( void )
{
	memcpy( msd_tx_buffer, (uint8_t *) & csw, sizeof(CSW_t) );
	txbdp->BDADDR = msd_tx_buffer;
	txbdp->BDCNT = sizeof(CSW_t);
	usb_ack( txbdp );
	usb_set_in_handler( 1, send_csw_ack );
}

static void
msd_setup ( void )
{
	unsigned char *packet;

	packet = bdp->BDADDR;
	DPRINTF ( "IFace %02u ", packet[USB_bInterface] );
	switch ( packet[USB_bmRequestType] &
		 ( USB_bmRequestType_TypeMask |
		   USB_bmRequestType_RecipientMask ) ) {
	case ( USB_bmRequestType_Class | USB_bmRequestType_Interface ):
		switch ( packet[USB_bRequest] ) {
		default:
			usb_RequestError (  );
		}
		break;
	default:
		usb_RequestError (  );
	}
}

static void
data_out ( void )
{
	DPRINTF( "msd_out\n" );
	memcpy( msd_subblock_ptr, msd_rx_buffer, rxbdp->BDCNT );	// TODO: Warning buffer overflow
	msd_subblock_ptr += rxbdp->BDCNT;
	cbw.dDataTransferLenght -= rxbdp->BDCNT;
	csw.dDataResidue -= rxbdp->BDCNT;

	rxbdp->BDCNT = MSD_RXTX_BUFFER_SIZE;
	usb_ack( rxbdp );

	if ( 0 == csw.dDataResidue ) {
		usb_unset_out_handler( 1 );
		scsi_data_received();
	}
}

static void
data_in( void )
{
	uint8_t   bytes_to_send;

	DPRINTF( "msd_in\n" );
	bytes_to_send = ( csw.dDataResidue > MSD_RXTX_BUFFER_SIZE ) ? MSD_RXTX_BUFFER_SIZE : (uint8_t) csw.dDataResidue;	// Assert( bytes_to_send <= DataResidue <= MSD_RXTX_BUFFER_SIZE )
	memcpy( msd_tx_buffer, msd_subblock_ptr, bytes_to_send );
	msd_subblock_ptr += bytes_to_send;
	cbw.dDataTransferLenght -= bytes_to_send;
	csw.dDataResidue -= bytes_to_send;
	txbdp->BDCNT = bytes_to_send;
	usb_ack( txbdp );
	if ( 0 == csw.dDataResidue ) {
		usb_unset_in_handler( 1 );                  /* TODO: Check if ZLP needed */
		scsi_data_sent();
	}
}

static void
receive_cbw( void )
{
	// Check validity and meaningfulness of CBW
	if ( ( sizeof(CBW_t) == rxbdp->BDCNT) &&
	     ( MSD_CBW_Signature == ((CBW_t *) rxbdp->BDADDR)->dSignature ) &&
	     ( MSD_MAX_LUN >= cbw.bLUN ) && ( 0x01 <= cbw.bCBLength ) &&
	     /*      (cbw.bCBLength <= 0x10) && */
	     ( 0 == (cbw.bmFlags & ~0x80) ) ) {
		memcpy( &cbw, rxbdp->BDADDR, sizeof(CBW_t) );	// Buffer overrun checked above
		reset_csw();
		scsi_handle_cdb();
	}
	rxbdp->BDCNT = MSD_RXTX_BUFFER_SIZE;
	usb_ack( rxbdp );
}

static void
send_csw_ack( void )
{
	usb_unset_in_handler( 1 );
	usb_set_out_handler( 1, receive_cbw );
}

static void
reset_csw( void )
{
	csw.dSignature = MSD_CSW_Signature;
	csw.dTag = cbw.dTag;
	csw.dDataResidue = 0UL;
	csw.bStatus = MSD_CSW_CMD_PASSED;
}

