/*
Implementation of SCSI


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

#include <string.h>
#include <libpic30.h>
#include "honkutil.h"
#include "msd.h"
#include "scsi.h"
#include "mmc.h"

sense_data_t    sense_data;

inquery_response_t __attribute__((space(prog))) def_resp_data = INQUERY_RESPONSE_DATA;

uint32_t	current_lba;
uint16_t	n_blocks;

void
scsi_inquiry(void)
{
	_prog_addressT  p;

	_init_prog_address(p, def_resp_data);
	(void) _memcpy_p2d16(msd_block_buffer, p, sizeof(def_resp_data));
	csw.dDataResidue = sizeof(inquery_response_t);
	csw.bStatus = 0x00;
	msd_send_buffer();
}

void
scsi_mode_select(void) {

}

void
scsi_mode_sense(void)
{
	msd_block_buffer[0] = 0x03; //Data length excluding this byte
	msd_block_buffer[1] = 0x00; //Media type SBC
	msd_block_buffer[2] = 0x00; //Not write-protected
	msd_block_buffer[3] = 0x00; //No mode parameter block descriptors
	csw.dDataResidue = 4ul;
	csw.bStatus = 0x00;
	msd_send_buffer();
}

/* void
 * scsi_prevent_removal(void)
 * {
 * }
 */

void
scsi_report_luns(void)
{
}

void
scsi_request_sense(void)
{
	memcpy(msd_block_buffer, &sense_data, sizeof(sense_data_t));
	csw.dDataResidue = sizeof(sense_data_t);
	csw.bStatus = 0x00;
	msd_send_buffer();
}

void
scsi_send_diagnosic(void)
{
}

void
scsi_test_unit_ready(void)
{
}

/* =============================================================================================
 * Block commands
 * ============================================================================================= */

void
scsi_format_unit(void)
{
	return;
}

void
scsi_read(void)
{
	uint8_t        *p;

	p = (uint8_t *) &current_lba;
	p[3] = cbw.CB[2];
	p[2] = cbw.CB[3];
	p[1] = cbw.CB[4];
	p[0] = cbw.CB[5];

	p = (uint8_t *) &n_blocks;
	p[1] = cbw.CB[7];
	p[0] = cbw.CB[8];

	csw.bStatus = MSD_CSW_CMD_PASSED;
	csw.dDataResidue = 0;

	if ( (current_lba + n_blocks) > mmc_get_max_lba() ) {
		csw.bStatus = MSD_CSW_CMD_FAILED;
		sense_data.response_code = SCSI_SENSE_S_ILLEGAL_REQUEST;
		sense_data.asc = SCSI_SENSE_ASC_LBA_OUT_OF_RANGE;
		sense_data.ascq = SCSI_SENSE_ASCQ_LBA_OUT_OF_RANGE;
		// TODO: Should we stall here?
		return;
	}

	scsi_data_sent();                             /* Initiate 1:st block immediatly  */
}

void scsi_data_sent( void ) {
	if (n_blocks) {
		if (MMC_READ_SUCCESS == mmc_read(msd_block_buffer, current_lba)) {
			current_lba++;
			n_blocks--;
			csw.dDataResidue = mmc_get_block_size();
			msd_send_buffer();
		} else {
			csw.bStatus = MSD_CSW_CMD_FAILED;
			sense_data.response_code = SCSI_SENSE_S_MEDIUM_ERROR;
			sense_data.asc = SCSI_SENSE_ASC_UNREC_READ_ERROR;
			sense_data.ascq = SCSI_SENSE_ASCQ_UNREC_READ_ERROR;
			csw.dDataResidue = 0;
			// TODO: Stall here? How to indicate early errors.
		}
	}
}

void
scsi_read_capacity(void)
{
	*((uint32_t *) & msd_block_buffer[0]) = htonl(mmc_get_max_lba());
	*((uint32_t *) & msd_block_buffer[4]) = htonl(mmc_get_block_size());
	csw.dDataResidue = 8ul;
}

// TODO: Not required
void
scsi_start_stop_unit(void)
{
}

// TODO: Not required
void
scsi_syncronize_cache(void)
{
}

// TODO: Optional
void
scsi_verify(void)
{
}

