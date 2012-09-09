; This work is licensed under the Creative Commons Attribution 3.0 Unported License.
; To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
; or send a letter to
;   Creative Commons,
;   171 Second Street,
;   Suite 300,
;   San Francisco,
;   California,
;   94105,
;   USA.


#include "P18CXXX.INC"

	STRING CODE

DFU_PAGE_SIZE	EQU	64

erase_ptr	EQU	-3
erase_flash:
	GLOBAL erase_flash
		MOVLW	LOW(erase_ptr)
		MOVFF	PLUSW1, TBLPTRL
		MOVLW	LOW(erase_ptr+1)
		MOVFF	PLUSW1, TBLPTRH
		MOVLW	LOW(erase_ptr+2)
		MOVFF	PLUSW1, TBLPTRU
		BSF		EECON1, EEPGD
		BCF		EECON1, CFGS
		BSF		EECON1, WREN
		BSF		EECON1, FREE
		BCF		INTCON,	GIE
		MOVLW	0x55
		MOVWF	EECON2, 0
		MOVLW	0xAA
		MOVWF	EECON2, 0
		BSF		EECON1, WR
		BSF		INTCON, GIE
		RETURN

read_dst	EQU	-2
read_src	EQU -5
read_flash:
	GLOBAL	read_flash
		MOVLW	LOW(read_src)
		MOVFF	PLUSW1, TBLPTRL
		MOVLW	LOW(read_src+1)
		MOVFF	PLUSW1, TBLPTRH
		MOVLW	LOW(read_src+2)
		MOVFF	PLUSW1, TBLPTRU
		MOVLW	LOW(read_dst)
		MOVFF	PLUSW1, FSR0L
		MOVLW	LOW(read_dst+1)
		MOVFF	PLUSW1, FSR0H
		MOVLW	DFU_PAGE_SIZE
read_loop:
		TBLRD*+
		MOVFF	TABLAT, POSTINC0
		DECFSZ	WREG, F
		BRA		read_loop
		RETURN


; TODO: Check stack usage
write_dst	EQU	-3
write_src	EQU	-5
write_flash:
	GLOBAL	write_flash
		MOVLW	LOW(write_src)
		MOVFF	PLUSW1, FSR0L
		MOVLW	LOW(write_src+1)
		MOVFF	PLUSW1, FSR0H
		MOVLW	LOW(write_dst)
		MOVFF	PLUSW1, TBLPTRL
		MOVLW	LOW(write_dst+1)
		MOVFF	PLUSW1, TBLPTRH
		MOVLW	LOW(write_dst+2)
		MOVFF	PLUSW1, TBLPTRU
		MOVLW	2
write_flash_outer:
		MOVWF	POSTDEC1
		MOVLW	32
write_flash_block:
		MOVFF	POSTINC0, TABLAT
		TBLWT+*
		DECFSZ	WREG, F
		BRA		write_flash_block
		BSF		EECON1, EEPGD
		BCF		EECON1, CFGS
		BSF		EECON1, WREN
		BCF		INTCON, GIE
		MOVLW	0x55
		MOVWF	EECON2, 0
		MOVLW	0xAA
		MOVWF	EECON2, 0
		BSF		EECON1, WR
		MOVF	PREINC1
		DECFSZ	WREG, F
		BRA		write_flash_outer
		BCF		EECON1, WREN
		BSF		INTCON, GIE
		RETURN
	END

