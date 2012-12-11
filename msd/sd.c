/*
 * =====================================================================================
 *
 *       Filename:  sd.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/19/2012 04:44:00 PM
 *       Revision:  
 *
 *         Author:  Tomas Holmqvist (TH), LeHonk@users.noreply.github.com
 *   Organization:  
 *
 * =====================================================================================
 */

#define	SD_R1_OK		0x00			/*  */
#define	SD_R1_IDLE		0x01			/*  */
#define	SD_R1_ERASE_RESET	0x02			/*  */
#define	SD_R1_ILLEGAL_CMD	0x04			/*  */
#define	SD_R1_CMD_CRC_ERR	0x08			/*  */
#define	SD_R1_ERASE_SEQ_ERR	0x10			/*  */
#define	SD_R1_ADDR_ERR		0x20			/*  */
#define	SD_R1_PARM_ERR		0x40			/*  */


#define	GO_IDLE_STATE		0			/*  */
#define	SEND_OP_COND		1			/*  */
#define	APP_SEND_OP_COND	41			/*  */
#define	SEND_IF_COND		8			/*  */
#define	SEND_CSD		9			/*  */
#define	SEND_CID		10			/*  */
#define	STOP_TRANSMISSION	12			/*  */
#define	SET_BLOCKLEN		16			/*  */
#define	READ_SINGLE_BLOCK	17			/*  */
#define	READ_MULTIPLE_BLOCK	18			/*  */
#define	SET_BLOCK_COUNT		23			/*  */
#define	SET_WR_BLOCK_ERASE_CNT				/*  */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sd_init
 *  Description:  
 * =====================================================================================
 */
    void
sd_init ( void )
{
    return;
}		/* -----  end of function sd_init  ----- */


