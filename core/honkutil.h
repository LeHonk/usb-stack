#ifndef __HONKUTIL_H__
#define __HONKUTIL_H__

#include <stdint.h>

/* Stolen from http://graphics.stanford.edu/~seander/bithacks.html */
#define MIN(x,y)	(y) ^ (((x) ^ (y)) & -((x) < (y)))
#define MAX(x,y)	(x) ^ (((x) ^ (y)) & -((x) < (y)))

#define SWAP(x,y)	(((x) ^ (y)) && (((y) ^= (x) ^= (y) ^= (x)), ((x) ^= (y))))

inline static uint16_t
htons ( uint16_t n )
{
	return ( ( n & 0xFF ) << 8 ) | ( ( n & 0xFF00 ) >> 8 );
}

inline static uint16_t
ntohs ( uint16_t n )
{
	return ( ( n & 0xFF ) << 8 ) | ( ( n & 0xFF00 ) >> 8 );
}

inline static uint32_t
htonl ( uint32_t n )
{
	return ( ( n & 0xFF ) << 24 ) | ( ( n & 0xFF00 ) << 8 ) |
		( ( n & 0xFF0000 ) >> 8 ) | ( ( n & 0xFF000000 ) >> 24 );
}

inline static uint32_t
ntohl ( uint32_t n )
{
	return ( ( n & 0xFF ) << 24 ) | ( ( n & 0xFF00 ) << 8 ) |
		( ( n & 0xFF0000 ) >> 8 ) | ( ( n & 0xFF000000 ) >> 24 );
}

#endif
