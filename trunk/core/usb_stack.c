/* 
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

Influence and inspiration taken from http://pe.ece.olin.edu/ece/projects.html
*/

#include <string.h>

#include "usb_stack.h"

//#undef DTSEN
//#define DTSEN 0

ROM const struct usb_device_descriptor_st        *device_descriptor;
ROM const struct usb_configuration_descriptor_st *config_descriptor;
ROM const struct usb_string_descriptor_st       **string_descriptors;
unsigned int                                      num_string_descriptors;

usb_handler_t reset_handler;
usb_handler_t sof_handler;
usb_handler_t class_setup_handler[USB_NUM_INTERFACES];
usb_handler_t vendor_setup_handler;
usb_ep_t      endpoints[USB_ARCH_NUM_EP];

/* Allocate buffers for buffer description table and the actual buffers */
#pragma udata usb_bdt
BDentry   usb_bdt[2 * USB_ARCH_NUM_EP]
#if defined(PIC_24F)
	__attribute__(( aligned( 512 ) ))
#endif
	;					// TODO: Make USB_ARCH_ALIGN(x) macro to beutify this

#pragma udata usb_data
/* Only claim buffer for ep 0 */
#if USB_PP_BUF_MODE == 0
unsigned char usb_ep0_out_buf[USB_EP0_BUFFER_SIZE];
unsigned char usb_ep0_in_buf[USB_EP0_BUFFER_SIZE];
#else
#error "Ping pong buffer not implemented yet!"
#endif
#pragma udata

// TODO: create an enum for different enumeration (no pun intended) states
// TODO: clarify what is kept in these variables
unsigned int usb_device_status;
unsigned int usb_configured;
unsigned char usb_addr_pending;									// The usb address the host wants this device to answer to.
																								// To be set right after transaction

usb_status_t trn_status;                        // Global since it is needed everywere
BDentry  *bdp, *rbdp;                           // Dito
ROM const uint8_t *desc_ptr;                    // These also
int	desc_len;

/* Forward Reference Prototypes */
void      usb_handle_error ( void );
void      usb_handle_reset ( void );
void      usb_handle_transaction ( void );
void      usb_handle_setup ( void );
void      usb_handle_out ( void );
void      usb_handle_in ( void );
void      usb_handle_StandardDeviceRequest ( BDentry * );
void      usb_handle_StandardInterfaceRequest ( BDentry * );
void      usb_handle_StandardEndpointRequest ( BDentry * );
//void usb_handle_ClassRequest( void );
//void usb_handle_VendorRequest( void );
void      usb_RequestError ( void );
void      usb_set_address ( void );

static void send_descriptor( void );

