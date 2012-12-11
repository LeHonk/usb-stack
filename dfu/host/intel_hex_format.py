#
# Josh Lifton (c) 2004
# Permission is hereby granted to use this document in any way you see fit.
#

import string
import binascii

class IntelHexFile:
    """
    A class for representing Intel hex files.
    For example,
    
    IntelHexFormat.IntelHexFile('MyHexFile.hex').toByteString()

    returns an ordered string of bytes that could then be sent
    over a serial port to a microcontroller for storage in
    flash memory.

    On the other hand,
    
    IntelHexFormat.IntelHexFile('MyHexFile.hex').toHexString()

    returns a string of hexadecimal numbers more suitable for
    being read by people.
    """

    dataRecordString = '00'
    eofRecordString = '01'
    extendedSegmentRecordString = '02'
    extendedLinearRecordString = '04'

    def __init__(self, fileName):
        self.numberOfBytes = 0
        self.fileName = fileName
        file = open(self.fileName,'r')
        self.records = []
        line=file.readline()
        while line:
            self.records.append(IntelHexFileRecord(line))
            if self.records[-1].startAddress + self.records[-1].recordLength > self.numberOfBytes:
                self.numberOfBytes = self.records[-1].startAddress + self.records[-1].recordLength
            line=file.readline()
        file.close()

    def toHexArray(self, size=None, startAddress=0):
        "This method returns an array of hex strings as they would appear in memory."

        if size is None:
            size = self.numberOfBytes
        hexArray = ['ff']*self.numberOfBytes
        if size>0:
            for record in self.records:
                if record.recordType == self.dataRecordString:
                    for i in range(record.recordLength):
                        hexArray[record.startAddress + i] = record.dataBytes[2*i:2*i+2]
        else:
            print '   Warning: size is zero or negative.'
        return hexArray[startAddress:startAddress+size]

    def toHexString(self, size=None, startAddress=0):
        "This method returns a single hex string as they would appear in memory."

        if size is None:
            size = self.numberOfBytes
        hexArray = self.toHexArray(size, startAddress)
        hexString = ''
        for hexByte in hexArray:
            hexString = hexString + hexByte
        return hexString

    def toByteString(self, size=None, startAddress=0):
        "This method returns a single string of bytes as they would appear in memory."

        if size is None:
            size = self.numberOfBytes
        hexString = self.toHexString(size, startAddress)
        return binascii.a2b_hex(hexString)
        
class IntelHexFileRecord:
    "A class for representing a single record (line) in an Intel hex file."
    
    def __init__(self, line):
        if line[0] == ':':
            self.recordLength = int(line[1:3],16)
            self.startAddress = int(line[3:7],16)
            self.recordType = line[7:9]
            self.dataBytes = line[9:9+2*self.recordLength]
            self.checksum = int(line[9+2*self.recordLength:2+9+2*self.recordLength],16)
        else:
            print '   ERROR.  The following record is malformed:'
            print line


