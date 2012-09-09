#!/usr/bin/env python2.7

from collections import namedtuple
import struct
from zlib import crc32

_suffix_fmt_str = '=HHHH3sBI'
Suffix = namedtuple( 'DfuSuffix', 'bcdDevice idProduct idVendor bcdDFU ucDfuSignature bLength dwCRC' )
generic_suffix = Suffix._make((0xFFFF, 0xFFFF, 0xFFFF, 0x0100, 'DFU', 16, 0))

def get_suffix(blob):
	suffixblob = blob[-16:]
	suffix = Suffix._make(struct.unpack(_suffix_fmt_str, suffixblob))
	return suffix

def report_suffix(blob, _na, force=False):
	try:
		cur = verify_suffix(blob, generic_suffix)
	except SuffixError:
		if force:
			cur = get_suffix(blob)
		else:
			print se
			sys.exit(-1)
	print '''
Device Firmeware Upgrade file suffix
------------------------------------
Signature:      %(ucDfuSignature)s
DFU version:    %(bcdDFU)2x
Vendor ID:      %(idVendor)04X
Product ID:     %(idProduct)04X
Device version: %(bcdDevice)4x
Suffix length:  %(bLength)d
Checksum:       %(dwCRC)08X
''' % cur

def verify_suffix(blob, usr, force=False):

	cur = get_suffix(blob)
	
	# Check signature
	if (cur.ucDfuSignature != 'DFU'):
		raise SignatureSuffixError( 'DFU signature not found', None)

	# Check dfu version
	if (cur.bcdDFU != 0x0100):
		raise SignatureSuffixError( 'Wrong DFU version', current.bcdDFU )
	
	# Check suffix length
	if (cur.bLength != 16):
		raise SignatureSuffixError( 'Wrong suffix length: %d' % current.bLength, current.bcdDFU )
		
	# Check VID/PID/DID
	if (cur.idVendor != 0xFFFF and 0xFFFF != usr.idVendor != cur.idVendor):
		raise IdSuffixError( 'Vendor ID mismatch' )
	if (cur.idProduct != 0xFFFF and 0xFFFF != usr.idProduct != cur.idProduct):
		raise IdSuffixError( 'Product ID mismatch' )
	if (cur.bcdDevice != 0xFFFF and 0xFFFF != usr.bcdDevice != cur.bcdProduct):
		raise IdSuffixError( 'Device ID mismatch' )
	
	# Check crc32 sum
	csum = crc32(blob[:-4])
	if (csum != cur.dwCRC):
		raise CrcSuffixError("CRC32 error")
	return current

def append_suffix(blob, usr_suffix, force=False):
	try:
		current = verify_suffix(blob, generic_suffix)
	except SignatureSuffixError as se:
		if force:
			current = None
			pass
	suffix = struct.pack(_suffix_fmt_str, usr_suffix)
	usr_suffix.dwCRC = crc32(blob+suffix[:-4])
	suffix = struct.pack(_suffix_fmt_str, usr_suffix)
	return blob+suffix

def remove_suffix(blob, usr_suffix, force=False):
	try:
		check_suffix(blob, usr_suffix)
	except SuffixError:
		if force:
			return blob[:-16]
	else:
		return blob[:-16]

class SuffixError(Exception):
	def __init__(self, msg):
		self.msg = msg
	def __str__(self):
		return repr(self.msg)

class SignatureSuffixError(SuffixError):
	def __init__(self, msg, ver):
		self.msg = msg
		self.ver = ver
	def __str__(self):
		return repr(self.msg) + '\tversion: %s' % repr(self.ver)
		
class IdSuffixError(SuffixError):
	pass

class CrcSuffixError(SuffixError):
	pass

if __name__ == '__main__':
	from sys import exit
	import argparse
	import os.path
	
	def xint(s):
		return int(x, 0)

	parser = argparse.ArgumentParser( description='Check, add or delete a DFU 1.1 file suffix.',
									  epilog='''Default action is to verify the suffix.
												Default output filename is the input filename with
												".dfu" appended or removed depending on action''')
	action = parser.add_mutually_exclusive_group()
	action.add_argument( '--verify', '--chk', help='Verify dfu suffix (Default).', dest='action', action='store_const', const=verify_suffix)
	action.add_argument( '--append', '--add', help='Append dfu suffix.', dest='action', action='store_const', const=append_suffix)
	action.add_argument( '--remove', '--del', help='Remove dfu suffix.', dest='action', action='store_const', const=remove_suffix)
	action.add_argument( '--report', '--get', help='Report current dfu suffix', dest='action', action='store_const', const=report_suffix)
	parser.add_argument( 'vid', help='The Vendor ID to use.',  action='store', type=xint, nargs='?', default=0xFFFF );
	parser.add_argument( 'pid', help='The Product ID to use.', action='store', type=xint, nargs='?', default=0xFFFF );
	parser.add_argument( 'did', help='The Device version to use.',  action='store', type=xint, nargs='?', default=0xFFFF );
	parser.add_argument( 'file',           help='Input file.',  type=argparse.FileType('rb'), nargs=1 );
	parser.add_argument( '-o', '--output', help='Output file.', type=argparse.FileType('wb'), dest='outfile', nargs=1, metavar='file.dfu' )
	parser.add_argument( '-f', '--force',  help='Forcefully try to execute given command. May result in unusable files.', action='store_true', default=False )

	args = parser.parse_args()
	(rootname, ext) = os.path.splitext( args.infile.name )

	user_suffix = Suffix._make( args.did, args.pid, args.vid, 0x0100, 'DFU', 16, 0 )
	
	try:
		blob = infile.read()
	except FileNotFoundException:
		print 'File "%file" not found\n' % args.infile
		sys.exit(1)
	infile.close()
	
	try:
		blob = args.action(blob, user_suffix, args.force)
	except SuffixError as se:
			print 'Check failed! %s\n' % se
			exit(1)

	if args.outfile is None:
		args.outfile = open( rootname + '.dfu', 'wb' )
	args.outfile.write(blob)
	exit(0)
