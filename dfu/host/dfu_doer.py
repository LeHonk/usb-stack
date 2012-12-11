#!/usr/bin/env python2.7
import usb.core
import usb.util
import dfu_suffix

class runtime_target( object ):
    def __init__( self, vid_, pid_, did_, ignore_iface_=False ):
        self._vid = vid_
        self._pid = pid_
        self._did = did_
        self._ignore_iface = ignore_iface_
        
    def __call__( self, device ):
        if ( device.idVendor==self._vid or self._vid==0xFFFF ):
            if ( device.idProduct==self._pid or self._pid==0xFFFF ):
                if ( device.bcdDevice==self._did or self._did==0xFFFF ):
                    if not self._ignore_iface:
                        for cfg in device:
                            if usb.util.find_descriptor( cfg, bInterfaceClass=0xFE, 
                                                              bInterfaceSubclass=0x01, 
                                                              bInterfaceProtocol=0x02 ) is not None:
                                return True
        return False

class DfuDevice():

    DFU_DETACH    = 0
    DFU_DNLOAD    = 1
    DFU_UPLOAD    = 2
    DFU_GETSTATUS = 3
    DFU_CLRSTATUS = 4
    DFU_GETSTATE  = 5
    DFU_ABORT     = 6

    # Device states
    appIDLE                = 0
    appDETACH              = 1
    dfuIDLE                = 2
    dfuDNLOAD_SYNC         = 3
    dfuDNBUSY              = 4
    dfuDNLOAD_IDLE         = 5
    dfuMANIFEST_SYNC       = 6
    dfuMANIFEST            = 7
    dfuMANIFEST_WAIT_RESET = 8
    dfuUPLOAD_IDLE         = 9
    dfuERROR               = 10
    
    # Statuses
    OK = 0x00
    errTarget       = 0x01
    errFILE         = 0x02
    errWRITE        = 0x03
    errERASE        = 0x04
    errCHECK_ERASED = 0x05
    errPROG         = 0x06
    errVERIFY       = 0x07
    errADDRESS      = 0x08
    errNOTDONE      = 0x09
    errFIRMWARE     = 0x0A
    errVENDOR       = 0x0B
    errUSBR         = 0x0C
    errPOR          = 0x0D
    errUNKNOWN      = 0x0E
    errSTALLEDPKT   = 0x0F
    
    default_status_strings = { 0x00: 'OK: No error condition is present.',
                               0x01: 'errTARGET: File is not targeted for use by this device.',
                               0x02: 'errFILE: File is for this device but fails some vendor-specific verification test.',
                               0x03: 'errWRITE: Device is unable to write memory.',
                               0x04: 'errERASE: Memory erase function failed.',
                               0x05: 'errCHECK_ERASED: Memory erase check failed.',
                               0x06: 'errPROG: Program memory function failed.',
                               0x07: 'errVERIFY: Programmed memory failed verification.',
                               0x08: 'errADDRESS: Cannot program memory due to received address that is out of range.',
                               0x09: 'errNOTDONE: Received DFU_DNLOAD with wLength = 0, but device does not think it has all of the data yet.',
                               0x0A: 'errFIRMWARE: Device’s firmware is corrupt. It cannot return to run-time (non-DFU) operations.',
                               0x0B: 'errVENDOR: iString indicates a vendor-specific error.',
                               0x0C: 'errUSBR: Device detected unexpected USB reset signaling.',
                               0x0D: 'errPOR: Device detected unexpected power on reset.',
                               0x0E: 'errUNKNOWN: Something went wrong, but the device does not know what it was.',
                               0x0F: 'errSTALLEDPKT: Device stalled an unexpected request.'}

    def __init__( self, suffix, altsetting, force=False ):
        self.suffix = suffix
        self.altsetting = altsetting
        targets = usb.core.find(find_all=1, custom_match=runtime_target( suffix , force))
        if len(targets) == 0:
            raise 'No device matching this firmware found'
        if len(targets) > 1:
            raise 'Multiple devices matching this firmware found.'
        self.device = targets[0]
        self.rtm_iface = usb.util.find_descriptor( self.device, bDescriptorType = 0x04, bInterfaceClass = 0xFE, bInterfaceSubClass = 0x01, bInterfaceProtocol = 0x01 )
        if rtm_iface is None:
            if force:
                self.rtm_iface = device[0][(0,0)]
                # Fake functional descriptor, no capabilities, 1 sec DetachTimeOut, 8 bytes TransferSize
                self.functional = [ 0x09, 0x21, 0x00, 0xe8, 0x03, 0x08, 0x00, 0x10, 0x01 ]
            else:
                raise 'No DFU runtime interface found'
        self.mode = 'runtime'
        self.dfu_iface = None
        self.functional = usb.util.find_descriptor( self.rtm_iface, bDescriptorType = 0x21 )
        self.bitWillDetach = (self.functional[2] & 0x08) == 0x08
        self.bitManifestationTolerant = (self.functional[2] & 0x04) == 0x04
        self.bitCanUpload = (self.functional[2] & 0x02) == 0x02
        self.bitCanDnload = (self.functional[2] & 0x01) == 0x01
        self.wDetachTimeOut = self.functional[3] | self.functional[4] << 8
        self.wTransferSize = self.functional[5] | self.functional[6] << 8
        self.target_lock = threading.Lock()

    def switch_to_dfumode( self, force=False ):
        def dfumode_target(device):
            config = device[0]
            iface  = config[(0,0)]
            if ( iface.bInterfaceClass==0xFE and
                 iface.bInterfaceSubClass==0x01 and
                 iface.bProtocol==0x02 ):
                return True
            return False

        device.ctrl_transfer(0b00100001, 0, self.wDetachTimeOut/2, self.rtm_iface.bInterfaceNumber )

        if not self.bitWillDetach:
            target.reset();
        else:
            sleep(self.wDetachTimeOut)
        self.device = usb.core.find( custom_match=dfumode_target )
    
        if self.device is None:
            raise "Couldn't find target device after dfu_detach, you loose"

        self.mode = 'dfumode'
        self.dfu_iface = self.device[0][(0, 0)]
        try:
            self.device.set_interface_altsetting( dfu_target_iface, self.altsetting )
        except usb.core.USBError:
            raise 'Error setting alternative interface %(alt)d' % self.altsetting
        self.dfu_iface = self.device[0][(0, self.altsetting)]
        if self.dfu_iface.iInterface != 0:
            print 'Using alternative target interface %d (%s)' % ( self.altsetting, 
                usb.util.get_string( self.device, self.dfu_iface.iInterface ))

        # Functional descriptor should be the same, but perhaps we faked one previously
        self.functional = usb.util.find_descriptor( self.rtm_iface, bDescriptorType = 0x21 )
        self.bitWillDetach = (self.functional[2] & 0x08) == 0x08
        self.bitManifestationTolerant = (self.functional[2] & 0x04) == 0x04
        self.bitCanUpload = (self.functional[2] & 0x02) == 0x02
        self.bitCanDnload = (self.functional[2] & 0x01) == 0x01
        self.wDetachTimeOut = self.functional[3] | self.functional[4] << 8
        self.wTransferSize = self.functional[5] | self.functional[6] << 8
        self.target_lock.release()
        self.get_status()

    def get_status( self ):
        with self.target_lock:
            self.status = self.device.ctrl_transfer( 0b10100001, DFU_GETSTATUS, 0, self.dfu_iface.bInterfaceNumber, 6 )
        ms = self.status[1] | self.status[2]<<8 | self.status[3]<<16
        t = thread.Timer( ms/1000.0, self.target_lock.release() )
        t.start()
        return self.status[0]

    def get_status_string( self ):
        if self.status == None: self.get_status()
        if self.status[5] != 0:
            return usb.util.get_string( device, 256, status[5] )
        else:
            return default_status_strings[status[0]]

    def clr_status( self ):
        self.device.ctrl_transfer( 0b00100001, DFU_CLRSTATUS, 0, self.dfu_iface.bInterfaceNumber )

    def abort( self ):
        self.device.ctrl_transfer( 0b00100001, DFU_ABORT, 0, self.dfu_iface.bInterfaceNumber )

    def get_state( self ):
        reply = device.ctrl_transfer( 0b10100001, DFU_GETSTATE, 0, self.dfu_iface.bInterfaceNumber, 1 )
        return reply[0]

    def upload( self ):
        if not bitCanUpload:
            raise 'Target not capable of upload'
        block_counter = itertools.cycle(xrange(65536))
        while True:
            chunk = self.device.ctrl_transfer(  0b10100001, DFU_UPLOAD, block_counter.next(), self.dfu_iface.bInterfaceNumber, self.wTransferSize )
            blob = itertools.chain(blob, chunk)
            if len(chunk) < self.wTransferSize:
                break
        return append_suffix( blob, self.suffix )

    def dnload( self, blob, force=False ):
        def chop( it, n ):
            while True:
                bit = itertools.islice(it, n)
                yield itertools.chain([bit.next()], bit)

        if not self.bitCanDnload:
            raise 'Target not capable of download'
        blob = remove_suffix( blob, self.suffix, force )
        block_counter = itertools.cycle(xrange(65536))
        for chunk in chop(blob, self.wTransferSize):
            with self.target_lock:
                self.device.ctrl_transfer( 0b00100001, DFU_DNLOAD, block_counter.next(), self.dfu_iface.bInterfaceNumber, chunk )
                self.target_lock.release()
            get_status()
            if self.status[0] != OK:
                raise self.get_status_string()
        else:
            # ZLP
            self.device.ctrl_transfer( 0b00100001, DFU_DNLOAD, block_counter.next(), self.dfu_iface.bInterfaceNumber, 0 )
            self.get_status()
            if self.status[0] != OK:
                raise self.get_status_string()
        # Manifestation

    def reset( self ):
        self.device.reset()
        # TODO: Handling of expected error == New firmeware running
        try:
            self.get_status()
        except usb.core.Error:
            pass
        else:
            if self.status[0] != OK:
                raise 'Device stuck in DFU mode'

