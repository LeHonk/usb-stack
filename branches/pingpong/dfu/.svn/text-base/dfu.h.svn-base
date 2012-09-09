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

#ifndef __DFU_H__
#define __DFU_H__

#include "usb_stack.h"

#define DFU_PAGE_SIZE			2048

#if DFU_PAGE_SIZE == 8
#define DFU_XFER_SIZE 8u
#elif DFU_PAGE_SIZE == 16
#define DFU_XFER_SIZE 16u
#elif DFU_PAGE_SIZE == 32
#define DFU_XFER_SIZE 32u
#elif DFU_PAGE_SIZE == 64
#define DFU_XFER_SIZE 64u
#else
#define DFU_XFER_SIZE 64u
#endif

#define DFU_bitWillDetach		0x08
#define DFU_bitManifestTolerant	0x04
#define DFU_bitCanUpload		0x02
#define	DFU_bitCanDnload		0x01

#ifndef DFU_INTERFACE_NUMBER
#error	"Must define runtime DFU_INTERFACE_NUMBER"
#endif

#ifndef DFU_INTERFACE_STRING_NUMBER
/* No string number defined - set it to zero to indicate non-existant */
#define DFU_INTERFACE_STRING_NUMBER 0x01
#endif

#define DFU_INTERFACE_STRING	USB_STRING_DESCRIPTOR('H','o','n','k','e','n',' ','D','F','U',' ','b','o','o','t','l','o','a','d','e','r')

#define DFU_INTERFACE_DESC		{ sizeof(struct usb_interface_descriptor_st)	/* bLength */							\
															, USB_INTERFACE_DESCRIPTOR_TYPE			 					/* bDescriptorType */			\
															, DFU_INTERFACE_NUMBER												/* bInterfaceNumber */		\
															, 0x00																				/* bAlternateSetting */		\
															, 0x00																				/* bNumEndpoints */				\
															, 0xFE																				/* bInterfaceClass */			\
															, 0x01																				/* bInterfaceSubclass */	\
															, 0x01																				/* bInterfaceProtocol */	\
															, DFU_INTERFACE_STRING_NUMBER									/* iInterface */					\
															}

struct dfu_functional_descriptor_st {
	uint8_t		bLength;
	uint8_t		bDescriptorType;
	uint8_t		bmAttributes;
	uint16_t	wDetachTimeout;
	uint16_t	wTransferSize;
	uint16_t	bcdDFUVersion;
};

#define DFU_FUNCTIONAL_DESC		{ sizeof(struct dfu_functional_descriptor_st) /* bLength */							\
															, 0x21																				/* bDescriptorType */			\
															, 	DFU_bitWillDetach |																									\
																	DFU_bitManifestTolerant |																						\
																	DFU_bitCanUpload |																									\
																	DFU_bitCanDnload													/* bmAttributes */				\
															, 0x0080																			/* wDetachTimeOut (ms) */	\
															, DFU_XFER_SIZE																/* wTransferSize */				\
															, 0x0110																			/* bcdDFUVersion */				\
															}

#define DFU_RUNTIME_DESC		DFU_INTERFACE_DESC, DFU_FUNCTIONAL_DESC

//extern void dfu_runtime_init( uint8_t interface_num );

#endif //__DFU_H__