void
usb_init ( ROM const struct usb_device_descriptor_st *device,
		       ROM const struct usb_configuration_descriptor_st *config,
					 ROM const struct usb_string_descriptor_st **strings,
					 int num_strings )
{
	int       i;

	DPRINTF( "USB init " );

	// TODO: If self powered check VUSB to see if host connected, possibly also check USBID for OTG
	device_descriptor = device;
	config_descriptor = config;
	string_descriptors = strings;
	num_string_descriptors = num_strings;

	ResetPPbuffers();

	DisableAllUsbInterrupts();
  EnableAllUsbErrorInterrupts();                // Enable all errors to set flag in UIR

	ClearAllUsbErrorInterruptFlags();
	ClearAllUsbInterruptFlags();

	ConfigureUsbHardware();

	reset_handler = usb_handle_reset;
	sof_handler = NULL;
	vendor_setup_handler = NULL;

	for ( i = 0; i < USB_NUM_INTERFACES; i++ ) {
		class_setup_handler[i] = NULL;
	}

	for ( i = 0; i < USB_ARCH_NUM_EP; i++ ) {
		endpoints[i].type = 0;
		endpoints[i].buffer_size = 0;
		endpoints[i].out_buffer = NULL;
		endpoints[i].in_buffer = NULL;
		endpoints[i].out_handler = NULL;
		endpoints[i].in_handler = NULL;
	}

	usb_register_endpoint( 0, USB_EP_CONTROL, USB_EP0_BUFFER_SIZE, usb_ep0_out_buf, usb_ep0_in_buf, NULL, NULL );	// Register ep0 - no handlers

	/* Configure endpoints */
	for ( i = 0; i < USB_ARCH_NUM_EP; i++ ) {
		/* Configure endpoints (USB_UEPn - registers) */
		USB_UEP[i] = endpoints[i].type;
		/* Configure buffer descriptors */
#if USB_PP_BUF_MODE == 0
		usb_bdt[USB_CALC_BD( i, USB_DIR_OUT, USB_PP_EVEN )].BDCNT = endpoints[i].buffer_size;
		usb_bdt[USB_CALC_BD( i, USB_DIR_OUT, USB_PP_EVEN )].BDADDR = endpoints[i].out_buffer;
		usb_bdt[USB_CALC_BD( i, USB_DIR_OUT, USB_PP_EVEN )].BDSTAT = UOWN + DTSEN;
		usb_bdt[USB_CALC_BD( i, USB_DIR_IN, USB_PP_EVEN )].BDCNT = 0;
		usb_bdt[USB_CALC_BD( i, USB_DIR_IN, USB_PP_EVEN )].BDADDR = endpoints[i].in_buffer;
		usb_bdt[USB_CALC_BD( i, USB_DIR_IN, USB_PP_EVEN )].BDSTAT = DTS;	// Set DTS => First packet inverts, ie. is Data0
#else
		// TODO: Implement Ping-Pong buffering setup.
#error "PP Mode not implemented yet"
#endif
	}

	/* TODO: this isn't actually 100% correct. "usb_device_status" is a runtime variable
	   In the case of a bus powered device it will always be 0x000 but in the case of
	   self powered with bus powered option it becames variable to the current powered
	   State. This is a minor thing and for now but it may need to be addressed if there is
	   any hardware that is dual powered. --JTR */
#ifdef USB_SELF_POWERED
	usb_device_status = 0x0001;
#else
	usb_device_status = 0x0000;
#endif
	usb_configured = 0x00;
	usb_addr_pending = 0x00;
}

void
usb_start( void )
{
	DPRINTF( "Starting usb " );
  EnableUsb();                                  // Enable USB-hardware
  while ( SingleEndedZeroIsSet() );             // Busywait for initial power-up
	DPRINTF( "sucessful\n" );
	// JTR USB_ACTIV should be enabled in the SUSPEND mode handler.
	EnableUsbInterrupt( USB_STALL + USB_IDLE + USB_TRN + USB_SOF + USB_UERR + USB_URST );
}

void
usb_stop( void )
{
	DPRINTF( "Stopping usb " );
	DisableUsbInterrupts();
	DisableUsb();
	DPRINTF( "detached\n" );
}

void
usb_handler( void )
{
	if ( USB_ERROR_FLAG ) {
		usb_handle_error();
		ClearUsbInterruptFlag( USB_UERR );
	} else if ( USB_RESET_FLAG ) {
		reset_handler();
		ClearUsbInterruptFlag( USB_URST );
	} else if ( USB_ACTIVITY_FLAG ) {
		/* Activity - unsuspend */
		WakeupUsb();
		ClearUsbInterruptFlag( USB_ACTIV );
    DisableUsbInterrupt( USB_ACTIV );           // Disable usb activity interrupt, following JTR's suggestion
		DPRINTF( "Active\n" );
	} else if ( USB_IDLE_FLAG ) {
		/* Idle - suspend */
		SuspendUsb();
		usb_low_power_request();
		ClearUsbInterruptFlag( USB_IDLE );
    EnableUsbInterrupt( USB_ACTIV );            // Enable usb activity interrupt, per JTR's suggestion
		DPRINTF( "Idle\n" );
	} else if ( USB_STALL_FLAG ) {
		/* Stall detected
		 * Not sure when this interrupt fires
		 * as the host never should send out a stall.
		 * Perhaps as a respons to our own stalls? 
		 * For now just ignore it. */
		ClearUsbInterruptFlag( USB_STALL );
		DPRINTF( "Stall interrupt fired" );
	} else if ( USB_SOF_FLAG ) {
		/* Start-of-frame */
		if ( sof_handler )
			sof_handler();
		ClearUsbInterruptFlag( USB_SOF );
	} else if ( USB_TRANSACTION_FLAG ) {
		usb_handle_transaction();
		ClearUsbInterruptFlag( USB_TRN );	// Side effect: advance USTAT Fifo
	}
}

