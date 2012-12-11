/*
USB Descriptors for a Mass Storage Device


 
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

ROM const struct usb_device_descriptor_st hid_device_descriptor = {
	sizeof(hid_device_descriptor),
	USB_DEVICE_DESCRIPTOR_TYPE,
	0x0200,								// bcdUSB
	0x00,									// bDeviceClass
	0x00,									// bDeviceSubClass
	0x00,									// bDeviceProtocol
	USB_EP0_BUFFER_SIZE,
	USB_VID,
	USB_PID,
	USB_DEV,
	USB_iManufacturer,
	USB_iProduct,
	USB_iSerialNum,
	USB_NUM_CONFIGURATIONS
};

#define KB_RPT_SIZE 0x003F
#define PT_RPT_SIZE 0x0032
//#define USB_CONFIG_DESC_TOT_LENGTH (9+9+9+0x3F+7+9+9+32+7)
ROM const struct {
	struct usb_configuration_descriptor_st	config;
	struct usb_interface_descriptor_st			kb_iface;
	struct usb_hid_descriptor_st						kb_hid;
	struct usb_endpoint_descriptor_st				kb_ep;
	struct usb_interface_descriptor_st			pt_iface;
	struct usb_hid_descriptor_st						pt_hid;
	struct usb_endpoint_descriptor_st				pt_ep;
} hid_configuration = {
	{
		sizeof(struct usb_configuration_descriptor_st),
		USB_CONFIGURATION_DESCRIPTOR_TYPE,
		sizeof(hid_configuration),
		USB_NUM_INTERFACES,
		0x01,					// bConfigurationValue
		0x00,					// iConfiguration (0=none)
		0xA0,					// bmAttributes (0x80 = bus powered, 0x20 = remote wakeup)
		0x32,					// bMaxPower (in 2 mA units, 0x32=50=100 mA)
	},{
		sizeof(struct usb_interface_descriptor_st),
		USB_INTERFACE_DESCRIPTOR_TYPE,
		0x00,					// bInterfaceNumber
		0x00,					// bAlternateSetting
		0x01,					// bNumEndpoints (excluding EP0)
		0x03,					// bInterfaceClass (0x03=HID)
		0x01,					// bInterfaceSubClass (0x00=no subclass, 0x01=Boot device)
		0x01,					// bInterfaceProtocol (0x01 Keyboard)
		0x00,					// iInterface (none)
	},{
		sizeof(struct usb_hid_descriptor_st),
		USB_HID_DESCRIPTOR_TYPE,
		0x0101,				// bcdHID
		0x00,					// bCountryCode (0x00=N/A)
		0x01,					// bNumDescriptors
		USB_HID_REPORT_DESCRIPTOR_TYPE,
		KB_RPT_SIZE
	},{
		sizeof(struct usb_endpoint_descriptor_st),
		USB_ENDPOINT_DESCRIPTOR_TYPE,
		0x81,					// bEndpointAddress (0x80=in)
		0x03,					// bmAttributes (0x02=bulk, 0x03=intr)
		0x0008,				// wMaxPacketSize (low byte)
		0x0A,					// bInterval
	},{
		sizeof(struct usb_interface_descriptor_st),
		USB_INTERFACE_DESCRIPTOR_TYPE,
		0x01,					// bInterfaceNumber
		0x00,					// bAlternateSetting
		0x01,					// bNumEndpoints
		0x03,					// bInterfaceClass (0x03=HID Class)
		0x01,					// bInterfaceSubClass (0x00=No subclass, 0x01=Boot device)
		0x02,					// bInterfaceProtocol (0x02 Mouse)
		0x00,					// iInterface (none)
	},{
		sizeof(struct usb_hid_descriptor_st),
		USB_HID_DESCRIPTOR_TYPE,
		0x0101,				// bcdHID
		0x00,					// bCountryCode (0x00=N/A)
		0x01,					// bNumDescriptors
		USB_HID_REPORT_DESCRIPTOR_TYPE,
		PT_RPT_SIZE
	},{
		sizeof(struct usb_endpoint_descriptor_st),
		USB_ENDPOINT_DESCRIPTOR_TYPE,
		0x82,					// bEndpointAddress
		0x03,					// bmAttributes (0x03=intr)
		0x0008,				// wMaxPacketSize
		0x0A					// bInterval
	}
};

ROM const uint8_t kb_report_descriptor[0x3F] = {
		0x05, 0x01,   // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,   // USAGE (Keyboard)
    0xa1, 0x01,   // COLLECTION (Application)
    0x05, 0x07,   //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,   //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,   //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,   //   LOGICAL_MINIMUM (0)
    0x25, 0x01,   //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,   //   REPORT_SIZE (1)
    0x95, 0x08,   //   REPORT_COUNT (8)
    0x81, 0x02,   //   INPUT (Data,Var,Abs)
    0x95, 0x01,   //   REPORT_COUNT (1)
    0x75, 0x08,   //   REPORT_SIZE (8)
    0x81, 0x01,   //   INPUT (Cnst)
    0x95, 0x05,   //   REPORT_COUNT (5)
    0x75, 0x01,   //   REPORT_SIZE (1)
    0x05, 0x08,   //   USAGE_PAGE (LEDs)
    0x19, 0x01,   //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,   //   USAGE_MAXIMUM (Kana)
    0x91, 0x02,   //   OUTPUT (Data,Var,Abs)
    0x95, 0x01,   //   REPORT_COUNT (1)
    0x75, 0x03,   //   REPORT_SIZE (3)
    0x91, 0x01,   //   OUTPUT (Cnst)
    0x95, 0x06,   //   REPORT_COUNT (6)
    0x75, 0x08,   //   REPORT_SIZE (8)
    0x15, 0x00,   //   LOGICAL_MINIMUM (0)
    0x25, 0x65,   //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,   //   USAGE_PAGE (Keyboard)
    0x19, 0x00,   //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,   //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,   //   INPUT (Data,Ary,Abs)
    0xc0,         // END_COLLECTION
};

ROM const uint8_t pt_report_descriptor[0x32] = {
    0x05, 0x01,   // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,   // USAGE (Mouse)
    0xa1, 0x01,   // COLLECTION (Application)
    0x09, 0x01,   //   USAGE (Pointer)
    0xa1, 0x00,   //   COLLECTION (Physical)
    0x05, 0x09,   //     USAGE_PAGE (Button)
    0x19, 0x01,   //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,   //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,   //     LOGICAL_MINIMUM (0)
    0x25, 0x01,   //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,   //     REPORT_COUNT (3)
    0x75, 0x01,   //     REPORT_SIZE (1)
    0x81, 0x02,   //     INPUT (Data,Var,Abs)
    0x95, 0x01,   //     REPORT_COUNT (1)
    0x75, 0x05,   //     REPORT_SIZE (5)
    0x81, 0x01,   //     INPUT (Cnst)
    0x05, 0x01,   //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,   //     USAGE (X)
    0x09, 0x31,   //     USAGE (Y)
    0x15, 0x81,   //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,   //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,   //     REPORT_SIZE (8)
    0x95, 0x02,   //     REPORT_COUNT (2)
    0x81, 0x06,   //     INPUT (Data,Var,Rel)
    0xc0,         //   END_COLLECTION
    0xc0,         // END_COLLECTION
};
 
ROM const USB_STRING_DESCRIPTOR(langid, USB_LANGID_English_United_States);
ROM const USB_STRING_DESCRIPTOR(manufacturer, 'H','o','n','k','e','n');
ROM const USB_STRING_DESCRIPTOR(product, 'D','e','s','k','t','o','p',' ','D','e','m','o');
ROM const USB_STRING_DESCRIPTOR(serialnum, '0','0','0','0','0','0','0','0','0','0','0','1');

#define NUM_STRINGS	4
ROM const struct usb_string_descriptor_st * hid_str_descs[NUM_STRINGS] = {
	(const struct usb_string_descriptor_st *) &langid,
	(const struct usb_string_descriptor_st *) &manufacturer,
	(const struct usb_string_descriptor_st *) &product,
	(const struct usb_string_descriptor_st *) &serialnum
};

#endif //__DESCRIPTORS_H__
