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
*/

#ifndef __USB_STACK_H__
#define __USB_STACK_H__

#include <stdint.h>

/* TODO: Move into family specific .h file */
#define USB_TOKEN_Mask	0b00111100
#define USB_TOKEN_OUT	0b00000100
#define USB_TOKEN_IN	0b00100100
#define USB_TOKEN_SOF	0b00010100
#define USB_TOKEN_SETUP	0b00110100
#define USB_TOKEN_DATA0	0b00001100
#define USB_TOKEN_DATA1	0b00101100
#define USB_TOKEN_DATA2	0b00011100		/* High speed isochronous endpoints only */
#define USB_TOKEN_MDATA	0b00111100		/* High speed isochronous enpoints and hub devices only */
#define USB_TOKEN_ACK	0b00001000
#define USB_TOKEN_NAK	0b00101000
#define USB_TOKEN_STALL	0b00111000
#define USB_TOKEN_NYET	0b00011000		/* High speed devices only */
#define USB_TOKEN_PRE	0b00110000
#define USB_TOKEN_ERR	0b00110000
#define USB_TOKEN_SPLIT	0b00100000		/* Hub devices only */
#define USB_TOKEN_PING	0b00010000		/* High speed devices only */

/* Descriptor Types */
#define USB_DEVICE_DESCRIPTOR_TYPE					 					1u
#define USB_CONFIGURATION_DESCRIPTOR_TYPE			 				2u
#define USB_STRING_DESCRIPTOR_TYPE					 					3u
#define USB_INTERFACE_DESCRIPTOR_TYPE				 					4u
#define USB_ENDPOINT_DESCRIPTOR_TYPE				 					5u
#define USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE		 			6u
#define USB_OTHER_SPEED_CONFIGURATION_DESCRIPTOR_TYPE	7u
#define USB_INTERFACE_POWER_DESCRIPTOR_TYPE			 			8u
#define USB_OTG_DESCRIPTOR_TYPE						 						9u
#define USB_DEBUG_DESCRIPTOR_TYPE											10u
#define USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE			11u
#define USB_HID_DESCRIPTOR_TYPE												0x21
#define USB_HID_REPORT_DESCRIPTOR_TYPE								0x22

#define USB_bmRequestType			0
#define USB_bRequest					1
#define USB_wValue						2
#define USB_bDescriptorIndex	2
#define USB_wValueHigh				3
#define USB_bDescriptorType		3
#define	USB_wIndex						4
#define USB_bInterface				4
#define USB_wIndexHigh				5
#define USB_wLength						6
#define	USB_wLengthHigh				7
#define USB_bData							8

#define USB_bmRequestType_PhaseMask			0b10000000
#define USB_bmRequestType_H2D						0b00000000
#define USB_bmRequestType_D2H						0b10000000
#define USB_bmRequestType_TypeMask			0b01100000
#define USB_bmRequestType_Standard			0b00000000
#define USB_bmRequestType_Class					0b00100000
#define USB_bmRequestType_Vendor				0b01000000
#define USB_bmRequestType_RecipientMask	0b00000011
#define USB_bmRequestType_Device				0b00000000
#define USB_bmRequestType_Interface			0b00000001
#define USB_bmRequestType_Endpoint			0b00000010
#define USB_bmRequestType_Other					0b00000011

struct usb_descriptor_st {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
};

struct usb_device_descriptor_st {
  uint8_t  bLength;               /* Size of this descriptor in bytes */
  uint8_t  bDescriptorType;       /* DEVICE descriptor type */
  uint16_t bcdUSB;                /* Binay Coded Decimal Spec. release */
  uint8_t  bDeviceClass;          /* Class */
  uint8_t  bDeviceSubClass;       /* Subclass */
  uint8_t  bDeviceProtocol;       /* Protocol */
  uint8_t  bMaxPacketSize0;       /* Max packet size for Endpoint 0 */
  uint16_t idVendor;              /* Vendor ID */
  uint16_t idProduct;             /* Product ID */
  uint16_t bcdDevice;             /* Device release number */
  uint8_t  iManufacturer;         /* Index of manu. string descriptor */
  uint8_t  iProduct;              /* Index of prod. string descriptor */
  uint8_t  iSerialNumber;         /* Index of S.N.  string descriptor */
  uint8_t  bNumConfigurations;    /* Number of configurations */
};

struct usb_configuration_descriptor_st {
  uint8_t  bLength;               /* size of this descriptor in bytes */
  uint8_t  bDescriptorType;       /* CONFIGURATION descriptor type */
  uint16_t wTotalLength;          /* total length of configuration */
  uint8_t  bNumInterfaces;        /* number of interfaces in this configuration */
  uint8_t  bConfigurationValue;   /* Configuration Index */
  uint8_t  iConfiguration;        /* Index of string descriptor */
  uint8_t  bmAttibutes;           /* */
  uint8_t  MaxPower;              /* maximum power consumption (x2) */
};