void
usb_handle_error ( void )
{
	DPRINTF( "Error 0x%X\n", UsbErrorInterruptFlags() );
	/* No errorhandler for now, just clear offending flag */
	ClearAllUsbErrorInterruptFlags();
}

void
usb_handle_reset( void )
{

	int       i;

	// TODO: Handle as in usb_init()
#ifdef USB_SELF_POWERED
	usb_device_status = 0x0001;
#else
	usb_device_status = 0x0000;
#endif
	usb_configured = 0x00;
	usb_addr_pending = 0x00;

	for ( i = 0; USB_TRANSACTION_FLAG || i < USB_USTAT_FIFO_DEPTH; i++ ) {	// Must poll TRN Flag and clear, then wait 6 cycles. for next flag set. --JTR
    ClearUsbInterruptFlag( USB_TRN );           // Advance USTAT FIFO to clear
		Nop();																			// Delay 6 cycles
		Nop();
		Nop();
		Nop();
		Nop();
		Nop();
	}

	for ( i = 0; i < USB_ARCH_NUM_EP; i++ )
    USB_UEP[i] = 0;                             // Disable all endpoints

  SetUsbAddress( 0 );                           // After reset we don't have an address
	ClearAllUsbInterruptFlags();
	ClearAllUsbErrorInterruptFlags();

	/* (Re)Configure endpoints (USB_UEPn - registers) */
	for ( i = 0; i < USB_ARCH_NUM_EP; i++ ) {
		USB_UEP[i] = endpoints[i].type;
		/* Configure buffer descriptors */
#if USB_PP_BUF_MODE == 0
		usb_bdt[USB_CALC_BD( i, USB_DIR_OUT, USB_PP_EVEN )].BDCNT = endpoints[i].buffer_size;
		usb_bdt[USB_CALC_BD( i, USB_DIR_OUT, USB_PP_EVEN )].BDADDR = endpoints[i].out_buffer;
		usb_bdt[USB_CALC_BD( i, USB_DIR_OUT, USB_PP_EVEN )].BDSTAT = UOWN + DTSEN;
		usb_bdt[USB_CALC_BD( i, USB_DIR_IN, USB_PP_EVEN )].BDCNT = 0;
		usb_bdt[USB_CALC_BD( i, USB_DIR_IN, USB_PP_EVEN )].BDADDR = endpoints[i].in_buffer;
		usb_bdt[USB_CALC_BD( i, USB_DIR_IN, USB_PP_EVEN )].BDSTAT = DTS;	// Set DTS => First packet inverts, ie. is Data0
#else
		// TODO: Implement Ping-Pong buffering setup.
#error "PP Mode not implemented yet"
#endif
	}
	EnableAllUsbErrorInterrupts();
	DPRINTF( "Reset\n" );
}

void
usb_handle_transaction( void )
{

	trn_status = GetUsbTransaction (  );
	bdp = &usb_bdt[USB_USTAT2BD( trn_status )];
	rbdp = &usb_bdt[USB_CALC_BD( USB_USTAT2EP( trn_status ),
                               USB_DIR_IN,      // All replies in IN direction
				      								 USB_PP_EVEN )];	// TODO: Implement Ping-Pong buffering

//      DPRINTF("USTAT: 0x%02X PID 0x%02X DATA%c ", trn_status, bdp->BDSTAT % USB_TOKEN_Mask, (bdp->BDSTAT & 0x40)?'1':'0');
	switch ( bdp->BDSTAT & USB_TOKEN_Mask ) {
	case USB_TOKEN_SETUP:
		usb_handle_setup();
		break;
	case USB_TOKEN_OUT:
		usb_handle_out();
		break;
	case USB_TOKEN_IN:
		usb_handle_in();
		break;
	default:
		/* Default case of unknown TOKEN - discard */
		DPRINTF( "Unknown token\n" );
	}
}