def upload( suffix, force, altsetting, dfufile, *arguments ):
    try:
        file = open( dfufile, 'wb' )
        target = DfuTarget( suffix, altsetting, force )
        target.swich_to_dfumode()
        firmware = target.upload()
        file.write( firmware )
        target.reset()
        file.close()
    except IOError as ioe:
        print ioe
        sys.exit(1)
    except e:
        print e
        sys.exit(2)

def dnload( suffix, force, altsetting, dfufile, *arguments ):
    try:
        file = open( dfufile, 'rb' )
        firmware = file.read()
        file.close()
        target = DfuTarget( suffix, altsetting, force )
        target.dnload( firmware )
        target.reset()
    except e:
        # TODO: Better errorhandling
        raise e
        
def verify( suffix, force, altsetting, dfufile, *arguments ):
    file = open( dfufile, 'rb' )
    firmware_file = file.read()
    target = DfuDevice( suffix, altsetting, force )
    target.switch_to_dfu_mode()
    firmware_target = target.upload()
    for address, (f,t) in enumerate(zip(firmware_file, firmware_target)):
        if f != t:
            print 'Target differs from file at address 0x%08X' % address
            return False    
    print 'Verification OK'
    return True

def detect( vid=0xFFFF, pid=0xFFFF, did=0xFFFF, force=False, dfufile=None, **arguments ):
    if dfufile:
        blob = dfufile.read()
        suffix = dfu_suffix.get_suffix( blob )
        vid = suffix.idVendor
        pid = suffix.idProduct
        did = suffix.bcdDevice

    if not vid: vid = 0xFFFF
    if not pid: pid = 0xFFFF
    if not did: did = 0xFFFF

    targets = usb.core.find(find_all=1, custom_match=runtime_target(vid, pid, did, force) )
    if len(targets) == 0:
        print 'No target found'
        return False
    elif len(targets) == 1:
        print 'Matching target found'
        # TODO: Enumerate alternative interfaces
        return True
    else:
        print 'Multiple matching targets found.'
        if suffix.idVendor == 0xFFFF:
            print 'Try specifying a narrower selection with --vid'
        elif suffix.idProduct == 0xFFFF:
            print 'Try specifying a narrower selection with --pid'
        elif suffix.bcdDevice == 0xFFFF:
            print 'Try specifying a narrower selection with --did'
        else:
            print 'Try removing non-wanted devices or rewrite this program to take more identifiers into account'
        print 'idVendor idProduct bcdDevice sProduct sManufaturer'
        print '-' * 40
        for dev in targets:
            print '0x%(idVendor)04X 0x%(idProduct)04X 0x%(bcdDevice)04X' % dev, \
                  usb.util.get_string( dev, 25, dev.iProduct ), \
                  usb.util.get_string( dev, 25, dev.iManufaturer )
        return False

