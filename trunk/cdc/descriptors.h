/*
 * 
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to
 *   Creative Commons,
 *   171 Second Street,
 *   Suite 300,
 *   San Francisco,
 *   California,
 *   94105,
 *   USA.
 */

#ifndef __DESCRIPTORS_H__
#define __DESCRIPTORS_H__

#include "usb_stack.h"
#include "cdc.h"

ROM const struct usb_device_descriptor_st cdc_device_descriptor = {
  sizeof(cdc_device_descriptor),                // bLength
  USB_DEVICE_DESCRIPTOR_TYPE,                   // bDescriptorType
  0x0200,                                       // bcdUSB
  0x02,                                         // bDeviceClass
  0x00,                                         // bDeviceSubClass
  0x00,                                         // bDeviceProtocol
  USB_EP0_BUFFER_SIZE,                          // bMaxPacketSize
  USB_VID,                                      // idVendor
  USB_PID,                                      // idProduct
  USB_DEV,                                      // bcdDevice
  USB_iManufacturer,                            // iManufacturer
  USB_iProduct,                                 // iProduct
  USB_iSerialNum,                               // iSerialNumber (none)
  USB_NUM_CONFIGURATIONS                        // bNumConfigurations 
};

//#define USB_CONFIG_DESC_TOT_LENGTH (9+9+5+4+5+5+7+9+7+7)	//+DFU_RUNTIME_DESC_LENGTH)
ROM const struct {
	struct usb_configuration_descriptor_st	config;
	struct usb_interface_descriptor_st			cdc_ctrl_iface;
	struct cdc_function_descriptor5_st			cdc_header;
	struct cdc_function_descriptor4_st			cdc_acm;
	struct cdc_function_descriptor5_st			cdc_union;
	struct cdc_function_descriptor5_st			cdc_management;
	struct usb_endpoint_descriptor_st				cdc_control;
	struct usb_interface_descriptor_st			cdc_xfer_iface;
	struct usb_endpoint_descriptor_st				cdc_bulk_out;
	struct usb_endpoint_descriptor_st				cdc_bulk_in;
} cdc_config_descriptor = {
	{
		sizeof(struct usb_configuration_descriptor_st),	// bLength
		USB_CONFIGURATION_DESCRIPTOR_TYPE,          // bDescriptorType
		sizeof(cdc_config_descriptor),              // wTotalLength
		USB_NUM_INTERFACES,                         // bNumInterfaces
		0x01,                                       // bConfigurationValue
		0x00,                                       // iConfiguration (0=none)
		0x80,                                       // bmAttributes (0x80 = bus powered)
		0x32                                        // bMaxPower (in 2 mA units, 50=100 mA)
	},{
		sizeof(struct usb_interface_descriptor_st), // bLength (Interface0 descriptor starts here)
		USB_INTERFACE_DESCRIPTOR_TYPE,              // bDescriptorType
		0x00,                                       // bInterfaceNumber
		0x00,                                       // bAlternateSetting
		0x01,                                       // bNumEndpoints (excluding EP0)
		0x02,                                       // bInterfaceClass (0x00=per endpoint specified, 0xFF=vendor specific)
		0x02,                                       // bInterfaceSubClass (0x00=per endpoint specified, 0xFF=vendor specific)
		0x00,                                       // bInterfaceProtocol (0x00=no protocol, 0xFE=as by command set, 0xFF=vendor specific)
		0x00                                        // iInterface (none)
	},{
		0x05,                                       // bFunctionLength
		0x24,                                       // bDescriptorType
		0x00,                                       // bDescriptorSubtype (CDC header descriptor)
		0x10,                                       // bcdCDC (low byte)
		0x01                                        // bcdCDC (high byte)
	},{
		0x04,                                       // bFunctionLength
		0x24,                                       // bDescriptorType
		0x02,                                       // bDescriptorSubtype (CDC abstract control management descriptor)
		0x02                                        // bmCapabilities
	},{
		0x05,                                       // bFunctionLength
		0x24,                                       // bDescriptorType
		0x06,                                       // bDescriptorSubtype (CDC union descriptor)
		0x00,                                       // bControlInterface
		0x01                                        // bSubordinateInterface0
	},{
		0x05,                                       // bFunctionLength
		0x24,                                       // bDescriptorType
		0x01,                                       // bDescriptorSubtype (Call management descriptor)
		0x01,                                       // bmCapabilities
		0x01                                        // bInterfaceNum
	},{
		sizeof(struct usb_endpoint_descriptor_st),  // bLength (Endpoint1 descriptor)
		USB_ENDPOINT_DESCRIPTOR_TYPE,               // bDescriptorType
		0x81,                                       // bEndpointAddress
		0x03,                                       // bmAttributes (0x03=intr)
		CDC_NOTICE_BUFFER_SIZE,                     // wMaxPacketSize
		0x40                                        // bInterval
	},{
		sizeof(struct usb_interface_descriptor_st), // bLength (Interface1 descriptor)
		USB_INTERFACE_DESCRIPTOR_TYPE,              // bDescriptorType
		0x01,                                       // bInterfaceNumber
		0x00,                                       // bAlternateSetting
		0x02,                                       // bNumEndpoints
		0x0A,                                       // bInterfaceClass
		0x00,                                       // bInterfaceSubClass
		0x00,                                       // bInterfaceProtocol (0x00=no protocol, 0xFE=functional unit, 0xFF=vendor specific)
		0x00                                        // iInterface
	},{
		sizeof(struct usb_endpoint_descriptor_st),  // bLength (Enpoint2 descriptor)
		USB_ENDPOINT_DESCRIPTOR_TYPE,               // bDescriptorType
		0x02,                                       // bEndpointAddress
		0x02,                                       // bmAttributes (0x02=bulk)
		CDC_RXTX_BUFFER_SIZE,                       // wMaxPacketSize
		0x40                                        // bInterval
	},{
		sizeof(struct usb_endpoint_descriptor_st),  // bLength
		USB_ENDPOINT_DESCRIPTOR_TYPE,               // bDescriptorType
		0x82,                                       // bEndpointAddress
		0x02,                                       // bmAttributes (0x02=bulk)
		CDC_RXTX_BUFFER_SIZE,                       // wMaxPacketSize
		0x40                                        // bInterval
	}
	//, DFU_RUNTIME_DESC
};

ROM const USB_STRING_DESCRIPTOR(langid, USB_LANGID_English_United_States);
ROM const USB_STRING_DESCRIPTOR(manufacturer, 'H','o','n','k','e','n');
ROM const USB_STRING_DESCRIPTOR(product, 'C','D','C',' ','D','e','m','o');
ROM const USB_STRING_DESCRIPTOR(serialnum, '0','0','0','0','0','0','0','0','0','0','0','2');

ROM const struct usb_string_descriptor_st * cdc_str_descs[USB_NUM_STRINGS] = {
	(const struct usb_string_descriptor_st *) &langid,
	(const struct usb_string_descriptor_st *) &manufacturer,
	(const struct usb_string_descriptor_st *) &product,
	(const struct usb_string_descriptor_st *) &serialnum
};

#endif //__DESCRIPTORS_H__