void
usb_handle_setup( void )
{
	DPRINTF ( "bmReqType 0x%02X ", bdp->BDADDR[USB_bmRequestType] );
	rbdp->BDSTAT = DTSEN;			// Reclaim reply buffer
	switch ( bdp->BDADDR[USB_bmRequestType] & USB_bmRequestType_TypeMask ) {
	case USB_bmRequestType_Standard:
		switch ( bdp->BDADDR[USB_bmRequestType] & USB_bmRequestType_RecipientMask ) {
		case USB_bmRequestType_Device:
			usb_handle_StandardDeviceRequest( bdp );
			break;
		case USB_bmRequestType_Interface:
			usb_handle_StandardInterfaceRequest( bdp );
			break;
		case USB_bmRequestType_Endpoint:
			usb_handle_StandardEndpointRequest( bdp );
			break;
		default:
			usb_RequestError();
		}
		break;
	case USB_bmRequestType_Class:
		if ( USB_NUM_INTERFACES > bdp->BDADDR[USB_bInterface]
		     && class_setup_handler[bdp->BDADDR[USB_bInterface]] )
			class_setup_handler[bdp->BDADDR[USB_bInterface]]();
		else
			usb_RequestError();
		break;
	case USB_bmRequestType_Vendor:
		if ( vendor_setup_handler )
			vendor_setup_handler();
		else
			usb_RequestError();
		break;
	default:
		usb_RequestError();
	}
	/* Prepare endpoint for new reception */
	bdp->BDCNT = endpoints[USB_USTAT2EP( trn_status )].buffer_size;
	// TODO: Better DataToggleSyncronization algorithm, according to JTR.
	// Suggestion to look at MC stack is probably bad as this stack is Microchip USB stack agnostic
	bdp->BDSTAT =
		( !
		  ( bdp->
		    BDADDR[USB_bmRequestType] & USB_bmRequestType_PhaseMask )
&& ( bdp->BDADDR[USB_wLength]
     || bdp->BDADDR[USB_wLengthHigh] ) ) ? UOWN + DTS + DTSEN : UOWN + DTSEN;
	// JTR Note. that CONTROL IN and OUT DATA packet transfers do not come back here and there is no
	// univesal way and place of setting up EP0 after these DATA transfers in this stack.
	// JTR Note. that there is a PIC18 silicon errata issue that this does not address by being here.  See DS80220F-page 6
	EnablePacketTransfer();
}