void
scsi_write(void)
{
	uint8_t        *p;

	p = (uint8_t *) &current_lba;
	p[3] = cbw.CB[2];
	p[2] = cbw.CB[3];
	p[1] = cbw.CB[4];
	p[0] = cbw.CB[5];

	p = (uint8_t *) &n_blocks;
	p[1] = cbw.CB[7];
	p[0] = cbw.CB[8];

	csw.bStatus = MSD_CSW_CMD_PASSED;

	csw.dDataResidue = 512;
	msd_recv_buffer();
}

void
scsi_data_received( void ) {
	if (n_blocks--) {
		if ( mmc_write_protected() ) {
			csw.bStatus = MSD_CSW_CMD_FAILED;
			sense_data.sense_key = SCSI_SENSE_S_NOT_READY;
			sense_data.asc = SCSI_SENSE_ASC_WRITE_PROTECTED;
			sense_data.ascq = SCSI_SENSE_ASCQ_WRITE_PROTECTED;
		} else if ( mmc_write( msd_block_buffer, current_lba ) ) {
			csw.bStatus = MSD_CSW_CMD_FAILED;
			sense_data.sense_key = SCSI_SENSE_S_MEDIUM_ERROR;
			sense_data.asc = SCSI_SENSE_ASC_DEV_WRITE_FAULT;
			sense_data.ascq = SCSI_SENSE_ASCQ_DEV_WRITE_FAULT;  
		}
		current_lba++;
		csw.dDataResidue = 512;
		msd_recv_buffer();
	} else {
		msd_send_csw();
	}
}

/* ===========================================================================
 * Multimedia commands                                                       *
 * ===========================================================================
 */

void
scsi_read_format_capacities(void)
{
}

void scsi_read_TOC_PMA_ATIP(void)
{
}

/* ===========================================================================
 * Housekeeping functions                                                    *
 * ===========================================================================
 */

void
reset_sense_data(void)
{
	sense_data.response_code = 0x70;
	sense_data.valid = 0x0;
	sense_data.obselete = 0x00;
	sense_data.sense_key = 0x0;
	sense_data.reserved = 0x0;
	sense_data.ili = 0x0;
	sense_data.eom = 0x0;
	sense_data.filemark = 0x0;
	sense_data.information = 0x00000000;
	sense_data.add_sense_len = 0x0a;
	sense_data.cmd_specific_info = 0x00000000;
	sense_data.asc = 0x00;
	sense_data.ascq = 0x00;
	sense_data.fruc = 0x00;
	sense_data.sence_key_specific[0] = 0x00;
	sense_data.sence_key_specific[1] = 0x00;
	sense_data.sence_key_specific[2] = 0x00;
}

void
scsi_handle_cdb(void)
{
	switch (cbw.CB[0]) {
	case SCSI_CDB_CMD_INQUIRY:
		scsi_inquiry();
		break;
	case SCSI_CDB_CMD_READ_CAPACITY:
		scsi_read_capacity();
		break;
	case SCSI_CDB_CMD_READ_10:
		scsi_read();
		break;
	case SCSI_CDB_CMD_WRITE_10:
		scsi_write();
		break;
	case SCSI_CDB_CMD_REQUEST_SENSE:
		scsi_request_sense();
		break;
	case SCSI_CDB_CMD_SENSE_6:
		scsi_mode_sense();
		break;
	case SCSI_CDB_CMD_TEST_UNIT_READY:
		scsi_test_unit_ready();
		break;
//	case SCSI_CDB_CMD_VERIFY:
//		scsi_verify();
//		break;
//	case SCSI_CDB_CMD_START_STOP_UNIT:
//		scsi_start_stop_unit();
//		break;
	default:
    csw.bStatus = MSD_CSW_CMD_FAILED;           //Command failed
		csw.dDataResidue = 0;
		reset_sense_data();
		sense_data.sense_key = SCSI_SENSE_S_ILLEGAL_REQUEST; //Illegal command
		sense_data.asc = SCSI_SENSE_ASC_INVALID_COMMAND; //Invalid command operation code
		// sense_data.ascq = SCSI_SENSE_ASCQ_INVALID_COMMAND;
	}
}
