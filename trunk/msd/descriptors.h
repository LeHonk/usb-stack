/*
USB Descriptors for a Mass Storage Device

$Id: descriptors.h 81 2012-07-10 22:47:59Z tomhol $
 
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

#ifndef __DESCRIPTORS_H__
#define __DESCRIPTORS_H__

#include "usb_stack.h"

ROM const struct usb_device_descriptor_st msd_device_descriptor = {
  sizeof(msd_device_descriptor),                // bLength
  USB_DEVICE_DESCRIPTOR_TYPE,                   // bDescriptorType
  0x0200,                                       // bcdUSB
  0x00,                                         // bDeviceClass
  0x00,                                         // bDeviceSubClass
  0x00,                                         // bDeviceProtocol
  USB_EP0_BUFFER_SIZE,                          // bMaxPacketSize
  USB_VID,                                      // idVendor
  USB_PID,                                      // idProduct
  USB_DEV,                                      // bcdDevice
  USB_iManufacturer,                            // iManufacturer
  USB_iProduct,                                 // iProduct
  USB_iSerialNum,                               // iSerialNumber (Mandatory for MSD class)
  USB_NUM_CONFIGURATIONS                        // bNumConfigurations 
};

#define USB_CONFIG_DESC_TOT_LENGTH (9+9+7+7)	//+DFU_RUNTIME_DESC_LENGTH)
ROM const struct {
	struct usb_configuration_descriptor_st	config;
	struct usb_interface_descriptor_st			msd_iface;
	struct usb_endpoint_descriptor_st				msd_in_ep;
	struct usb_endpoint_descriptor_st				msd_out_ep;
//	struct dfu_descriptor_st								dfu_iface;
} msd_config_descriptor = {
	{
		sizeof(struct usb_configuration_descriptor_st),					// bLength
    USB_CONFIGURATION_DESCRIPTOR_TYPE,          // bDescriptorType
    sizeof(msd_config_descriptor),              // wTotalLength
    USB_NUM_INTERFACES,                         // bNumInterfaces
    0x01,                                       // bConfigurationValue
    0x00,                                       // iConfiguration ( 0 = none)
    0x80,                                       // bmAttributes (0x80 = bus powered)
    0x32,                                       // bMaxPower (in 2 mA units, 50=100 mA)
	},{
    sizeof(struct usb_interface_descriptor_st), // bLength (Interface0 descriptor starts here)
    USB_INTERFACE_DESCRIPTOR_TYPE,              // bDescriptorType
    0x00,                                       // bInterfaceNumber
    0x00,                                       // bAlternateSetting
    USB_NUM_ENDPOINTS,                          // bNumEndpoints (excluding EP0)
    0x08,                                       // bInterfaceClass (0x00=per endpoint specified, 0xFF=vendor specific)
    0x06,                                       // bInterfaceSubClass (0x00=per endpoint specified, 0xFF=vendor specific)
    0x50,                                       // bInterfaceProtocol (0x00=no protocol, 0xFE=as by command set, 0xFF=vendor specific)
    0x00,                                       // iInterface (none)
	},{
    sizeof(struct usb_endpoint_descriptor_st),  // bLength
    USB_ENDPOINT_DESCRIPTOR_TYPE,               // bDescriptorType
    0x81,                                       // bEndpointAddress (0x80=in)
    0x02,                                       // bmAttributes (0x02=bulk, 0x03=intr)
    MSD_RXTX_BUFFER_SIZE,                       // wMaxPacketSize
    0x00,                                       // bInterval
	},{
    sizeof(struct usb_endpoint_descriptor_st),  // bLength
    USB_ENDPOINT_DESCRIPTOR_TYPE,               // bDescriptorType
    0x01,                                       // bEndpointAddress (0x00=out)
    0x02,                                       // bmAttributes (0x02=bulk)
    MSD_RXTX_BUFFER_SIZE,                       // wMaxPacketSize
    0x00                                        // bInterval
//	},{DFU_RUNTIME_DESC
	}
};

ROM const USB_STRING_DESCRIPTOR(langid, USB_LANGID_English_United_States);
ROM const USB_STRING_DESCRIPTOR(manufacturer, 'H','o','n','k','e','n');
ROM const USB_STRING_DESCRIPTOR(product, 'C','a','r','d','r','e','a','d','e','r');
ROM const USB_STRING_DESCRIPTOR(serialnum, '0','0','0','0','0','0','0','0','0','0','0','1');

ROM const struct usb_string_descriptor_st * msd_str_descs[USB_NUM_STRINGS] = {
	(const struct usb_string_descriptor_st *) &langid,
	(const struct usb_string_descriptor_st *) &manufacturer,
	(const struct usb_string_descriptor_st *) &product,
	(const struct usb_string_descriptor_st *) &serialnum
};

#endif //__DESCRIPTORS_H__