void
usb_handle_StandardDeviceRequest( BDentry * bdp )
{
	usb_device_request_t *packet = (usb_device_request_t *) bdp->BDADDR;
	unsigned int i;

	switch ( packet->bRequest ) {
	case USB_REQUEST_GET_STATUS:
		rbdp->BDADDR[0] = usb_device_status & 0xFF;
		rbdp->BDADDR[1] = usb_device_status >> 8;
		rbdp->BDCNT = 2;
		rbdp->BDSTAT = UOWN + DTS + DTSEN;
		DPRINTF( "DEV Get_Status\n" );
		break;
	case USB_REQUEST_CLEAR_FEATURE:
		if ( 0x0001u == packet->wValue ) {	// TODO: Remove magic (REMOTE_WAKEUP_FEATURE)
			usb_device_status &= ~0x0002;
			rbdp->BDCNT = 0;
			rbdp->BDSTAT = UOWN + DTS + DTSEN;
			DPRINTF( "DEV Clear_Feature 0x%04X\n", packet->wValue );
		} else
			usb_RequestError();
		break;
	case USB_REQUEST_SET_FEATURE:
		if ( 0x0001u == packet->wValue ) {	// TODO: Remove magic (REMOTE_WAKEUP_FEATURE)
			usb_device_status |= 0x0002;
			rbdp->BDCNT = 0;
			rbdp->BDSTAT = UOWN + DTS + DTSEN;
			DPRINTF( "DEV Set_Feature 0x%04X\n", packet->wValue );
		} else
			usb_RequestError();
		break;
	case USB_REQUEST_SET_ADDRESS:
		if ( 0x007Fu >= packet->wValue ) {
			usb_addr_pending = (uint8_t) packet->wValue;
			rbdp->BDCNT = 0;
			rbdp->BDSTAT = UOWN + DTS + DTSEN;
			usb_set_in_handler( 0, usb_set_address );
			DPRINTF( "DEV Set address 0x%02X\n", usb_addr_pending );
		} else
			usb_RequestError();
		break;
	case USB_REQUEST_GET_DESCRIPTOR:
		switch ( HIGHB(packet->wValue) ) {
		case USB_DEVICE_DESCRIPTOR_TYPE:
			desc_ptr = (ROM const uint8_t *) device_descriptor;
			desc_len = device_descriptor->bLength;
			DPRINTF( "DEV Dev_desc_req " );
			break;
		case USB_CONFIGURATION_DESCRIPTOR_TYPE:
			if ( LOWB(packet->wValue) >= device_descriptor->bNumConfigurations )
				usb_RequestError();
			desc_ptr = (ROM const uint8_t *) config_descriptor;
			desc_len = config_descriptor->wTotalLength;
			for ( i = 0; i < LOWB(packet->wValue); i++ ) {	// Implicit linked list traversal until requested configuration
				desc_ptr += desc_len;
				desc_len = ((struct usb_configuration_descriptor_st *) desc_ptr)->wTotalLength;
			}
			DPRINTF( "DEV Conf_desc_req 0x%02X 0x%04X\n", LOWB(packet->wValue), packet->wLength );
			break;
		case USB_STRING_DESCRIPTOR_TYPE:
			// TODO: Handle language request. For now return standard language.
			DPRINTF( "DEV Str_desc_req 0x%02X 0x%04X\n", LOWB(packet->wValue), packet->wLength );
			if ( LOWB(packet->wValue) >= num_string_descriptors )
				usb_RequestError();
			desc_ptr = (ROM const uint8_t *) string_descriptors[LOWB(packet->wValue)];
			desc_len = desc_ptr[0];
			break;
		case USB_INTERFACE_DESCRIPTOR_TYPE:
		case USB_ENDPOINT_DESCRIPTOR_TYPE:
		default:
			usb_RequestError();
		}
		if ( packet->wLength < desc_len )
			desc_len = packet->wLength;
    send_descriptor();                      // Send first part of packet right away
		usb_set_in_handler( 0, send_descriptor );
		break;
	case USB_REQUEST_GET_CONFIGURATION:
		rbdp->BDADDR[0] = usb_configured;
		rbdp->BDCNT = 1;
		rbdp->BDSTAT = UOWN + DTS + DTSEN;
		DPRINTF ( "DEV Get_Config\n" );
		break;
	case USB_REQUEST_SET_CONFIGURATION:
		if ( USB_NUM_CONFIGURATIONS >= LOWB(packet->wValue) ) {
			// TODO: Support multiple configurations by user callback void(*cfg_handler_t)(unsigned char), per JTR suggestion
			/* That'll also be a good place for user function to set (register) endpoint handlers.
			   Ought to be a nice implementation... /Honken */
			/* Configure endpoints (USB_UEPn - registers) */
			for ( i = 0; i < USB_ARCH_NUM_EP; i++ ) {
				USB_UEP[i] = endpoints[i].type;
				/* Configure buffer descriptors */
#if USB_PP_BUF_MODE == 0
				usb_bdt[USB_CALC_BD( i, USB_DIR_OUT, USB_PP_EVEN )].BDCNT = endpoints[i].buffer_size;
				usb_bdt[USB_CALC_BD( i, USB_DIR_OUT, USB_PP_EVEN )].BDSTAT = UOWN + DTSEN;
				usb_bdt[USB_CALC_BD( i, USB_DIR_IN, USB_PP_EVEN )].BDCNT = 0;
				usb_bdt[USB_CALC_BD( i, USB_DIR_IN, USB_PP_EVEN )].BDSTAT = DTS + DTSEN;	// Set DTS => First packet inverts, ie. is Data0
#else
				// TODO: Implement Ping-Pong buffering setup.
#error "PP Mode not implemented yet"
#endif
			}
			usb_configured = LOWB(packet->wValue);
			rbdp->BDCNT = 0;
			rbdp->BDSTAT = UOWN + DTS + DTSEN;
			DPRINTF( "DEV Set_Config 0x%04X\n", packet->wValue );
		} else
			usb_RequestError();
		break;
	case USB_REQUEST_SET_DESCRIPTOR:
	default:
		usb_RequestError();
	}
}

