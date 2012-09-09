# $Id: Makefile 75 2012-07-09 00:08:36Z tomhol $

MCU = 24FJ256GB106

TARGET = bootloader.hex

ASRCS =
CSRCS = usb_stack.c dfu.c dfu_p24f.c

LIBS = libc libm libpic30 libpPIC24Fxxx

#LFLAGS = -Wl,--no-data-init

include ../core/Makefile.include

