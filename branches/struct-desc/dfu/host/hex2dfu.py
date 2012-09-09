#!/usr/bin/env python2.7

# TODO: Actual values
devid     = { 'p24fj256gb106': 0xFFFF
			, 'p18f2550': 0x1234
			}

targetmem = { 'int_flash': 0
			, 'int_eeprom': 1
#			, 'ext_flash': 2
#			, 'ext_eeprom': 3
			}

# TODO: Actual values
maxmem    = { 'p24fj256gb106':	{'int_flash':255*1024, 'int_eeprom':2048}
			, 'p18f2550':		{'int_flash':255*1024, 'int_eeprom':2048}
			}

blocksize = { 'p24fj256gb106':	{'int_flash':64, 'int_eeprom':16}
			, 'p18f2550':		{'int_flash':32, 'int_eeprom':16}
			}


if __name__ = '__main__':
	import sys
	import argparse
	import os.path
	from intelhex import IntelHex
	from cStringIO import StringIO
	from dfu_suffix import *

	parser = argparse.ArgumentParser( description='Convert an Intel HEX file into a dfu file suitable for OpenPICUSB bootloader.',
									  epilog='''Default output filename is the input filename with
												".dfu" in stead of ".hex".''')
	action = parser.add_mutually_exclusive_group( required=True )
#	parser.add_argument( '-f', '--force',     help='Forcefully try to execute given command. May result in unusable files.', action='store_true', default=False )
	parser.add_argument( '-p', '--processor', help='Target processor (currently only p18f2550 and p24fj256bg106)', dest='proc', nargs=1, choices=devid, required=True )
	parser.add_argument( '-t', '--targetmem', help='Target memory', nargs=1, choices=targetmem, default='int_flash' )
	parser.add_argument( '-o', '--output',    help='Output file.', type=argparse.FileType('wb'), dest='outfile', nargs=1, metavar='file.dfu' )
	parser.add_argument( 'hexfile', help='Firmware file with DFU suffix.',  type=argparse.FileType('r'), nargs=1 )
	parser.add_argument( 'vid',     help='The Vendor ID to use.',  action='store', type=int, nargs='?', default=0xFFFF );
	parser.add_argument( 'pid',     help='The Product ID to use.', action='store', type=int, nargs='?', default=0xFFFF );
	parser.add_argument( 'did',     help='The Device version to use.',  action='store', type=int, nargs='?', default=0xFFFF );

	args = parser.parse_args()
	(rootname, ext) = os.path.splitext( args.hexfile.name )

	try:
		ih = IntelHex.fromfile(hexfile)
	except FileNotFoundException:
		print 'File "%(name)s" not found.' % args.hexfile
		sys.exit(1)
	hexfile.close();

	blob = StringIO()

	PROC = args.proc[0]
	TGTMEM = args.targetmem[0]

	DEVID = devid[PROC]
	MAXMEM = maxmem[PROC][TGTMEM]
	BLOCKSIZE = blocksize[PROC][TGTMEM]

#	Construct bootloader header
	blob.write( 'HBL\x01' )							# Magic identifier
	blob.write( struct.pack('>h', DEVID )			# Device ID in big endian 16bits
	blob.write( struct.pack('>h', tgt_mem[TGTMEM] )	# Target memory

	for addr in range(0, MAXMEM, BLOCKSIZE):
		blob.write(struct.pack('>l', addr)
		ih.tobinfile(blob, start=addr, size=BLOCKSIZE)

	blob_suffix = Suffix._make( args.did, args.pid, args.vid, 0x0100, 'DFU', 16, 0 )
	firmware = append_suffix(blob, user_suffix)
	
	if args.outfile is None:
		args.outfile = open( rootname + '.dfu', 'wb' )

	args.outfile.write(firmware)
	
	outfile.close()