void
usb_handle_StandardInterfaceRequest( BDentry * bdp )
{
	unsigned char *packet = bdp->BDADDR;

	switch ( packet[USB_bRequest] ) {
	case USB_REQUEST_GET_STATUS:
		rbdp->BDADDR[0] = 0x00;
		rbdp->BDADDR[1] = 0x00;
		rbdp->BDCNT = 2;
		rbdp->BDSTAT = UOWN + DTS + DTSEN;
		DPRINTF( "IF Get_Status\n" );
		break;
	case USB_REQUEST_GET_INTERFACE:
		if ( USB_NUM_INTERFACES > packet[USB_bInterface] ) {
			// TODO: Implement alternative interfaces, or move responsibility to class/vendor functions.
			rbdp->BDADDR[0] = 0;
			rbdp->BDCNT = 1;
			rbdp->BDSTAT = UOWN + DTS + DTSEN;
			DPRINTF( "IF Get_Interface\n" );
		} else
			usb_RequestError();
		break;
	case USB_REQUEST_SET_INTERFACE:
		if ( USB_NUM_INTERFACES > packet[USB_bInterface] && 0u == packet[USB_wValue] ) {
			// TODO: Implement alternative interfaces...
			rbdp->BDCNT = 0;
			rbdp->BDSTAT = UOWN + DTS + DTSEN;
			DPRINTF( "IF Set_Interface 0x%2X\n",
				  packet[USB_bInterface] );
		} else
			usb_RequestError();
		break;
	case USB_REQUEST_CLEAR_FEATURE:
	case USB_REQUEST_SET_FEATURE:
	default:
		usb_RequestError();
	}
}

void
usb_handle_StandardEndpointRequest( BDentry * bdp )
{
	unsigned char *packet;
	unsigned char epnum;
	unsigned char dir;
	BDentry  *epbd;

	packet = bdp->BDADDR;

	switch ( packet[USB_bRequest] ) {
	case USB_REQUEST_GET_STATUS:
		/* JTR this code block is (was?) not correct. It is a throw back to the 16C765
		   stack where the STALL status was B0 in the UEPx. As it is the EPSTALL
		   bit is NOT B0 on the PIC24 so this code was not correct for this family.
		   Instead we will use the STALL bits in the BD table. */
		/* Honken: So this get_status is for the host to check if feature (ENDPOINT_HALT) is set?
		   I had missunderstood it to check if a stall acctually had occured */
		epnum = packet[USB_wIndex] & 0x0F;
		dir = packet[USB_wIndex] >> 7;
		epbd = &usb_bdt[USB_CALC_BD ( epnum, dir, USB_PP_EVEN )];
		rbdp->BDADDR[0] = ( epbd->BDSTAT & BSTALL ) ? 0x01 : 0x00;
#if USB_PP_BUF_MODE > 0
		epbd = &usb_bdt[USB_CALC_BD ( epnum, dir, USB_PP_ODD )];
		rbdp->BDADDR[0] |= ( epbd->BDSTAT & BSTALL ) ? 0x01 : 0x00;
#endif
		rbdp->BDADDR[1] = 0x00;
		rbdp->BDCNT = 2;
		rbdp->BDSTAT = UOWN + DTS + DTSEN;
		DPRINTF ( "EP Get_Status\n" );
		break;
	case USB_REQUEST_CLEAR_FEATURE:
		/* JTR This block was not complete. The CLEAR ENDPOINT FEATURE
		   must also reset the data toggling. This is a real PAIN when ping-pong
		   is enabled.
		   As it is this really is an application event and there should be a 
		   call back and protocol for handling the possible lost of a data packet. */
		// TODO: Ping-Pong support.
		epnum = packet[USB_wIndex] & 0x0F;
		dir = packet[USB_wIndex] >> 7;
		epbd = &usb_bdt[USB_CALC_BD ( epnum, dir, USB_PP_EVEN )];
		epbd->BDSTAT &= ~BSTALL;
		if ( dir )
			epbd->BDSTAT |= DTS;	// JTR added IN EP set DTS as it will be toggled to zero next transfer
		else
			epbd->BDSTAT &= ~DTS;
#if USB_PP_BUF_MODE > 0
		epbd = &usb_bdt[USB_CALC_BD ( epnum, dir, USB_PP_ODD )];
		epbd->BDSTAT &= ~BSTALL;
		// TODO: Reset data toggling to correct sync
#endif
		rbdp->BDCNT = 0;
		rbdp->BDSTAT = UOWN + DTS + DTSEN;
		DPRINTF( "EP Clear_Feature 0x%02X\n", packet[USB_wIndex] );
		break;
	case USB_REQUEST_SET_FEATURE:
		epnum = packet[USB_wIndex] & 0x0F;
		dir = packet[USB_wIndex] >> 7;
		epbd = &usb_bdt[USB_CALC_BD( epnum, dir, USB_PP_EVEN )];
		epbd->BDSTAT |= BSTALL;
#if USB_PP_BUF_MODE > 0
		epbd = &usb_bdt[USB_CALC_BD( epnum, dir, USB_PP_ODD )];
		epbd->BDSTAT |= BSTALL;
#endif
		rbdp->BDCNT = 0;
		rbdp->BDSTAT = UOWN + DTS + DTSEN;
		DPRINTF( "EP Set_Feature 0x%02X\n", packet[USB_wIndex] );
		break;
	case USB_REQUEST_SYNCH_FRAME:
	default:
		usb_RequestError();
	}
}

