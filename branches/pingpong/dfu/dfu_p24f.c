/*
Arcitecture specific flash memory functions
$Id: dfu_p24f.c 75 2012-07-09 00:08:36Z tomhol $

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

#include <p24Fxxxx.h>
#include <libpic30.h>
#include <assert.h>
#include "dfu.h"


#define FLASH_PAGE  512u                        /* Flash Page size (minimum erasable area) in instructions */
#define FLASH_ROW   64u                         /* Flash Row size (minimum writable area) in instructions */

unsigned int
get_devid( void )
{
	unsigned int devid;

	_memcpy_p2d16( (char *) &devid, 0xFF0000, 1u );
	return devid;
}

void
erase_flash( uint32_t dst )
{
/* 	_erase_flash( (_prog_addressT) dst ); */
	uint16_t	offset;
	
	TBLPAG = dst >> 16;
	offset = dst & 0xFFFF;
	__builtin_tblwtl( offset, 0x0000 );
	NVMCON = 0x4042;
	asm("DISI #5");
	__builtin_write_NVM();
}

void
read_flash( uint8_t *dst, uint32_t src )
{
	_memcpy_p2d24( dst, src, DFU_PAGE_SIZE );
}

void
write_flash( uint32_t dst, const uint8_t *src )
{
	uint16_t	n;
	uint16_t	i;
	uint16_t	offset;
	const uint16_t	*data;

	data = (const uint16_t *) src;

	assert( 0x00000000 == dst % FLASH_ROW * 4 );  /* Assert that destination is aligned on a flash row */

	NVMCON = 0x4001;

	for ( n=0; n<_FLASH_PAGE/FLASH_ROW; n++ ) {

		TBLPAG = dst >> 16;
		offset = dst & 0xFFFF;

		for ( i=0; i<_FLASH_ROW; i++ ) {
			__builtin_tblwtl( offset, *data++);
			assert( 0x0000 == (*data & 0xFF00));        /* Assert that high bits 32:25 are meaningless  */
			__builtin_tblwth( offset, *data++);
			offset += 2;
		}
		asm("DISI #5");                             /* Disable interrupts for 5 cycles */
		__builtin_write_NVM();                      /* Unlock and set WR */

		dst += 4 * FLASH_PAGE;
	}
}

