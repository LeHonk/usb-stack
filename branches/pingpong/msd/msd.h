/* $Id: msd.h 74 2012-07-05 22:40:24Z tomhol $
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

#include <stdint.h>

#define MSD_CBW_Signature	0x43425355
typedef struct _MSD_CBW {
	uint32_t  dSignature;
	uint32_t  dTag;
	uint32_t  dDataTransferLenght;
	uint8_t   bmFlags;
	int       reserved0:4;
	int       bLUN:4;
	int       reserved1:3;
	int       bCBLength:5;
	uint8_t   CB[16];
} CBW_t;

#define MSD_CSW_Signature	0x53425355
#define MSD_CSW_CMD_PASSED	0x00
#define MSD_CSW_CMD_FAILED	0x01
#define MSD_CSW_PHASE_ERROR	0x02
typedef struct _MSD_CSW {
	uint32_t  dSignature;
	uint32_t  dTag;
	uint32_t  dDataResidue;
	uint8_t   bStatus;
} CSW_t;

extern CBW_t cbw;

extern CSW_t csw;

extern uint8_t msd_block_buffer[512];

/* Initialize usb msd subsystem */
void      InitMSD ( void );

void      msd_send_buffer ( void );
void			msd_recv_buffer ( void );
void			msd_send_csw ( void );

#endif