void
usb_handle_in( void )
{
	DPRINTF("In  EP: %u Handler: 0x%p\t", trn_status >> 3, endpoints[USB_USTAT2EP(trn_status)].in_handler);
	if ( endpoints[USB_USTAT2EP( trn_status )].in_handler ) {
		endpoints[USB_USTAT2EP( trn_status )].in_handler();
	} else {
//              DPRINTF("No handler\n");
	}
}

void
usb_handle_out( void )
{
	DPRINTF("Out EP: %u Handler: 0x%p\t", trn_status >> 3, endpoints[USB_USTAT2EP(trn_status)].out_handler);
	//rbdp->BDSTAT &= ~UOWN;                                                                        // Reclaim reply buffer
	if ( endpoints[USB_USTAT2EP( trn_status )].out_handler ) {
		endpoints[USB_USTAT2EP( trn_status )].out_handler();
	} else {
//              DPRINTF("No handler\n");
	}
}

usb_handler_t
usb_register_reset_handler( usb_handler_t handler )
{
	usb_handler_t previous = reset_handler;

	if ( handler )
		reset_handler = handler;
	else
		reset_handler = &usb_handle_reset;
	return previous;
}

usb_handler_t
usb_register_sof_handler( usb_handler_t handler )
{
	usb_handler_t previous = sof_handler;

	sof_handler = handler;
	return previous;
}

usb_handler_t
usb_register_class_setup_handler( unsigned char iface, usb_handler_t handler )
{
	usb_handler_t previous = NULL;

	if ( USB_NUM_INTERFACES > iface ) {
		previous = class_setup_handler[iface];
		class_setup_handler[iface] = handler;
	}	// Else silently ignore.
	return previous;
}

usb_handler_t
usb_register_vendor_setup_handler( usb_handler_t handler )
{
	usb_handler_t previous = vendor_setup_handler;

	vendor_setup_handler = handler;
	return previous;
}

void
usb_register_endpoint( unsigned int ep, usb_uep_t type,
			unsigned int buf_size, unsigned char *out_buf, unsigned char *in_buf,
			usb_handler_t out_handler, usb_handler_t in_handler )
{
	endpoints[ep].type = type;
	endpoints[ep].buffer_size = buf_size;
	endpoints[ep].out_buffer = out_buf;
	endpoints[ep].in_buffer = in_buf;
	endpoints[ep].out_handler = out_handler;
	endpoints[ep].in_handler = in_handler;
}

void
usb_set_in_handler( int ep, usb_handler_t in_handler )
{
	endpoints[ep].in_handler = in_handler;
}

void
usb_set_out_handler( int ep, usb_handler_t out_handler )
{
	endpoints[ep].out_handler = out_handler;
}

