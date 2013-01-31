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
#ifndef __MSD_H__
#define __MSD_H__

struct cdc_function_descriptor_st {
	uint8_t	bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
};

struct cdc_function_descriptor4_st {
	uint8_t	bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t bData;
};

struct cdc_function_descriptor5_st {
	uint8_t	bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubtype;
	uint8_t	bData0;
	uint8_t	bData1;
};

/* Initialize usb cdc acm subsystem */
void		InitCDC( void );
void		OpenCDC( uint8_t config, uint16_t spbrg );
char		BusyCDC( void );
char		DataRdyCDC( void );

char		getcCDC( void );
uint16_t	getaCDC( char *array, uint16_t length );
uint16_t	getsCDC( char *string, uint16_t length );

void		putcCDC( char c );
uint16_t	putaCDC( const char *array, uint16_t length );
uint16_t	putsCDC( const char *string );

#endif