if __name__ == '__main__':
    import sys
    import argparse
    import os.path
    
    def xint(s):
        return int(s, 0)

    parser = argparse.ArgumentParser()
    action = parser.add_subparsers( title='Action' )
    upload_parser = action.add_parser( 'upload', help='Upload (new) firmware to device.' )
    upload_parser.set_defaults( action=upload )
    upload_parser.add_argument( 'dfufile',  help='Firmware file with DFU suffix.', type=argparse.FileType('rb', 0) )
    dnload_parser = action.add_parser( 'dnload', help='Download firmware from device.' )
    dnload_parser.set_defaults( action=dnload )
    dnload_parser.add_argument( 'dfufile',  help='File to save as.', type=argparse.FileType('wb', 0) )
    verify_parser = action.add_parser( 'verify', help='Verify firmware on device.' )
    verify_parser.set_defaults( action=verify )
    verify_parser.add_argument( 'dfufile',  help='Firmware file with DFU suffix to verify against.', type=argparse.FileType('rb', 0) )
    detect_parser = action.add_parser( 'detect', help='Detect presence of device, no actions taken.' )
    detect_parser.set_defaults( action=detect )
    detect_parser.add_argument( 'dfufile',  help='Detect devices matching file with DFU suffix.', type=argparse.FileType('rb', 0), nargs='?' )
    parser.add_argument( '--vid',    help='idVendor when uploading or to override idVendor from file.',  type=xint )
    parser.add_argument( '--pid',    help='idProduct when uploading or to override idProduct from file.', type=xint )
    parser.add_argument( '--did',    help='bcdDevice when uploading or to override bcdDevice from file.', type=xint )
    parser.add_argument( '--alt',    help='Alternative setting, ie. Change which memory on target to reprogram', action='store', type=int, default=0 )
    parser.add_argument( '--ignore_missing_interface', help='Try DFU_DETACH even if no dfu interface was found', dest='force', action='store_true', default=False )

    args = parser.parse_args()

    sys.exit( args.action( **vars(args) ) and 0 or 1)
    
    if dfu_target_device is None:
        print "Couldn't find target device after dfu_detach, you loose"
        sys.exit(1)
        
    dfu_target_device.set_configuration()
    dfu_target_config = dfu_target_device[0]
    dfu_target_iface = dfu_target_config[(0, args.alt)]
    try:
        dfu_target.set_interface_altsetting( dfu_target_iface, args.alt )
    except usb.core.USBError:
        print 'Error setting alternative interface %(alt)d' % args
        sys.exit(1)

    if dfu_target_iface.iInterface != 0:
        print 'Using alternative target interface %d (%s)' % ( args.alt, usb.util.get_string( usb_target_device, dfu_target_iface.iInterface ))
    
    