void
usb_ack( BDentry * bd )
{
	bd->BDSTAT = ( ( bd->BDSTAT ^ DTS ) & DTS ) | UOWN | DTSEN;	// TODO: Accomodate for >=256 byte buffers
}

void
usb_ack_zero( BDentry * bd )
{
	bd->BDCNT = 0;
	bd->BDSTAT = ( ( bd->BDSTAT ^ DTS ) & DTS ) | UOWN | DTSEN;
}

/* JTR this needs to be fixed.
   If such a function is going to be used then surely the EP count needs
   to be a parameter Currently as the stack stands as a CDC stack There
   are no OUT transfers > 8 bytes so we do not need this help function. */
// TODO: Correct things according to JTR
void
usb_ack_out( BDentry * bd )
{
	bd->BDCNT = USB_MAX_BUFFER_SIZE;	// TODO: Fix correct size for current EP
	bd->BDSTAT = ( ( bd->BDSTAT ^ DTS ) & DTS ) | UOWN | DTSEN;
}

void
usb_RequestError( void )
{
	unsigned int i;

	usb_bdt[USB_CALC_BD( 0, USB_DIR_IN, USB_PP_EVEN )].BDSTAT = UOWN + BSTALL;
	usb_bdt[USB_CALC_BD( 0, USB_DIR_IN, USB_PP_ODD )].BDSTAT = UOWN + BSTALL;

	usb_bdt[USB_CALC_BD( 0, USB_DIR_OUT, USB_PP_EVEN )].BDCNT = USB_EP0_BUFFER_SIZE;
	usb_bdt[USB_CALC_BD( 0, USB_DIR_OUT, USB_PP_EVEN )].BDSTAT = UOWN + BSTALL;

	usb_bdt[USB_CALC_BD( 0, USB_DIR_OUT, USB_PP_ODD )].BDCNT = USB_EP0_BUFFER_SIZE;
	usb_bdt[USB_CALC_BD( 0, USB_DIR_OUT, USB_PP_ODD )].BDSTAT = UOWN + BSTALL;

	DPRINTF( "Request error\n" );
	for ( i = 0; i < bdp->BDCNT; i++ )
		DPRINTF( "0x%02X ", bdp->BDADDR[i] );
	DPRINTF( "\n" );
}

void
usb_set_address( void )
{
	if ( 0x80u > usb_addr_pending ) {
		SetUsbAddress( usb_addr_pending );
		usb_addr_pending = 0xFF;
		DPRINTF( "Address set 0x%02X\n", GetUsbAddress() );
	}
	usb_unset_in_handler( 0 );		// Unregister handler
}

void usb_send_descriptor( ROM const uint8_t *desc, int length) {
	desc_ptr = desc;
	desc_len = length;
	send_descriptor();

}
static void
send_descriptor( void )
{
	unsigned int i;
	BDentry  *bd;
	size_t    packet_len;

	if ( desc_len ) {
		packet_len = ( desc_len < USB_EP0_BUFFER_SIZE ) ? desc_len : USB_EP0_BUFFER_SIZE;
		bd = &usb_bdt[USB_CALC_BD ( 0, USB_DIR_IN, USB_STAT2PPI ( trn_status ) ? 0x01 : 0x00 )];
		DPRINTF( "Send bd: 0x%p dst: 0x%p src: 0x%p len: %u Data: ", rbdp, rbdp->BDADDR, desc_ptr, packet_len );
		//ARCH_memcpy(rbdp->BDADDR, usb_desc_ptr, packet_len);
		for ( i = 0; i < packet_len; i++ ) {
			rbdp->BDADDR[i] = desc_ptr[i];
			DPRINTF( "0x%02X ", rbdp->BDADDR[i] );
		}
		DPRINTF( "\n" );
	} else {
		packet_len = 0;			// Send ZLP
		usb_unset_in_handler( 0 );
		DPRINTF( "Send done\n" );
	}
	rbdp->BDCNT = (uint8_t) packet_len;
	usb_ack( rbdp );			// Packet length always less then 256 on endpoint 0
	desc_ptr += packet_len;
	desc_len -= packet_len;
}
