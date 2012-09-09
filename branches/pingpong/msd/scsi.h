/*
Implementation of SCSI protocol subset
$Id: scsi.h 74 2012-07-05 22:40:24Z tomhol $

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

#ifndef __SCSI_H__
#define __SCSI_H__

#include <stdint.h>

#define SCSI_SENSE_S_NOT_READY				0x02;
#define SCSI_SENSE_S_MEDIUM_ERROR			0x03;
#define SCSI_SENSE_S_ILLEGAL_REQUEST		0x05;
#define SCSI_SENSE_S_UNIT_ATTENTION			0x06;
#define SCSI_SENSE_ASC_DEV_WRITE_FAULT		0x03;
#define SCSI_SENSE_ASC_UNREC_READ_ERROR		0x11;
#define SCSI_SENSE_ASC_INVALID_COMMAND		0x20;
#define SCSI_SENSE_ASC_LBA_OUT_OF_RANGE		0x21;
#define SCSI_SENSE_ASC_WRITE_PROTECTED		0x27;
#define SCSI_SENSE_ASC_MEDIUM_NOT_PRESENT	0x3A;
#define SCSI_SENSE_ASCQ_LBA_OUT_OF_RANGE	0x00;
#define SCSI_SENSE_ASCQ_MEDIUM_NOT_PRESENT	0x00;
#define SCSI_SENSE_ASCQ_DEV_WRITE_FAULT		0x00;
#define SCSI_SENSE_ASCQ_UNREC_READ_ERROR	0x00;
#define SCSI_SENSE_ASCQ_WRITE_PROTECTED		0x00;

// _SCSI_SENSE_DATA
typedef union {
	struct {
		uint8_t         bytes[18];
	};
	struct {
		int             response_code:7;
		int             valid:1;
		uint8_t         obselete;
		int             sense_key:4;
		int             reserved:1;
		int             ili:1;
		int             eom:1;
		int             filemark:1;
		uint32_t        information;
		uint8_t         add_sense_len;
		uint32_t        cmd_specific_info;
		uint8_t         asc;
		uint8_t         ascq;
		uint8_t         fruc;
		uint8_t         sence_key_specific[3];
	};
}               sense_data_t;
#define DEFAULT_SENSE_DATA	\
				{ 0x70										/* Valid, Responce Code:7 */ \
				, 0x00										/* Obselete */ \
				, 0x00										/* Filemark, EOM, ILI Reserved, Sense Key:4 */ \
				, 0x00, 0x00, 0x00, 0x00	/* Information:32 */ \
				, 0x0A										/* Add Sense length */ \
				, 0x00, 0x00, 0x00, 0x00	/* Cmd Specific Info */ \
				, 0x00										/* ASC */ \
				, 0x00										/* ASCQ */ \
				, 0x00										/* FRUC */ \
				, 0x00 , 0x00, 0x00				/* Sense Key Specific */ \
				}

#define SCSI_CDB_CMD_TEST_UNIT_READY			0x00
#define SCSI_CDB_CMD_REQUEST_SENSE				0x03
#define SCSI_CDB_CMD_FORMAT_UNIT					0x04
#define SCSI_CDB_CMD_READ_6								0x08
#define SCSI_CDB_CMD_WRITE_6							0x0A
#define SCSI_CDB_CMD_INQUIRY							0x12
#define SCSI_CDB_CMD_SELECT_6							0x15
#define SCSI_CDB_CMD_SENSE_6							0x1A
#define SCSI_CDB_CMD_START_STOP_UNIT			0x1B
#define SCSI_CDB_CMD_SEND_DIAGNOSTIC			0x1D
#define SCSI_CDB_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL	0x1E
#define SCSI_CDB_CMD_READ_FORMAT_CAPACITIES	0x23
#define SCSI_CDB_CMD_READ_CAPACITY				0x25
#define SCSI_CDB_CMD_READ_10							0x28
#define SCSI_CDB_CMD_WRITE_10							0x2A
#define SCSI_CDB_CMD_VERIFY								0x2F
#define SCSI_CDB_CMD_SYNCHRONIZE_CACHE		0x35
#define SCSI_CDB_CMD_READ_TOC							0x43
#define SCSI_CDB_CMD_SELECT_10						0x55
#define SCSI_CDB_CMD_SENSE_10							0x5A
#define SCSI_CDB_CMD_REPORT_LUNS					0xA0
#define SCSI_CDB_CMD_READ_12							0xA8
#define SCSI_CDB_CMD_WRITE_12							0xAA

typedef struct _SCSI_INQUERY_RESPONSE {
	uint8_t         peripherial;
	uint8_t         removable;
	uint8_t         version;
	uint8_t         response_data_format;
	uint8_t         additional_length;
	uint8_t         sccstp;
	uint8_t         bqueetc;
	uint8_t         cmdque;
	char            vendor[8];
	char            product_id[16];
	char            product_rev[4];
}               inquery_response_t;

#define SCSI_INQ_PQ				0x70
#define SCSI_INQ_PDT			0x0F
#define SCSI_INQ_PDT_DA		0x00		/* Direct-access; Magnetic and flash device (SBC-2) */
#define SCSI_INQ_PDT_CD		0x05		/* CD/DVD Device (MMC-4) */
#define SCSI_INQ_PDT_OP		0x07		/* Optical memory device (Non-CD) (SBC) */
#define SCSI_INQ_PDT_RBC	0x0E		/* Reduced block command direct-access device (RBC) */
#define SCSI_INQ_RMB			0x80
#define SCSI_INQ_SPC_2		0x04
#define SCSI_INQ_SPC_3		0x05
#define SCSI_INQ_NORMACA	0x20
#define SCSI_INQ_HISUP		0x10
#define SCSI_INQ_RESP_FMT	0x0F
#define SCSI_INQ_RESP_FMT_V	0x02
#define SCSI_INQ_SCCS			0x80
#define SCSI_INQ_ACC			0x40
#define SCSI_INQ_TPGS			0x30
#define SCSI_INQ_3PC			0x08
#define SCSI_INQ_PROTECT	0x01
#define SCSI_INQ_BQUE			0x80
#define SCSI_INQ_ENCSERV	0x40
//
#define SCSI_INQ_VS1			0x20
#define SCSI_INQ_MULTIP		0x10
#define SCSI_INQ_MCHNGR		0x08
#define SCSI_INQ_ADDR16		0x01
#define SCSI_INQ_WBUS16		0x20
#define SCSI_INQ_SYNC			0x10
#define SCSI_INQ_LINKED		0x08
#define SCSI_INQ_CMDQUE		0x02
//
#define SCSI_INQ_VS2      0x01

#define INQUERY_RESPONSE_DATA	\
				{ 0x00	/*Direct access block device (SBC-2) */		\
				, 0x80	/* Device is removable */									\
				, 0x04	/* SPC-2 compliance */										\
				, 0x02	/* Respons data format */									\
				, 0x20	/* 0x24 byte response */									\
				, 0x00	/* No additional field */									\
				, 0x00	/* --"-- */																\
				, 0x00	/* --"-- */																\
				, {'H','o','n','k','e','n','.','.'}					/* T10 Vendor ID TODO: Register */				\
				, {'M','M','C',' ','C','a','r','d','R','e','a','d','e','r','.','.'}	/* Product ID */	\
				, {'0','0','0','1'}							/* Product revision	*/																\
				}

void scsi_handle_cdb(void);
void scsi_data_received( void );
void scsi_data_sent( void );

#endif
