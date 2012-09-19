/*
 * =====================================================================================
 *
 *       Filename:  hid.c
 *
 *    Description:  Implementation of HID class
 *
 *             Id:  
 *        Created:  04.03.2012 19:16:47
 *       Revision:  none
 *
 *         Author:  Tomas Holmqvist (TH), LeHonk@users.noreply.github.com
 *   Organization:  
 *
 * =====================================================================================
 */


#include <stdint.h>
#include "usb_stack.h"
#include "descriptors.h"
#include "hid.h"

#define	HID_REQUEST_GET_REPORT			0x01			/* Mandatory */
#define	HID_REQUEST_GET_IDLE				0x02			/*  */
#define	HID_REQUEST_GET_PROTOCOL		0x03			/* Required only for boot devices */
#define	HID_REQUEST_SET_REPORT			0x09			/*  */
#define	HID_REQUEST_SET_IDLE				0x0A			/*  */
#define	HID_REQUEST_SET_PROTOCOL		0x0B			/* Required only for boot devices */

static void hid_setup( void );
static void kb_out( void );
static void kb_in( void );
static void pt_in( void );

#pragma udata usb_data
static unsigned char kb_in_buffer[8];		// TODO: Remove magic
static unsigned char kb_out_buffer[8];
static unsigned char pt_in_buffer[8];
#pragma udata

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  InitHID
 *  Description:  
 * =====================================================================================
 */
void
InitHID( void )
{
	usb_init( &hid_device_descriptor, 
						&hid_configuration.config,
						hid_str_descs, NUM_STRINGS );
	usb_register_class_setup_handler( 0, hid_setup );			// Keyborad interface
	usb_register_class_setup_handler( 1, hid_setup );			// Mouse/Pointer interface
	usb_register_endpoint(1, USB_EP_INOUT, 8, kb_out_buffer, kb_in_buffer, kb_out, kb_in);
	usb_register_endpoint(2, USB_EP_IN, 8, NULL, pt_in_buffer, NULL, pt_in);
	usb_start();
}		/* -----  end of function InitHID  ----- */

static void hid_setup( void ) {
	unsigned char *packet = bdp->BDADDR;
	unsigned char req_type = packet[USB_bmRequestType];
	unsigned char req = packet[USB_bRequest];
	DPRINTF( "HID Setup IFace %02u\n", packet[USB_bInterface] );
	switch ( packet[USB_bRequest] ) {
		case HID_REQUEST_GET_REPORT:
			usb_ack_zero(rbdp);                       /* Send ZLP TODO: Answer correctly */
			break;
		case HID_REQUEST_GET_IDLE:
			break;
		case HID_REQUEST_GET_PROTOCOL:
			break;
		case USB_REQUEST_GET_DESCRIPTOR:
			if (packet[USB_wValue]!=USB_HID_REPORT_DESCRIPTOR_TYPE) {
				usb_RequestError();
				break;
			}
			switch ( packet[USB_wValue+1] ) {
				case 0:
					usb_send_descriptor( kb_report_descriptor, sizeof(kb_report_descriptor) );
					break;
				case 1:
					usb_send_descriptor( pt_report_descriptor, sizeof(pt_report_descriptor) );
					break;
				default:
					usb_RequestError(  );
			}
			break;
		case USB_REQUEST_SET_DESCRIPTOR:
		case HID_REQUEST_SET_REPORT:
		case HID_REQUEST_SET_IDLE:
		case HID_REQUEST_SET_PROTOCOL:
		default:
			usb_RequestError();
	}
}

static void kb_out( void ) {
	DPRINTF( "Keyboard OUT\n" );
}

static void kb_in( void ) {
	DPRINTF( "Keyboard IN\n" );
}

static void pt_in( void ) {
	DPRINTF( "Mouse IN\n" );
}