struct usb_interface_descriptor_st {
  uint8_t  bLength;               /* size of this descriptor in bytes */
  uint8_t  bDescriptorType;       /* INTERFACE descriptor type */
  uint8_t  bInterfaceNumber;      /* Interface Index */
  uint8_t  bAlternateSetting;     /* */
  uint8_t  bNumEndpoints;         /* Number of Endpoints */
  uint8_t  bInterfaceClass;       /* Class */
  uint8_t  bInterfaceSubClass;    /* Subclass */
  uint8_t  bInterfaceProtocol;    /* Protocol */
  uint8_t  iInterface;            /* Index of string descriptor */
};

struct usb_endpoint_descriptor_st {
  uint8_t  bLength;               /* Size of this descriptor in bytes */
  uint8_t  bDescriptorType;       /* ENDPOINT descriptor type */
  uint8_t  bEndpointAddress;      /* Address of the endpoint */
  uint8_t  bmAttributes;          /* Endpoint's attributes */
  uint16_t wMaxPacketSize;        /* Maximum packet size for this EP */
  uint8_t  bInterval;             /* Interval for polling EP in ms */
};

struct usb_hid_descriptor_st { 
  uint8_t  bLength;               /* Size of this descriptor in bytes */
  uint8_t  bDescriptorType;       /* HID descriptor type */
  uint16_t bcdHID;                /* Binay Coded Decimal Spec. release */
	uint8_t  bCountryCode;          /* Hardware target country */
  uint8_t  bNumDescriptors;       /* Number of HID class descriptors to follow */
  uint8_t  bRDescriptorType;      /* Report descriptor type */
  uint16_t wDescriptorLength;     /* Total length of Report descriptor */
};

struct usb_string_descriptor_st {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t string[];
};

struct usb_langid_string_descriptor_st {
	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint16_t langid;
};

#define USB_STR_DESC_LENGTH(...)  (sizeof((uint16_t[]){0, ##__VA_ARGS__})/sizeof(uint16_t)-1)
#define USB_STRING_DESCRIPTOR(name,...)				\
struct {							\
	uint8_t  bLength;					\
	uint8_t  bDescriptorType;				\
	uint16_t string[USB_STR_DESC_LENGTH(__VA_ARGS__)];	\
} name = {							\
	2*USB_STR_DESC_LENGTH(__VA_ARGS__)+2,			\
	USB_STRING_DESCRIPTOR_TYPE,				\
	{ __VA_ARGS__ }						\
}

/* struct usb_descriptor_collection_st {
 * 	const struct usb_device_descriptor		*device;
 * 	const struct usb_configuration_descriptor	*configuration;
 * 	const struct usb_string_descriptor 		**strings;
 * 	const int																	num_strings;
 * };
 */

#define USB_REQUEST_GET_STATUS		0
#define USB_REQUEST_CLEAR_FEATURE	1
#define USB_REQUEST_SET_FEATURE		3
#define USB_REQUEST_SET_ADDRESS		5
#define USB_REQUEST_GET_DESCRIPTOR	6
#define USB_REQUEST_SET_DESCRIPTOR	7
#define USB_REQUEST_GET_CONFIGURATION	8
#define USB_REQUEST_SET_CONFIGURATION	9
#define USB_REQUEST_GET_INTERFACE	10
#define USB_REQUEST_SET_INTERFACE	11
#define USB_REQUEST_SYNCH_FRAME		12

#include "usb_lang.h"

/* Include user configuration */
#include "usb_config.h"

#ifndef USB_VID
#error "Must supply own Vendor ID (USB_VID) in usb_config.h"
#endif
#ifndef USB_PID
#error "Must supply Product ID (USB_PID) in usb_config.h"
#endif

#if defined(PIC_18F)
#include "usb_p18f.h"
#elif defined(PIC_24F)
#include "usb_p24f.h"
#else
#error "No processor defined"
#endif


typedef void (*usb_handler_t)( void );
typedef int16_t (*usb_datahandler_t)( void *context, uint16_t length, uint8_t *packet );


/*-----------------------------------------------------------------------------
 *  usb_datahandler is a callback on endpoint data.
 *
 *  In case of an OUT token it is called whenever new data is available (length 
 *  bytes @ *packet) and keeps beeing called until returning -1 ( or the
 *  unregister_out_handler(#) is called ).
 *  If returning ZERO the data is immediatly ack'ed.
 *
 *  In case of an IN token it is called efter the current buffer is sent and user
 *  is free to place new data in buffer (*packet with at most length bytes),
 *  number of bytes placed is indicated by return value. If ZERO a ZLP is sent.
 *  If -1 the callback is unregistred.
 *
 *  context is a user supplied pointer providing context.
 *-----------------------------------------------------------------------------*/

