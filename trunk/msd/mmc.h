/*
Implementation of MultiMediaCard access



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

#ifndef __MMC_H__
#define __MMC_H__

#include <stdint.h>

inline static uint32_t
mmc_get_max_lba ( void )
{
	return 4194303UL;			// 2GiB (2 * 1024*1024*1024 / 512 - 1)
}

inline static uint32_t
mmc_get_block_size ( void )
{
	return 512UL;
}

inline static int
mmc_write_protected( void )
{
	return 0;                                 /* TODO: Add actual check :-) */
}

#define MMC_READ_SUCCESS	0
#define MMC_READ_FAIL 		-1

inline static int8_t
mmc_read ( uint8_t * buffer, uint32_t lba )
{
	return MMC_READ_SUCCESS;
}

#define MMC_WRITE_SUCCESS	0
#define MMC_WRITE_FAIL -1

inline static int8_t
mmc_write( uint8_t * buffer, uint32_t lba )
{
  return MMC_WRITE_SUCCESS;
}

#endif