/* Structs for defining endpoints */
typedef struct USB_EP_TYPE {
	usb_uep_t type;
	unsigned in_DTS:1;
	unsigned out_DTS:1;
	unsigned in_PP:1;
	unsigned out_PP:1;
	unsigned int buffer_size;
#if USB_PP_BUF_MODE == 0 || USB_PP_BUF_MODE == 1
	unsigned char *in_buffer, *out_buffer;
#elif USB_PP_BUF_MODE == 2 || USB_PP_BUF_MODE == 3
	unsigned char *in_buffer_even, *in_buffer_odd;
	unsigned char *out_buffer_even, *out_buffer_odd;
#endif
	usb_datahandler_t in_handler, out_handler;
	void *in_context,  *out_context;
} usb_ep_t;

/* Misc */
#define HIGHB(x) ((x)>>8)
#define LOWB(x) ((x) & 0xFF)

#define XCAT(x,y) x ## y
#define CAT(x,y) XCAT(x,y)

/* Descriptors */
#if USB_NUM_CONFIGURATIONS > 1
#error "More than 1 configuration not supported yet"
#endif

/* Interfaces */
#ifndef USB_NUM_INTERFACES
#error "Number of interfaces not defined"
#endif

extern BDentry usb_bdt[];

#ifndef USB_EP0_BUFFER_SIZE
#define USB_EP0_BUFFER_SIZE 8u
#endif

#ifndef USB_MAX_BUFFER_SIZE
#define USB_MAX_BUFFER_SIZE 8u
#elif USB_MAX_BUFFER_SIZE == 8u
#elif USB_MAX_BUFFER_SIZE == 16u
#elif USB_MAX_BUFFER_SIZE == 32u
#elif USB_MAX_BUFFER_SIZE == 64u
#else
#error "USB_MAX_BUFFER_SIZE needs to be 8, 16, 32 or 64 bytes"
#endif

typedef struct USB_DEVICE_REQUEST {
	unsigned char bmRequestType;
	unsigned char bRequest;
	unsigned int wValue;
	unsigned int wIndex;
	unsigned int wLength;
} usb_device_request_t;

extern usb_status_t trn_status;

extern BDentry *bdp, *rbdp;

void
usb_init ( ROM const struct usb_device_descriptor_st *device_descriptor,
					 ROM const struct usb_configuration_descriptor_st *configuration_descriptor,
					 ROM const struct usb_string_descriptor_st **string_descriptors,
					 int num_strings );

void usb_start( void );

void usb_stop( void );

usb_handler_t usb_register_sof_handler( usb_handler_t handler );
usb_handler_t usb_register_reset_handler( usb_handler_t handler );
usb_handler_t usb_register_class_setup_handler ( uint8_t interface_num, usb_datahandler_t handler, void *context );
usb_handler_t usb_register_vendor_setup_handler ( usb_datahandler_t handler, void *context );

#if USB_PP_BUF_MODE == 0 || USB_PP_BUF_MODE == 1
void usb_register_endpoint( unsigned int endpoint, usb_uep_t type,
				unsigned int buffer_size,
				unsigned char *out_buffer,
				unsigned char *in_buffer,
				usb_datahandler_t out_handler, void *out_context,
				usb_datahandler_t in_handler, void *in_context );
#elif USB_PP_BUF_MODE == 2 || USB_PP_BUF_MODE == 3
void usb_register_endpoint( unsigned int endpoint, usb_uep_t type,
				unsigned int buffer_size,
				unsigned char *out_buffer_even, unsigned char *out_buffer_odd,
				unsigned char *in_buffer_even, unsigned char in_buffer_odd,
				usb_datahandler_t out_handler, void *out_context,
				usb_datahandler_t in_handler, void *in_context );
#endif

void usb_set_in_handler( int ep, usb_datahandler_t handler, void *context );
void usb_set_out_handler( int ep, usb_datahandler_t handler, void *context );

#define usb_unset_in_handler(ep) usb_set_in_handler(ep, (usb_datahandler_t) 0, (void *) 0)
#define usb_unset_out_handler(ep) usb_set_out_handler(ep, (usb_datahandler_t) 0, (void *) 0)

void usb_handler ( void );

//void usb_ack ( BDentry * );

//void usb_ack_zero ( BDentry * );

void usb_ack_out ( BDentry * );		// JTR puts this in the attic

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  usb_send
 *  Description:  Prepares length bytes pointed to by *data for sending on endpoint ep.
 *                Returns the number of bytes actually sent or negative if anything
 *                negative happened, like no IN stream for indicated endpoint or that
 *                the endpoint is currently occupied with previous commitments.
 *                When data is sent the in_handler is called.
 * =====================================================================================
 */
int16_t usb_send( uint8_t ep, uint16_t length, const uint8_t *data,  usb_datahandler_t in_handler, void *context );

void usb_send_descriptor( ROM const uint8_t *descriptor, int length );

void usb_RequestError( void );

#endif /* USB_STACK_H */
