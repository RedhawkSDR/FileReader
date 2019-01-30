#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file distributed with this 
# source distribution.
# 
# This file is part of REDHAWK Basic Components FileReader.
# 
# REDHAWK Basic Components FileReader is free software: you can redistribute it and/or modify it under the terms of 
# the GNU Lesser General Public License as published by the Free Software Foundation, either
# version 3 of the License, or (at your option) any later version.
# 
# REDHAWK Basic Components FileReader is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
# PURPOSE.  See the GNU Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License along with this
# program.  If not, see http://www.gnu.org/licenses/.
#

import ossie.utils.testing
from ossie.cf import CF
import os, sys, subprocess
from omniORB import any
import time
from ossie.utils import sb
import struct
import random
from math import isnan
from ossie.properties import props_from_dict, props_to_dict
from ossie.utils.bluefile import bluefile, bluefile_helpers
from bulkio.bulkioInterfaces import BULKIO__POA
from bulkio.sri import create as createSri
from bulkio.timestamp import create as createTs
import bulkio

DEBUG_LEVEL = 3

STRING_MAP = {'octet':'B',
              'char':'b',
              'short':'h',
              'ushort':'H',
              'long':'i',
              'ulong':'I',
              'float':'f',
              'double':'d'}

BYTE_MAP = {'octet':1,
            'char':1,
            'short':2,
            'ushort':2,
            'long':4,
            'ulong':4,
            'float':4,
            'double':8}

def toStr(data, dataType):
    """pack data in to a string
    """
    return struct.pack("%s%s" % (len(data), STRING_MAP[dataType]), *data)

def fromStr(dataStr, dataType):
    """unpack data from a string
    """
    return struct.unpack("%s%s" % (long(len(dataStr)/BYTE_MAP[dataType]), STRING_MAP[dataType]), dataStr)

def flip(dataStr, dataType):
    """given data packed into a string - reverse bytes for a given word length and returned the byte-flipped 
     string
    """
    numBytes = BYTE_MAP[dataType]
    out = ""
    for i in xrange(len(dataStr) / numBytes):
        l = list(dataStr[numBytes * i:numBytes * (i + 1)])
        l.reverse()
        out += ''.join(l)
    
    return out

def swap(data, dataType):
    dataStr = toStr(data, dataType)
    strFlip = flip(dataStr, dataType)
    return fromStr(strFlip, dataType)

class ResourceTests(ossie.utils.testing.ScaComponentTestCase):
    """Test for all resource implementations in FileReader"""

    def testScaBasicBehavior(self):
        #######################################################################
        # Launch the resource with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)

        #######################################################################
        # Verify the basic state of the resource
        self.assertNotEqual(self.comp, None)
        self.assertEqual(self.comp.ref._non_existent(), False)

        self.assertEqual(self.comp.ref._is_a("IDL:CF/Resource:1.0"), True)

        #######################################################################
        # Validate that query returns all expected parameters
        # Query of '[]' should return the following set of properties
        expectedProps = []
        expectedProps.extend(self.getPropertySet(kinds=("configure", "execparam"), modes=("readwrite", "readonly"), includeNil=True))
        expectedProps.extend(self.getPropertySet(kinds=("allocate",), action="external", includeNil=True))
        props = self.comp.query([])
        props = dict((x.id, any.from_any(x.value)) for x in props)
        # Query may return more than expected, but not less
        for expectedProp in expectedProps:
            self.assertEquals(props.has_key(expectedProp.id), True)

        #######################################################################
        # Verify that all expected ports are available
        for port in self.scd.get_componentfeatures().get_ports().get_uses():
            port_obj = self.comp.getPort(str(port.get_usesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a("IDL:CF/Port:1.0"),  True)

        for port in self.scd.get_componentfeatures().get_ports().get_provides():
            port_obj = self.comp.getPort(str(port.get_providesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a(port.get_repid()),  True)

        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp.start()
        self.comp.stop()

        #######################################################################
        # Simulate regular resource shutdown
        self.comp.releaseObject()


    # ##############
    # HELPER METHODS
    # ##############

    def setupWithDataFile(self, file):
        #Create Test Data File if it doesn't exist
        if not os.path.isfile(file):
            with open(file, 'wb') as dataIn:
                dataIn.write(os.urandom(1024))
        
        #Read in Data from Test File
        size = os.path.getsize(file)
        with open (file, 'rb') as dataIn:
            data = list(struct.unpack('b'*size, dataIn.read(size)))
            
        #Create Components and Connections
        self.comp = sb.launch('../FileReader.spd.xml')
        self.comp.source_uri = file
        self.sink = sb.DataSink()
        return data

    def startFlow(self):
        sb.start()
        self.comp.playback_state = 'PLAY'
        time.sleep(2)
        sb.stop()
        return self.sink.getData()

    def tearDownFlow(self, dataFileIn=None):
        if self.comp: self.comp.releaseObject()
        if self.sink: self.sink.releaseObject()

        self.comp = None
        self.sink = None

        if (dataFileIn): os.remove(dataFileIn)

    def setupBasicSriTest(self, dataFileIn, portName, fileFormat, configureFunc=None):
        print "\n**" + self._testMethodName
        
        #Define test files
        data = self.setupWithDataFile(dataFileIn)
        
        #Connect
        self.comp.connect(self.sink, usesPortName=portName)
        
        #Configure fr for test
        self.comp.file_format = fileFormat
        if hasattr(configureFunc, '__call__'): configureFunc()
       
        #Execute the fileread
        readData = self.startFlow()

    def assertSRIOutputMode(self, expectedMode):
        #Check that the input and output files are the same          
        try:
            self.assertEqual(self.sink.sri().mode, expectedMode)
        except self.failureException as e:
            self.comp.releaseObject()
            self.sink.releaseObject()
            raise e


    # ##############
    #     TESTS
    # ##############
    
    def testCharPort(self):
        print "\n**" + self._testMethodName
        
        #Define test files
        dataFileIn = './data.in'
        data = self.setupWithDataFile(dataFileIn)
        self.comp.file_format = 'CHAR'
        self.comp.connect(self.sink, usesPortName='dataChar_out')
        readData = self.startFlow()
       
        #Check that the input and output files are the same          
        try:
            self.assertEqual(data, readData)
        except self.failureException as e:
            comp.releaseObject()
            sink.releaseObject()
            os.remove(dataFileIn)
            raise e
        
        #Release the components and remove the generated files
        self.tearDownFlow(dataFileIn)
        
        print "........ PASSED\n"
        return

    def testCharPortSriScalar(self):
        dataFileIn = './data.in'
        self.setupBasicSriTest(dataFileIn, 'dataChar_out', 'CHAR')
        self.assertSRIOutputMode(0)
        self.tearDownFlow(dataFileIn)
        print "........ PASSED\n"
    
    def testCharPortSriComplex(self):
        dataFileIn = './data.in'
        self.setupBasicSriTest(dataFileIn, 'dataChar_out', 'COMPLEX_CHAR')
        self.assertSRIOutputMode(1)
        self.tearDownFlow(dataFileIn)
        print "........ PASSED\n"
    
    def testOctetPort(self):
        #######################################################################
        # Test OCTET Functionality
        print "\n**TESTING OCTET PORT"
        
        #Define test files
        dataFileIn = './data.in'
        
        #Create Test Data File if it doesn't exist
        if not os.path.isfile(dataFileIn):
            with open(dataFileIn, 'wb') as dataIn:
                dataIn.write(os.urandom(1024))
        
        #Read in Data from Test File
        size = os.path.getsize(dataFileIn)
        with open (dataFileIn, 'rb') as dataIn:
            data = list(struct.unpack('c'*size, dataIn.read(size)))
            
        #Create Components and Connections
        comp = sb.launch('../FileReader.spd.xml')
        comp.source_uri = dataFileIn
        comp.file_format = 'OCTET'
        
        sink = sb.DataSink()
        comp.connect(sink, usesPortName='dataOctet_out')
        
        #Start Components & Push Data
        sb.start()
        comp.playback_state = 'PLAY'
        time.sleep(2)
        readData = sink.getData()
        sb.stop()
        
        #Check that the input and output files are the same          
        try:
            self.assertEqual(data, readData)
        except self.failureException as e:
            comp.releaseObject()
            sink.releaseObject()
            os.remove(dataFileIn)
            raise e
        
        #Release the components and remove the generated files
        comp.releaseObject()
        sink.releaseObject()
        os.remove(dataFileIn)
        
        print "........ PASSED\n"
        return
    
    def testOctetPortSriScalar(self):
        dataFileIn = './data.in'
        self.setupBasicSriTest(dataFileIn, 'dataOctet_out', 'OCTET')
        self.assertSRIOutputMode(0)
        self.tearDownFlow(dataFileIn)
        print "........ PASSED\n"
    
    def testOctetPortSriComplex(self):
        dataFileIn = './data.in'
        self.setupBasicSriTest(dataFileIn, 'dataOctet_out', 'COMPLEX_OCTET')
        self.assertSRIOutputMode(1)
        self.tearDownFlow(dataFileIn)
        print "........ PASSED\n"
    
    def testBlueShortPort(self):
        self.blueShortPortTests()
        
    def testBlueShortPortLittle(self):
        self.blueShortPortTests(outputOrder="little_endian")
        
    def testBlueShortPortBig(self):
        self.blueShortPortTests(outputOrder="big_endian")
        
    def blueShortPortTests(self, outputOrder="host_order"):
        #######################################################################
        # Test Bluefile SHORT Functionality
        print "\n**TESTING BLUEFILE + SHORT PORT + input(host)=%s_endian + output=%s"%(sys.byteorder,outputOrder)
        inputEndian = sys.byteorder+'_endian'
        
        #Define test files
        dataFileIn = './bluefile.in'
        
        #Create Test Data File if it doesn't exist
        if not os.path.isfile(dataFileIn):
            tmpSink = bluefile_helpers.BlueFileWriter(dataFileIn, BULKIO__POA.dataShort)
            tmpSink.start()
            tmpSri = createSri('bluefileShort', 5000)
            tmpSri.keywords = props_from_dict({'TEST_KW':1234})
            tmpSink.pushSRI(tmpSri)
            tmpTs = createTs()
            tmpSink.pushPacket(range(1024), tmpTs, True, 'bluefileShort')
            
        #Read in Data from Test File
        hdr, d = bluefile.read(dataFileIn, dict)
        data = list(d)
        keywords = hdr['ext_header']
        
        outputEndian = outputOrder
        if outputEndian == "host_order":
            outputEndian = sys.byteorder+'_endian'
        
        if inputEndian != outputEndian:
            data = list(struct.unpack('<'+'h'*len(data), struct.pack('>'+'h'*len(data), *data)))
            
        #Create Components and Connections
        comp = sb.launch('../FileReader.spd.xml')
        comp.source_uri = dataFileIn
        comp.output_bulkio_byte_order= outputOrder
        comp.file_format = 'BLUEFILE'
        
        sink = sb.DataSink()
        comp.connect(sink, usesPortName='dataShort_out')
        
        #Start Components & Push Data
        sb.start()
        comp.playback_state = 'PLAY'
        time.sleep(2)
        readData = sink.getData()
        readKeywords = props_to_dict(sink.sri().keywords)
        sb.stop()
        
        #Check that the input and output files are the same          
        try:
            self.assertEqual(data, readData)
        except self.failureException as e:
            comp.releaseObject()
            sink.releaseObject()
            os.remove(dataFileIn)
            raise e
        
        #Check that the keywords are the same
        try:
            self.assertEqual(keywords, readKeywords)
        except self.failureException as e:
            comp.releaseObject()
            sink.releaseObject()
            os.remove(dataFileIn)
            raise e
        
        #Release the components and remove the generated files
        comp.releaseObject()
        sink.releaseObject()
        os.remove(dataFileIn)
        
        print "........ PASSED\n"
        return
    
    def testBlueShortPortSwapped(self):
        self.blueShortPortSwappedTests()
        
    def testBlueShortPortSwappedLittle(self):
        self.blueShortPortSwappedTests(outputOrder="little_endian")
        
    def testBlueShortPortSwappedBig(self):
        self.blueShortPortSwappedTests(outputOrder="big_endian")
    
    def blueShortPortSwappedTests(self, outputOrder= "host_order"):
        #######################################################################
        # Test Bluefile Swapped SHORT Functionality
        inputOrder = 'big' if sys.byteorder=='little' else 'little'
        print "\n**TESTING BLUEFILE Swapped + SHORT PORT + input(reversed host)=%s_endian + output=%s"%(inputOrder,outputOrder)
        inputEndian = inputOrder+'_endian'
        
        #Define test files
        dataFileIn = './bluefile.in'
        dataFileInSwap = './bluefile.in.swap'
        
        #Create Test Data File if it doesn't exist
        if not os.path.isfile(dataFileIn):
            tmpSink = bluefile_helpers.BlueFileWriter(dataFileIn, BULKIO__POA.dataShort)
            tmpSink.start()
            tmpSri = createSri('bluefileShortSwapped', 5000)
            #kwVal = 1234
            #kwSwap = swap([kwVal], 'long')[0]
            #tmpSri.keywords = props_from_dict({'TEST_KW':kwSwap})
            tmpSri.keywords = props_from_dict({'TEST_KW':1234})
            tmpSink.pushSRI(tmpSri)
            tmpTs = createTs()
            #tmpSink.pushPacket(swap(range(1024),'short'), tmpTs, True, 'bluefileShortSwapped')
            tmpSink.pushPacket(range(1024), tmpTs, True, 'bluefileShortSwapped')
            
        #Read in Data from Test File, modify header, and rewrite
        hdr, d = bluefile.read(dataFileIn, dict)
        hdr['file_name'] = dataFileInSwap
        if hdr['head_rep'] == 'EEEI':
            hdr['head_rep'] = 'IEEE'
        else:
            hdr['head_rep'] = 'EEEI'
        if hdr['data_rep'] == 'EEEI':
            hdr['data_rep'] = 'IEEE'
        else:
            hdr['data_rep'] = 'EEEI'
        bluefile.write(dataFileInSwap, hdr, d)
            
        #Read in Data from Swapped Test File
        hdr, d = bluefile.read(dataFileInSwap, dict)
        keywords = hdr['ext_header']
        
        outputEndian = outputOrder
        if outputEndian == "host_order":
            outputEndian = sys.byteorder+'_endian'
        
        data = list(d) # this is the data in host byte order
        if (sys.byteorder+'_endian') != outputEndian:
            data = list(struct.unpack('<'+'h'*len(data), struct.pack('>'+'h'*len(data), *data)))
            
        #Create Components and Connections
        comp = sb.launch('../FileReader.spd.xml')
        comp.source_uri = dataFileInSwap
        comp.output_bulkio_byte_order= outputOrder
        comp.file_format = 'BLUEFILE'
        
        sink = sb.DataSink()
        comp.connect(sink, usesPortName='dataShort_out')
        
        #Start Components & Push Data
        sb.start()
        comp.playback_state = 'PLAY'
        time.sleep(2)
        readData = sink.getData()
        readKeywords = props_to_dict(sink.sri().keywords)
        sb.stop()
        
        #Check that the input and output files are the same          
        try:
            self.assertEqual(data, readData)
        except self.failureException as e:
            comp.releaseObject()
            sink.releaseObject()
            os.remove(dataFileIn)
            os.remove(dataFileInSwap)
            raise e
        
        #Check that the keywords are the same
        try:
            self.assertEqual(keywords, readKeywords)
        except self.failureException as e:
            comp.releaseObject()
            sink.releaseObject()
            os.remove(dataFileIn)
            os.remove(dataFileInSwap)
            raise e
        
        #Release the components and remove the generated files
        comp.releaseObject()
        sink.releaseObject()
        os.remove(dataFileIn)
        os.remove(dataFileInSwap)
        
        print "........ PASSED\n"
        return
    
    def testShortPort(self):
        # Input:  little
        # Output: host
        return self.ShortPort()
    
    def testShortPortOutputBigEndian(self):
        # Input:  little
        # Output: big
        return self.ShortPort(outputOrder="big_endian")    
    
    def testShortPortOutputLittleEndian(self):
        # Input:  little
        # Output: little
        return self.ShortPort(outputOrder="little_endian")      
    
    def testShortPortBigFileOutputHost(self):
        # Input:  big
        # Output: host
        return self.ShortPort(inputFileEndian="big_endian")   

    def testShortPortBigFileOutputBig(self):
        # Input:  big
        # Output: big
        return self.ShortPort(inputFileEndian="big_endian",outputOrder="big_endian")    

    def testShortPortBigFileOutputLittle(self):
        # Input:  big
        # Output: little
        return self.ShortPort(inputFileEndian="big_endian",outputOrder="little_endian")
    
    def ShortPort(self,inputFileEndian="little_endian", outputOrder= "host_order"):
        #######################################################################
        # Test SHORT Functionality
        print "\n**TESTING SHORT PORT"
        
        #Define test files
        dataFileIn = './data.in'
        
        #Create Test Data File if it doesn't exist
        if not os.path.isfile(dataFileIn):
            with open(dataFileIn, 'wb') as dataIn:
                myData = random.sample(range(2000),1000)
                if inputFileEndian=="little_endian":
                    dataIn.write(struct.pack('<'+'h'*(1000), *myData))
                elif inputFileEndian=="big_endian":
                    dataIn.write(struct.pack('>'+'h'*(1000), *myData))
        
        #Read in Data from Test File
        size = os.path.getsize(dataFileIn)
        data = []
        
        outputEndian = outputOrder
        if outputEndian == "host_order":
            outputEndian = sys.byteorder+'_endian'
        reverseChar = '>'
        if sys.byteorder == 'big':
            reverseChar = '<'
            
        with open (dataFileIn, 'rb') as dataIn:
            #print dataIn.read(size)
            if inputFileEndian == outputEndian:
                data = list(struct.unpack('@'+'h'*(size/2), dataIn.read(size)))
            else:
                data = list(struct.unpack(reverseChar+'h'*(size/2), dataIn.read(size)))
            
        #Create Components and Connections
        comp = sb.launch('../FileReader.spd.xml', execparams={'DEBUG_LEVEL':DEBUG_LEVEL})
        comp.source_uri = dataFileIn
        comp.output_bulkio_byte_order= outputOrder
        comp.file_format='SHORT_'+inputFileEndian.upper()
        
        sink = sb.DataSink()
        comp.connect(sink, usesPortName='dataShort_out')
        
        #Start Components & Push Data
        sb.start()
        comp.playback_state = 'PLAY'
        time.sleep(2)
        readData = sink.getData()
        sb.stop()
        
        #Check that the input and output files are the same          
        try:
            self.assertEqual(data, readData)
        except self.failureException as e:
            comp.releaseObject()
            sink.releaseObject()
            os.remove(dataFileIn)
            raise e
        
        #Release the components and remove the generated files
        comp.releaseObject()
        sink.releaseObject()
        os.remove(dataFileIn)
        
        print "........ PASSED\n"
        return
    
    def testShortPortSriScalar(self):
        dataFileIn = './data.in'
        self.setupBasicSriTest(dataFileIn, 'dataShort_out', 'SHORT_LITTLE_ENDIAN')
        self.assertSRIOutputMode(0)
        self.tearDownFlow(dataFileIn)
        print "........ PASSED\n"
    
    def testShortPortSriComplex(self):
        dataFileIn = './data.in'
        self.setupBasicSriTest(dataFileIn, 'dataShort_out', 'COMPLEX_SHORT_LITTLE_ENDIAN')
        self.assertSRIOutputMode(1)
        self.tearDownFlow(dataFileIn)
        print "........ PASSED\n"
    
    def ntestUShortPort(self):
        #######################################################################
        # Test USHORT Functionality
        print "\n**TESTING USHORT PORT"
        
        #Define test files
        dataFileIn = './data.in'
        
        #Create Test Data File if it doesn't exist
        if not os.path.isfile(dataFileIn):
            with open(dataFileIn, 'wb') as dataIn:
                dataIn.write(os.urandom(1024))
        
        #Read in Data from Test File
        size = os.path.getsize(dataFileIn)
        with open (dataFileIn, 'rb') as dataIn:
            data = list(struct.unpack('H'*(size/2), dataIn.read(size)))
            
        #Create Components and Connections
        comp = sb.launch('../FileReader.spd.xml')
        comp.source_uri = dataFileIn
        comp.file_format = 'USHORT'
        
        sink = sb.DataSink()
        comp.connect(sink, usesPortName='dataUshort_out')
        
        #Start Components & Push Data
        sb.start()
        comp.playback_state = 'PLAY'
        time.sleep(2)
        readData = sink.getData()
        sb.stop()
        
        #Check that the input and output files are the same          
        try:
            self.assertEqual(data, readData)
        except self.failureException as e:
            comp.releaseObject()
            sink.releaseObject()
            os.remove(dataFileIn)
            raise e
        
        #Release the components and remove the generated files
        comp.releaseObject()
        sink.releaseObject()
        os.remove(dataFileIn)
        
        print "........ PASSED\n"
        return

    def testMetadataContiguous(self):
        self.metadataTests(contiguous=True)

    def testMetadataNoncontiguous(self):
        self.metadataTests(contiguous=False)

    def testMetadataContiguousMultipleFiles(self):
        self.metadataTests(contiguous=True, override_uri='./metadata_multiple_files/')

    def metadataTests(self, contiguous, override_uri = None):

        #Define test files
        gap = 0.0
        if contiguous:
            dataFileIn = './metadata_contiguous/testdata.out'
        else:
            gap = 5.0
            dataFileIn = './metadata_noncontiguous/testdata.out'

        if override_uri:
            dataFileIn = override_uri

        if os.path.isdir(dataFileIn):
            fileList = sorted([os.path.join(dataFileIn,x) for x in os.listdir(dataFileIn) if not x.endswith('metadata.xml')])
        else:
            fileList = [dataFileIn]

        #Read in Data from Test Files
        data = []
        for fileIn in fileList:
            size = os.path.getsize(fileIn)
            with open (fileIn, 'rb') as dataIn:
                data += list(struct.unpack('h'*(size/2), dataIn.read(size)))
            
        #Create Components and Connections
        print "Launched Component"
        comp = sb.launch('../FileReader.spd.xml')
        #comp.log_level(CF.LogLevels.TRACE)

        #comp.advanced_properties.packet_size="10000"
        comp.advanced_properties.throttle_rate = "0"
        comp.source_uri = dataFileIn
        comp.file_format = 'SHORT_LITTLE_ENDIAN'
        comp.advanced_properties.use_metadata_file = True
        
        sink =  bulkio.InShortPort("dataShort_in")
        port = comp.getPort("dataShort_out")
        port.connectPort(sink._this(),"TestConnectionID")
        #comp.connect(sink, usesPortName='dataShort_out')
        
        #Start Components & Push Data
        sb.start()
        comp.playback_state = 'PLAY'
        time.sleep(2)

        eos = False
        packetData =[]
        while (not(eos)):
            packet= sink.getPacket() #data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed
            packetData.append(packet)
            eos = packet[2]
        
        #for pkt in packetData:
        #    print pkt[1:-3]
        #for fstat in comp.file_status:
        #    print(fstat['DCE:4baa9718-1f21-4f46-865c-aec82b00df91'])
        #Should have five packets
        self.assertEqual(len(packetData),7)
        readdata = []
        pkt_idx = 0
        #Verify Packet 1
        self.assertEqual(len(packetData[pkt_idx][0]),1000)
        self.assertAlmostEqual(packetData[pkt_idx][1].tfsec, 0.0,10)
        self.assertAlmostEqual(packetData[pkt_idx][1].twsec, 1541719801,10)
        self.assertEqual(packetData[pkt_idx][2],False)
        self.assertEqual(packetData[pkt_idx][3],"test_streamID")
        self.assertEqual(packetData[pkt_idx][4].keywords[0].id, "TEST_KW2")
        self.assertEqual(any.from_any(packetData[pkt_idx][4].keywords[0].value), "2222")
        self.assertEqual(packetData[pkt_idx][4].keywords[1].id, "TEST_KW1")
        self.assertEqual(any.from_any(packetData[pkt_idx][4].keywords[1].value), 1111)
        readdata+=packetData[pkt_idx][0]
        
        #Verify Packet 2
        pkt_idx = 1
        self.assertEqual(len(packetData[pkt_idx][0]),1000)
        self.assertAlmostEqual(packetData[pkt_idx][1].tfsec, 0.0001,10)
        self.assertAlmostEqual(packetData[pkt_idx][1].twsec, 1541719801,10)
        self.assertEqual(packetData[pkt_idx][2],False)
        self.assertEqual(packetData[pkt_idx][3],"test_streamID")
        self.assertEqual(packetData[pkt_idx][4].keywords[0].id, "TEST_KW2")
        self.assertEqual(any.from_any(packetData[pkt_idx][4].keywords[0].value), "2222")
        self.assertEqual(packetData[pkt_idx][4].keywords[1].id, "TEST_KW1")
        self.assertEqual(any.from_any(packetData[pkt_idx][4].keywords[1].value), 1111)
        readdata+=packetData[pkt_idx][0]
        
        #Verify Packet 3
        pkt_idx = 2
        self.assertEqual(len(packetData[pkt_idx][0]),1000)
        self.assertAlmostEqual(packetData[pkt_idx][1].tfsec, 0.0002,10)
        self.assertAlmostEqual(packetData[pkt_idx][1].twsec, 1541719801,10)
        self.assertEqual(packetData[pkt_idx][2],False)
        self.assertEqual(packetData[pkt_idx][3],"test_streamID")
        self.assertEqual(packetData[pkt_idx][4].keywords[0].id, "TEST_KW2")
        self.assertEqual(any.from_any(packetData[pkt_idx][4].keywords[0].value), "2222")
        self.assertEqual(packetData[pkt_idx][4].keywords[1].id, "TEST_KW1")
        self.assertEqual(any.from_any(packetData[pkt_idx][4].keywords[1].value), 1111)
        readdata+=packetData[pkt_idx][0]
        
        #Verify Packet 4
        pkt_idx = 3
        self.assertEqual(len(packetData[pkt_idx][0]),750)
        self.assertAlmostEqual(packetData[pkt_idx][1].tfsec, 0.0003,10)
        self.assertAlmostEqual(packetData[pkt_idx][1].twsec, 1541719801,10)
        self.assertEqual(packetData[pkt_idx][2],False)
        self.assertEqual(packetData[pkt_idx][3],"test_streamID")
        self.assertEqual(packetData[pkt_idx][4].keywords[0].id, "TEST_KW2")
        self.assertEqual(any.from_any(packetData[pkt_idx][4].keywords[0].value), "2222")
        self.assertEqual(packetData[pkt_idx][4].keywords[1].id, "TEST_KW1")
        self.assertEqual(any.from_any(packetData[pkt_idx][4].keywords[1].value), 1111)
        readdata+=packetData[pkt_idx][0]
        
        #Verify Packet 5
        pkt_idx = 4
        self.assertEqual(len(packetData[pkt_idx][0]),750)
        self.assertAlmostEqual(packetData[pkt_idx][1].tfsec, 0.000375,10)
        self.assertAlmostEqual(packetData[pkt_idx][1].twsec, 1541719801,10)
        self.assertEqual(packetData[pkt_idx][2],False)
        self.assertEqual(packetData[pkt_idx][3],"test_streamID")
        self.assertEqual(packetData[pkt_idx][4].keywords[0].id, "TEST_KW2")
        self.assertEqual(any.from_any(packetData[pkt_idx][4].keywords[0].value), "2222")
        self.assertEqual(packetData[pkt_idx][4].keywords[1].id, "TEST_KW1")
        self.assertEqual(any.from_any(packetData[pkt_idx][4].keywords[1].value), 1111)
        readdata+=packetData[pkt_idx][0]
        
        #Verify Packet 6
        pkt_idx = 5
        self.assertEqual(len(packetData[pkt_idx][0]),750)
        self.assertAlmostEqual(packetData[pkt_idx][1].tfsec, 0.00045,10)
        self.assertAlmostEqual(packetData[pkt_idx][1].twsec, 1541719801+gap,10)
        self.assertEqual(packetData[pkt_idx][2],False)
        self.assertEqual(packetData[pkt_idx][3],"test_streamID")
        self.assertEqual(packetData[pkt_idx][4].keywords[0].id, "TEST_KW2")
        self.assertEqual(any.from_any(packetData[pkt_idx][4].keywords[0].value), "2222")
        self.assertEqual(packetData[pkt_idx][4].keywords[1].id, "TEST_KW5")
        self.assertEqual(any.from_any(packetData[pkt_idx][4].keywords[1].value), 5555)
        readdata+=packetData[pkt_idx][0]
        
        #Verify Packet 7
        pkt_idx = 6
        self.assertEqual(len(packetData[pkt_idx][0]),750)
        self.assertAlmostEqual(packetData[pkt_idx][1].tfsec, 0.000525,10)
        self.assertAlmostEqual(packetData[pkt_idx][1].twsec, 1541719801+gap,10)
        self.assertEqual(packetData[pkt_idx][2],True)
        self.assertEqual(packetData[pkt_idx][3],"test_streamID")
        self.assertEqual(packetData[pkt_idx][4].keywords[0].id, "TEST_KW2")
        self.assertEqual(any.from_any(packetData[pkt_idx][4].keywords[0].value), "2222")
        self.assertEqual(packetData[pkt_idx][4].keywords[1].id, "TEST_KW5")
        self.assertEqual(any.from_any(packetData[pkt_idx][4].keywords[1].value), 5555)
        readdata+=packetData[pkt_idx][0]
        
        self.assertEqual(data, readdata)

        #Release the components and remove the generated files
        comp.releaseObject()
         
        print "........ PASSED\n"
        return
    
    def testMetadataTimeFilterPreserveSRI(self):
        
        dataFileIn = './metadata_contiguous/testdata.out'

        #Read in Data from Test File
        size = os.path.getsize(dataFileIn)
        with open (dataFileIn, 'rb') as dataIn:
            data = list(struct.unpack('h'*(size/2), dataIn.read(size)))
            
        #Create Components and Connections
        print "Launched Component"
        comp = sb.launch('../FileReader.spd.xml')
        #comp.log_level(CF.LogLevels.TRACE)

        #comp.advanced_properties.packet_size="10000"
        comp.advanced_properties.throttle_rate = "0"
        comp.advanced_properties.enable_time_filtering = True
        comp.advanced_properties.start_time = .00055
        comp.source_uri = dataFileIn
        comp.file_format = 'SHORT_LITTLE_ENDIAN'
        comp.advanced_properties.use_metadata_file = True
        
        sink =  bulkio.InShortPort("dataShort_in")
        port = comp.getPort("dataShort_out")
        port.connectPort(sink._this(),"TestConnectionID")
        #comp.connect(sink, usesPortName='dataShort_out')
        
        #Start Components & Push Data
        sb.start()
        comp.playback_state = 'PLAY'
        time.sleep(2)

        eos = False
        packetData =[]
        while (not(eos)):
            packet= sink.getPacket() #data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed
            packetData.append(packet)
            eos = packet[2]
        
        #for pkt in packetData:
        #    print pkt[1:-3]
        #for fstat in comp.file_status:
        #    print(fstat['DCE:4baa9718-1f21-4f46-865c-aec82b00df91'])
        #Should have five packets
        self.assertEqual(len(packetData),1)
        readdata = []
        pkt_idx = 0
        self.assertEqual(len(packetData[pkt_idx][0]),750)
        self.assertAlmostEqual(packetData[pkt_idx][1].tfsec, 0.000525,10)
        self.assertAlmostEqual(packetData[pkt_idx][1].twsec, 1541719801,10)
        self.assertEqual(packetData[pkt_idx][2],True)
        self.assertEqual(packetData[pkt_idx][3],"test_streamID")
        self.assertEqual(packetData[pkt_idx][4].keywords[0].id, "TEST_KW2")
        self.assertEqual(any.from_any(packetData[pkt_idx][4].keywords[0].value), "2222")
        self.assertEqual(packetData[pkt_idx][4].keywords[1].id, "TEST_KW5")
        self.assertEqual(any.from_any(packetData[pkt_idx][4].keywords[1].value), 5555)
        readdata+=packetData[pkt_idx][0]

        #Verify received Data
        self.assertEqual(data[5250:], readdata)

        #Release the components and remove the generated files
        comp.releaseObject()

        print "........ PASSED\n"
        return

    def testwithMetadataTimeFilteringStart(self):
        
        #Define test files
        dataFileIn = './metadata_multiple_files/testdata.out'
        
        #Read in Data from Test File
        size = os.path.getsize(dataFileIn)
        with open (dataFileIn, 'rb') as dataIn:
            data = list(struct.unpack('h'*(size/2), dataIn.read(size)))
            
        #Create Components and Connections
        print "Launched Component"
        comp = sb.launch('../FileReader.spd.xml')
        #comp.log_level(CF.LogLevels.TRACE)

        #comp.advanced_properties.packet_size="10000"
        comp.advanced_properties.throttle_rate = ""
        
        # This filtering should skip the first packet. This time is in the middle of the second packet, so we should get all of the second packet and the third packet 
        comp.advanced_properties.enable_time_filtering = True
        comp.advanced_properties.start_time = .00015
        
        comp.source_uri = dataFileIn       
        comp.file_format = 'SHORT_LITTLE_ENDIAN'
        comp.advanced_properties.use_metadata_file = True
        
        sink =  bulkio.InShortPort("dataShort_in")
        port = comp.getPort("dataShort_out")
        port.connectPort(sink._this(),"TestConnectionID")
        
        #Start Components & Push Data
        sb.start()
        comp.playback_state = 'PLAY'
        time.sleep(2)
        
        eos = False
        packetData =[]
        while (not(eos)):
            packet= sink.getPacket() #data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed
            packetData.append(packet)
            eos = packet[2]
        
        # Should have two packets with data
        # Last packet will have EOS==True
        num_pkts = 0
        for pkt in packetData:
            #print len(pkt[0]), pkt[1:-3]
            if len(pkt[0]) > 0:
                num_pkts+=1
        self.assertEqual(num_pkts,2)
        if num_pkts < len(packetData):
            self.assertEqual(len(packetData),3)
        
        readdata = []
        #Verify Packet 1
        self.assertEqual(len(packetData[0][0]),1000)
        self.assertAlmostEqual(packetData[0][1].tfsec, 0.0001,10)
        self.assertAlmostEqual(packetData[0][1].twsec, 1541719801,10)
        self.assertEqual(packetData[0][2],False)
        self.assertEqual(packetData[0][3],"test_streamID")
        self.assertEqual(packetData[0][4].keywords[0].id, "TEST_KW2")
        self.assertEqual(any.from_any(packetData[0][4].keywords[0].value), "2222")
        self.assertEqual(packetData[0][4].keywords[1].id, "TEST_KW1")
        self.assertEqual(any.from_any(packetData[0][4].keywords[1].value), 1111)
        readdata+=packetData[0][0]
        
        #Verify Packet 2
        self.assertEqual(len(packetData[1][0]),1000)
        self.assertAlmostEqual(packetData[1][1].tfsec, 0.0002,10)
        self.assertAlmostEqual(packetData[1][1].twsec, 1541719801,10)
        if num_pkts < len(packetData):
            self.assertEqual(packetData[1][2],False)
        else:
            self.assertEqual(packetData[1][2],True)
        self.assertEqual(packetData[1][3],"test_streamID")
        self.assertEqual(packetData[1][4].keywords[0].id, "TEST_KW2")
        self.assertEqual(any.from_any(packetData[1][4].keywords[0].value), "2222")
        self.assertEqual(packetData[1][4].keywords[1].id, "TEST_KW1")
        self.assertEqual(any.from_any(packetData[1][4].keywords[1].value), 1111)
        readdata+=packetData[1][0]
        
        if num_pkts < len(packetData):
            #Verify Packet 3 - empty EOS packet
            self.assertEqual(len(packetData[2][0]),0)
            self.assertEqual(packetData[2][2],True)
            self.assertEqual(packetData[2][3],"test_streamID")
        
        #Verify received Data
        self.assertEqual(data[1000:], readdata)
        
        #Release the components and remove the generated files
        comp.releaseObject()
          
        print "........ PASSED\n"
        return

    def testwithMetadataTimeFilteringEnd(self):
        
        #Define test files
        dataFileIn = './metadata_multiple_files/testdata.out'
        
        #Read in Data from Test File
        size = os.path.getsize(dataFileIn)
        with open (dataFileIn, 'rb') as dataIn:
            data = list(struct.unpack('h'*(size/2), dataIn.read(size)))
            
        #Create Components and Connections
        print "Launched Component"
        comp = sb.launch('../FileReader.spd.xml')
        #comp.log_level(CF.LogLevels.TRACE)

        #comp.advanced_properties.packet_size="10000"
        comp.advanced_properties.throttle_rate = ""

        # This filtering should skip the last packet. This time is in the middle of the second packet, so we should get all of the first and second packets
        comp.advanced_properties.enable_time_filtering = True
        comp.advanced_properties.stop_time = .00015

        comp.source_uri = dataFileIn       
        comp.file_format = 'SHORT_LITTLE_ENDIAN'
        comp.advanced_properties.use_metadata_file = True

        sink =  bulkio.InShortPort("dataShort_in")
        port = comp.getPort("dataShort_out")
        port.connectPort(sink._this(),"TestConnectionID")

        #Start Components & Push Data
        sb.start()
        comp.playback_state = 'PLAY'
        time.sleep(2)

        eos = False
        packetData =[]
        while (not(eos)):
            packet= sink.getPacket() #data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed
            packetData.append(packet)
            eos = packet[2]

        # Should have two packets with data
        # Last packet will have EOS==True
        num_pkts = 0
        for pkt in packetData:
            #print len(pkt[0]), pkt[1:-3]
            if len(pkt[0]) > 0:
                num_pkts+=1
        self.assertEqual(num_pkts,2)
        if num_pkts < len(packetData):
            self.assertEqual(len(packetData),3)

        readdata = []
        #Verify Packet 1
        self.assertEqual(len(packetData[0][0]),1000)
        self.assertAlmostEqual(packetData[0][1].tfsec, 0.0,10)
        self.assertAlmostEqual(packetData[0][1].twsec, 1541719801,10)
        self.assertEqual(packetData[0][2],False)
        self.assertEqual(packetData[0][3],"test_streamID")
        self.assertEqual(packetData[0][4].keywords[0].id, "TEST_KW2")
        self.assertEqual(any.from_any(packetData[0][4].keywords[0].value), "2222")
        self.assertEqual(packetData[0][4].keywords[1].id, "TEST_KW1")
        self.assertEqual(any.from_any(packetData[0][4].keywords[1].value), 1111)
        readdata+=packetData[0][0]

        #Verify Packet 2
        self.assertEqual(len(packetData[1][0]),1000)
        self.assertAlmostEqual(packetData[1][1].tfsec, 0.0001,10)
        self.assertAlmostEqual(packetData[1][1].twsec, 1541719801,10)
        if num_pkts < len(packetData):
            self.assertEqual(packetData[1][2],False)
        else:
            self.assertEqual(packetData[1][2],True)
        self.assertEqual(packetData[1][3],"test_streamID")
        self.assertEqual(packetData[1][4].keywords[0].id, "TEST_KW2")
        self.assertEqual(any.from_any(packetData[1][4].keywords[0].value), "2222")
        self.assertEqual(packetData[1][4].keywords[1].id, "TEST_KW1")
        self.assertEqual(any.from_any(packetData[1][4].keywords[1].value), 1111)
        readdata+=packetData[1][0]

        if num_pkts < len(packetData):
            #Verify Packet 3 - empty EOS packet
            self.assertEqual(len(packetData[2][0]),0)
            self.assertEqual(packetData[2][2],True)
            self.assertEqual(packetData[2][3],"test_streamID")

        #Verify received Data
        self.assertEqual(data[:2000], readdata)

        #Release the components and remove the generated files
        comp.releaseObject()

        print "........ PASSED\n"
        return

    def testwithMetadataTimeFilteringAll(self):
        
        #Define test files
        dataFileIn = './metadata_multiple_files/testdata.out'
        
        #Read in Data from Test File
        size = os.path.getsize(dataFileIn)
        with open (dataFileIn, 'rb') as dataIn:
            data = list(struct.unpack('h'*(size/2), dataIn.read(size)))
            
        #Create Components and Connections
        print "Launched Component"
        comp = sb.launch('../FileReader.spd.xml')
        #comp.log_level(CF.LogLevels.TRACE)

        #comp.advanced_properties.packet_size="10000"
        comp.advanced_properties.throttle_rate = ""

        # This filtering should skip the last packet. This time is in the middle of the second packet, so we should get all of the first and second packets
        comp.advanced_properties.enable_time_filtering = True
        comp.advanced_properties.start_time = -1.0
        comp.advanced_properties.stop_time = -1.0

        comp.source_uri = dataFileIn       
        comp.file_format = 'SHORT_LITTLE_ENDIAN'
        comp.advanced_properties.use_metadata_file = True

        sink =  bulkio.InShortPort("dataShort_in")
        port = comp.getPort("dataShort_out")
        port.connectPort(sink._this(),"TestConnectionID")

        #Start Components & Push Data
        sb.start()
        comp.playback_state = 'PLAY'
        time.sleep(2)

        eos = False
        packetData =[]
        while (not(eos)):
            packet= sink.getPacket() #data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed
            packetData.append(packet)
            eos = packet[2]

        # Should have two packets with data
        # Last packet will have EOS==True
        num_pkts = 0
        for pkt in packetData:
            #print len(pkt[0]), pkt[1:-3]
            if len(pkt[0]) > 0:
                num_pkts+=1
        self.assertEqual(num_pkts,3)
        if num_pkts < len(packetData):
            self.assertEqual(len(packetData),4)

        readdata = []
        #Verify Packet 1
        self.assertEqual(len(packetData[0][0]),1000)
        self.assertAlmostEqual(packetData[0][1].tfsec, 0.0,10)
        self.assertAlmostEqual(packetData[0][1].twsec, 1541719801,10)
        self.assertEqual(packetData[0][2],False)
        self.assertEqual(packetData[0][3],"test_streamID")
        self.assertEqual(packetData[0][4].keywords[0].id, "TEST_KW2")
        self.assertEqual(any.from_any(packetData[0][4].keywords[0].value), "2222")
        self.assertEqual(packetData[0][4].keywords[1].id, "TEST_KW1")
        self.assertEqual(any.from_any(packetData[0][4].keywords[1].value), 1111)
        readdata+=packetData[0][0]

        #Verify Packet 2
        self.assertEqual(len(packetData[1][0]),1000)
        self.assertAlmostEqual(packetData[1][1].tfsec, 0.0001,10)
        self.assertAlmostEqual(packetData[1][1].twsec, 1541719801,10)
        self.assertEqual(packetData[1][2],False)
        self.assertEqual(packetData[1][3],"test_streamID")
        self.assertEqual(packetData[1][4].keywords[0].id, "TEST_KW2")
        self.assertEqual(any.from_any(packetData[1][4].keywords[0].value), "2222")
        self.assertEqual(packetData[1][4].keywords[1].id, "TEST_KW1")
        self.assertEqual(any.from_any(packetData[1][4].keywords[1].value), 1111)
        readdata+=packetData[1][0]
        
        #Verify Packet 3
        self.assertEqual(len(packetData[2][0]),1000)
        self.assertAlmostEqual(packetData[2][1].tfsec, 0.0002,10)
        self.assertAlmostEqual(packetData[2][1].twsec, 1541719801,10)
        if num_pkts < len(packetData):
            self.assertEqual(packetData[2][2],False)
        else:
            self.assertEqual(packetData[2][2],True)
        self.assertEqual(packetData[2][3],"test_streamID")
        self.assertEqual(packetData[2][4].keywords[0].id, "TEST_KW2")
        self.assertEqual(any.from_any(packetData[2][4].keywords[0].value), "2222")
        self.assertEqual(packetData[2][4].keywords[1].id, "TEST_KW1")
        self.assertEqual(any.from_any(packetData[2][4].keywords[1].value), 1111)
        readdata+=packetData[2][0]

        if num_pkts < len(packetData):
            #Verify Packet 4 - empty EOS packet
            self.assertEqual(len(packetData[3][0]),0)
            self.assertEqual(packetData[3][2],True)
            self.assertEqual(packetData[3][3],"test_streamID")

        #Verify received Data
        self.assertEqual(data, readdata)

        #Release the components and remove the generated files
        comp.releaseObject()

        print "........ PASSED\n"
        return

    def testwithMetadataMultipleStreams(self):

        #Define test files
        dataFileIn = './metadata_twostreams/'

        #Read in Data from Test File
        #size = os.path.getsize(dataFileIn)
        #with open (dataFileIn, 'rb') as dataIn:
        #    data = list(struct.unpack('h'*(size/2), dataIn.read(size)))

        #Create Components and Connections
        print "Launched Component"
        comp = sb.launch('../FileReader.spd.xml')
        #comp.log_level(CF.LogLevels.TRACE)

        #comp.advanced_properties.packet_size="10000"
        comp.advanced_properties.throttle_rate = ""

        comp.source_uri = dataFileIn       
        comp.file_format = 'SHORT_LITTLE_ENDIAN'
        comp.advanced_properties.use_metadata_file = True
        
        sink =  bulkio.InShortPort("dataShort_in")
        port = comp.getPort("dataShort_out")
        port.connectPort(sink._this(),"TestConnectionID")

        #Start Components & Push Data
        sb.start()
        comp.playback_state = 'PLAY'
        time.sleep(2)

        eos = False
        packetData =[]
        count = 0 
        while (count<50):
            packet= sink.getPacket() #data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed
            if packet[0]:
              packetData.append(packet)
            count+=1

        #Validate the total number of packets and the correct streamIDs between all the packets. The verification of the contents of packets (data and metadate) are handled in other tests
        #Should get a total of 9 packets between the three data files
        self.assertEqual(len(packetData),9)

        streamID1Count = 0
        streamID2Count = 0 
        for packet in packetData:
            if packet[3]=="test_streamID":
                streamID1Count +=1
            elif packet[3]=="test_streamID2":
                streamID2Count +=1
            else:
                self.assertTrue(False, "Should Only received packets with the two known streamIDs")

        #Validate the number of packets per streamID received.
        self.assertEqual(streamID1Count, 7, "Should received 7 packets of stream ID 1, instead got %s"%streamID1Count)
        self.assertEqual(streamID2Count, 2, "Should received 2 packets of stream ID 2, instead got %s"%streamID2Count)

        #Validate that the files were read in alphabetical order
        self.assertEqual(packetData[0][3], "test_streamID2", "First packet should be from 'anotherdata.out' with stream ID 'test_streamID2'")
        self.assertEqual(len(packetData[0][0]), 1500,        "First packet should be from 'anotherdata.out' with 1500 samples")
        self.assertEqual(packetData[1][3], "test_streamID2", "Second packet should be from 'anotherdata.out' with stream ID 'test_streamID2'")
        self.assertEqual(len(packetData[1][0]), 1500,        "Second packet should be from 'anotherdata.out' with 1500 samples")
        self.assertEqual(packetData[2][3], "test_streamID",  "Third packet should be from 'testdata.out' with stream ID 'test_streamID'")
        self.assertEqual(len(packetData[2][0]), 1000,        "Third packet should be from 'testdata.out' with 1000 samples")
        self.assertEqual(packetData[3][3], "test_streamID",  "Fourth packet should be from 'testdata.out' with stream ID 'test_streamID'")
        self.assertEqual(len(packetData[3][0]), 1000,        "Fourth packet should be from 'testdata.out' with 1000 samples")
        self.assertEqual(packetData[4][3], "test_streamID",  "Fifth packet should be from 'testdata.out' with stream ID 'test_streamID'")
        self.assertEqual(len(packetData[4][0]), 1000,        "Fifth packet should be from 'testdata.out' with 1000 samples")
        self.assertEqual(packetData[5][3], "test_streamID",  "Sixth packet should be from 'testdata.out-1' with stream ID 'test_streamID'")
        self.assertEqual(len(packetData[5][0]), 750,         "Sixth packet should be from 'testdata.out-1' with 750 samples")
        self.assertEqual(packetData[6][3], "test_streamID",  "Seventh packet should be from 'testdata.out-1' with stream ID 'test_streamID'")
        self.assertEqual(len(packetData[6][0]), 750,         "Seventh packet should be from 'testdata.out-1' with 750 samples")
        self.assertEqual(packetData[7][3], "test_streamID",  "Eighth packet should be from 'testdata.out-1' with stream ID 'test_streamID'")
        self.assertEqual(len(packetData[7][0]), 750,         "Eighth packet should be from 'testdata.out-1' with 750 samples")
        self.assertEqual(packetData[8][3], "test_streamID",  "Ninth (last) packet should be from 'testdata.out-1' with stream ID 'test_streamID'")
        self.assertEqual(len(packetData[8][0]), 750,         "Ninth (last) packet should be from 'testdata.out-1' with 750 samples")

        #Release the components and remove the generated files
        comp.releaseObject()

        print "........ PASSED\n"
        return

    def testwithBadMetadataFile(self):

        #Define test files
        dataFileIn = './data.in'
        metadataFileIn = 'data.in.metadata.xml'

        #Create Test Data File if it doesn't exist
        if not os.path.isfile(dataFileIn):
            with open(dataFileIn, 'wb') as dataIn:
                dataIn.write(os.urandom(1024))
        #Define test files

        #Read in Data from Test File
        size = os.path.getsize(dataFileIn)
        with open (dataFileIn, 'rb') as dataIn:
            data = list(struct.unpack('h'*(size/2), dataIn.read(size)))

        #Create Components and Connections
        print "Launched Component"
        comp = sb.launch('../FileReader.spd.xml')

        #comp.advanced_properties.packet_size="10000"
        comp.advanced_properties.throttle_rate = ""
        comp.source_uri = dataFileIn       
        comp.file_format = 'SHORT_LITTLE_ENDIAN'
        comp.advanced_properties.use_metadata_file = True

        sink = sb.DataSink()
        comp.connect(sink, usesPortName='dataShort_out')

        #Start Components & Push Data
        sb.start()
        comp.playback_state = 'PLAY'
        time.sleep(1)

        #Confirm Error in File Status
        print "File Status"
        fileStatus = comp.file_status
        self.assertTrue(len(fileStatus)==1)
        self.assertTrue("ERROR" in fileStatus[0]['DCE:ebc0a4de-958f-4785-bbe4-03693c34f879'])
        self.assertTrue("Cannot open metadata file" in fileStatus[0]['DCE:ebc0a4de-958f-4785-bbe4-03693c34f879'])
        self.assertTrue(metadataFileIn in fileStatus[0]['DCE:ebc0a4de-958f-4785-bbe4-03693c34f879'])

        sb.stop()

        #Release the components and remove the generated files
        comp.releaseObject()
        sink.releaseObject()
        os.remove(dataFileIn)

    def testMultipleFiles(self):

        dataFileIn = './multiple_files/'
        fileList = sorted([os.path.join(dataFileIn,x) for x in os.listdir(dataFileIn)])

        #Read in Data from Test Files
        data = []
        for fileIn in fileList:
            size = os.path.getsize(fileIn)
            with open (fileIn, 'rb') as dataIn:
                data.append(list(struct.unpack('h'*(size/2), dataIn.read(size))))

        #Create Components and Connections
        print "Launched Component"
        comp = sb.launch('../FileReader.spd.xml')
        #comp.log_level(CF.LogLevels.TRACE)

        #comp.advanced_properties.packet_size="10000"
        comp.advanced_properties.throttle_rate = "0"
        comp.source_uri = dataFileIn
        comp.file_format = 'SHORT_LITTLE_ENDIAN'
        comp.advanced_properties.use_metadata_file = False

        sink =  bulkio.InShortPort("dataShort_in")
        port = comp.getPort("dataShort_out")
        port.connectPort(sink._this(),"TestConnectionID")
        #comp.connect(sink, usesPortName='dataShort_out')

        #Start Components & Push Data
        sb.start()
        comp.playback_state = 'PLAY'
        time.sleep(2)

        fileData = []
        newFile = True
        noMoreData = False
        while (not noMoreData):
            packet = sink.getPacket() #data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed
            if packet == bulkio.InShortPort.DataTransfer(None,None,None,None,None,None,None):
                noMoreData = True
            else:
                if newFile:
                    fileData.append(packet[0])
                    newFile = False
                else:
                    fileData[-1].extend(packet[0])
                if packet[2]:
                    newFile = True
        #for file in fileData:
        #    print 'NEW FILE, len =', len(file)
        #for fstat in comp.file_status:
        #    print(fstat['DCE:4baa9718-1f21-4f46-865c-aec82b00df91'])
        #Should have five packets
        self.assertEqual(len(fileData),len(data))
        for idx in xrange(len(data)):
            self.assertEqual(len(fileData[idx]),len(data[idx]))
            self.assertEqual(fileData[idx],data[idx])

        #Release the components and remove the generated files
        comp.releaseObject()

        print "........ PASSED\n"
        return

    def testUshortPortSriScalar(self):
        dataFileIn = './data.in'
        self.setupBasicSriTest(dataFileIn, 'dataUshort_out', 'USHORT_LITTLE_ENDIAN')
        self.assertSRIOutputMode(0)
        self.tearDownFlow(dataFileIn)
        print "........ PASSED\n"
    
    def testShortPortSriComplex(self):
        dataFileIn = './data.in'
        self.setupBasicSriTest(dataFileIn, 'dataUshort_out', 'COMPLEX_USHORT_LITTLE_ENDIAN')
        self.assertSRIOutputMode(1)
        self.tearDownFlow(dataFileIn)
        print "........ PASSED\n"

    def testFloatPort(self):
        return self.FloatPort()
    
    def testFloatPortOutputBigEndian(self):
        return self.FloatPort(outputOrder="big_endian")    
    
    def testFloatPortOutputLittleEndian(self):
        return self.FloatPort(outputOrder="little_endian")      
    
    def testFloatPortBigFileOutputHost(self):
        return self.FloatPort(inputFileEndian="big_endian")   

    def testFloatPortBigFileOutputBig(self):
        return self.FloatPort(inputFileEndian="big_endian",outputOrder="big_endian")    

    def testFloatPortBigFileOutputLittle(self):
        return self.FloatPort(inputFileEndian="big_endian",outputOrder="little_endian")

    def FloatPort(self,inputFileEndian="little_endian", outputOrder= "host_order"):
        #######################################################################
        # Test FLOAT Functionality
        print "\n**TESTING FLOAT PORT"
        
        #Define test files
        dataFileIn = './data.in'
        
        #Create Test Data File if it doesn't exist
        #Floats and Doubles are a special case, as 
        #NaNs can be generated randomly and will 
        #not equal one another
        if not os.path.isfile(dataFileIn):
            with open(dataFileIn, 'wb') as dataIn:
                for i in range(1024/4):
                    floatNum = struct.unpack('f', os.urandom(4))[0]
                    while isnan(floatNum):
                        floatNum = struct.unpack('f', os.urandom(4))[0]
                    if inputFileEndian=="little":
                        dataIn.write(struct.pack('<f', floatNum))
                    elif inputFileEndian=="big":
                        dataIn.write(struct.pack('>f', floatNum))
                        
                    
        
        #Read in Data from Test File
        size = os.path.getsize(dataFileIn)
        data = []
        
        outputEndian = outputOrder
        if outputEndian == "host_order":
            outputEndian = sys.byteorder+'_endian'
        reverseChar = '>'
        if sys.byteorder == 'big':
            reverseChar = '<'
            
        with open (dataFileIn, 'rb') as dataIn:
            #print dataIn.read(size)
            if inputFileEndian == outputEndian:
                data = list(struct.unpack('@'+'f'*(size/4), dataIn.read(size)))
            else:
                data = list(struct.unpack(reverseChar+'f'*(size/4), dataIn.read(size)))
            
        #Create Components and Connections
        comp = sb.launch('../FileReader.spd.xml')
        comp.source_uri = dataFileIn
        comp.file_format='FLOAT_'+inputFileEndian.upper()
        
        sink = sb.DataSink()
        comp.connect(sink, usesPortName='dataFloat_out')
        
        #Start Components & Push Data
        sb.start()
        comp.playback_state = 'PLAY'
        time.sleep(2)
        readData = sink.getData()
        sb.stop()
        
        #Check that the input and output files are the same          
        try:
            self.assertEqual(data, readData)
        except self.failureException as e:
            comp.releaseObject()
            sink.releaseObject()
            os.remove(dataFileIn)
            raise e
        
        #Release the components and remove the generated files
        comp.releaseObject()
        sink.releaseObject()
        os.remove(dataFileIn)
        
        print "........ PASSED\n"
        return
    
    def testFloatPortSriScalar(self):
        dataFileIn = './data.in'
        self.setupBasicSriTest(dataFileIn, 'dataFloat_out', 'FLOAT_LITTLE_ENDIAN')
        self.assertSRIOutputMode(0)
        self.tearDownFlow(dataFileIn)
        print "........ PASSED\n"
    
    def testFloatPortSriComplex(self):
        dataFileIn = './data.in'
        self.setupBasicSriTest(dataFileIn, 'dataFloat_out', 'COMPLEX_FLOAT_LITTLE_ENDIAN')
        self.assertSRIOutputMode(1)
        self.tearDownFlow(dataFileIn)
        print "........ PASSED\n"

    def testDoublePort(self):
        #######################################################################
        # Test DOUBLE Functionality
        print "\n**TESTING DOUBLE PORT"
        
        #Define test files
        dataFileIn = './data.in'
        
        #Create Test Data File if it doesn't exist
        #Floats and Doubles are a special case, as 
        #NaNs can be generated randomly and will 
        #not equal one another
        if not os.path.isfile(dataFileIn):
            with open(dataFileIn, 'wb') as dataIn:
                for i in range(1024/8):
                    doubleNum = struct.unpack('d', os.urandom(8))[0]
                    while isnan(doubleNum):
                        doubleNum = struct.unpack('d', os.urandom(8))[0]
                    dataIn.write(struct.pack('d', doubleNum))
        
        #Read in Data from Test File
        size = os.path.getsize(dataFileIn)
        with open (dataFileIn, 'rb') as dataIn:
            data = list(struct.unpack('d'*(size/8), dataIn.read(size)))
            
        #Create Components and Connections
        comp = sb.launch('../FileReader.spd.xml')
        comp.source_uri = dataFileIn
        comp.file_format = 'DOUBLE_LITTLE_ENDIAN'
        
        sink = sb.DataSink()
        comp.connect(sink, usesPortName='dataDouble_out')
        
        #Start Components & Push Data
        sb.start()
        comp.playback_state = 'PLAY'
        time.sleep(2)
        readData = sink.getData()
        sb.stop()
        
        #Check that the input and output files are the same          
        try:
            self.assertEqual(data, readData)
        except self.failureException as e:
            comp.releaseObject()
            sink.releaseObject()
            os.remove(dataFileIn)
            raise e
        
        #Release the components and remove the generated files
        comp.releaseObject()
        sink.releaseObject()
        os.remove(dataFileIn)
        
        print "........ PASSED\n"
        return
    
    def testDoublePortSriScalar(self):
        dataFileIn = './data.in'
        self.setupBasicSriTest(dataFileIn, 'dataDouble_out', 'DOUBLE_LITTLE_ENDIAN')
        self.assertSRIOutputMode(0)
        self.tearDownFlow(dataFileIn)
        print "........ PASSED\n"
    
    def testDoublePortSriComplex(self):
        dataFileIn = './data.in'
        self.setupBasicSriTest(dataFileIn, 'dataDouble_out', 'COMPLEX_DOUBLE_LITTLE_ENDIAN')
        self.assertSRIOutputMode(1)
        self.tearDownFlow(dataFileIn)
        print "........ PASSED\n"
        
    def testXmlPort(self):
        #######################################################################
        # Test XML Functionality
        print "\n**TESTING XML PORT"
        
        dataFileIn = './data.xml'

        with open (dataFileIn, 'rb') as file:
            data=file.read()

        #Set up FileReader
        comp = sb.launch('../FileReader.spd.xml')
        comp.source_uri = dataFileIn
        comp.file_format = 'XML'
        
        sink = sb.DataSink()
        comp.connect(sink, usesPortName='dataXML_out')

        #Start Components & Push Data
        sb.start()
        comp.playback_state = 'PLAY'
        time.sleep(2)
        readData = sink.getData()
        sb.stop()
        
        #Convert list of strings into a string
        readData = readData[0]

        #Check that the input and output files are the same
        try:
            self.assertEqual(data, readData)
        except self.failureException as e:
            comp.releaseObject()
            sink.releaseObject()            
            raise e
        
        #Release the components and remove the generated files
        comp.releaseObject()
        sink.releaseObject()
        
        print "........ PASSED\n"
        return
    
    def testBlueTimestampPrecision(self):
        #######################################################################
        # Test BLUE file high precision timecode using TC_PREC keyword
        print "\n**TESTING BLUE file with TC_PREC"
        
        #Define test files
        dataFileIn = './bluefile.in'
        
        #Create Test Data File if it doesn't exist
        if not os.path.isfile(dataFileIn):
            tmpSink = bluefile_helpers.BlueFileWriter(dataFileIn, BULKIO__POA.dataShort)
            tmpSink.start()
            tmpSri = createSri('bluefileShort', 5000)
            tmpSri.keywords = props_from_dict({'TEST_KW':1234})
            tmpSink.pushSRI(tmpSri)
            tmpTs = createTs()
            tmpSink.pushPacket(range(1024), tmpTs, True, 'bluefileShort')
            hdr, d = bluefile.read(dataFileIn, dict)
            hdr['xstart'] = 5.5
            hdr['timecode'] = 1234567890.0987654
            hdr['keywords']['TC_PREC'] = '3.2112E-08'
            # Together, start_time = 1234567895.598765432112
            bluefile.write(dataFileIn, hdr, d)
            
        #Read in Data from Test File
        hdr, d = bluefile.read(dataFileIn, dict)
        data = list(d)
        keywords = hdr['ext_header']
            
        #Create Components and Connections
        comp = sb.launch('../FileReader.spd.xml')
        comp.source_uri = dataFileIn
        comp.file_format = 'BLUEFILE'
        
        sink = sb.DataSink()
        comp.connect(sink, usesPortName='dataShort_out')
        
        #Start Components & Push Data
        sb.start()
        comp.playback_state = 'PLAY'
        time.sleep(2)
        readData, readTstamps = sink.getData(tstamps=True)
        readKeywords = props_to_dict(sink.sri().keywords)
        readXstart = sink.sri().xstart
        sb.stop()

        #Check that timestamps are the same
        # Source: hdr['xstart'], hdr['timecode']-long(631152000), and float(hdr['keywords']['TC_PREC'])
        # Output: readTsamps, and sink.sri().xstart
        src_twsec = float(long(hdr['timecode'])-long(631152000))
        src_twsec += long(hdr['xstart'])
        src_tfsec = hdr['timecode']-long(hdr['timecode'])
        src_tfsec += (hdr['xstart']-long(hdr['xstart']))
        src_tfsec += float(hdr['keywords']['TC_PREC'])
        out_twsec = readTstamps[0][1].twsec + long(readXstart)
        out_tfsec = readTstamps[0][1].tfsec + (readXstart-long(readXstart))

        try:
            self.assertEqual(src_twsec, out_twsec, "Whole seconds do not match.")
            self.assertAlmostEqual(src_tfsec, out_tfsec, places=12, msg="Fractional seconds do not match.")
        except self.failureException as e:
            print 'Source time info: timecode=%s  xstart=%s  TC_PREC=%s'%(repr(hdr['timecode']), repr(hdr['xstart']), hdr['keywords']['TC_PREC'])
            print 'Source time info: twsec=%s  tfsec=%s'%(repr(src_twsec), repr(src_tfsec))
            print 'Output time info: readTstamps0.twsec=%s  readTstamps0.tfsec=%s  readXstart=%s'%(repr(readTstamps[0][1].twsec), repr(readTstamps[0][1].tfsec), repr(readXstart))
            print 'Output time info: twsec=%s  tfsec=%s'%(repr(out_twsec), repr(out_tfsec))
            comp.releaseObject()
            sink.releaseObject()
            os.remove(dataFileIn)
            raise e
        
        #Check that the input and output files are the same          
        try:
            self.assertEqual(data, readData)
        except self.failureException as e:
            comp.releaseObject()
            sink.releaseObject()
            os.remove(dataFileIn)
            raise e
        
        #Check that the keywords are the same
        try:
            self.assertEqual(keywords, readKeywords)
        except self.failureException as e:
            comp.releaseObject()
            sink.releaseObject()
            os.remove(dataFileIn)
            raise e
        
        #Release the components and remove the generated files
        comp.releaseObject()
        sink.releaseObject()
        os.remove(dataFileIn)
        
        print "........ PASSED\n"
        return

    def testHostByteOrderProp(self):
        #######################################################################
        # Test the host_byte_order property indicates correct host endianness
        print "\n**TESTING HOST BYTE ORDER PROP VALUE"

        # Create Component
        comp = sb.launch('../FileReader.spd.xml')

        # Check the host_byte_order property
        try:
            self.assertEqual(sys.byteorder + '_endian', comp.host_byte_order)
        except self.failureException as e:
            comp.releaseObject()
            raise e

        # Release the components and remove the generated files
        comp.releaseObject()

        print "........ PASSED\n"

    def testIdleCpuUtilization(self):
        #######################################################################
        # Test the CPU utilization when playback state is STOP and throttle 0
        print "\n**TESTING IDLE CPU UTILIZATION"
        import psutil

        # Create Component
        comp = sb.launch('../FileReader.spd.xml')
        comp.advanced_properties.throttle_rate = '0'
        comp.start()

        # Check the CPU utilization
        try:
            pid = comp._process.pid()
            p = psutil.Process(pid)
            # psutil.Process API changed in version 2.0.0 of psutil
            get_cpu_percent = getattr(p,'cpu_percent', getattr(p,'get_cpu_percent', None))
            assert get_cpu_percent is not None, "Test Environment Error: Unable to find method for measuring CPU percent"
            cpu_usage = 0.0
            iterations = 8
            for _ in xrange(iterations):
                cpu_usage += get_cpu_percent()
                time.sleep(0.25)
            cpu_usage = cpu_usage / iterations
            self.assertTrue(cpu_usage < 1.0, 'CPU Usage too high: %s is not less than 1.0'%cpu_usage)
        except self.failureException as e:
            comp.releaseObject()
            raise e

        # Release the components and remove the generated files
        comp.stop()
        comp.releaseObject()

        print "........ PASSED\n"

    # TODO Add additional tests here
    #
    # See:
    #   ossie.utils.bulkio.bulkio_helpers,
    #   ossie.utils.bluefile.bluefile_helpers
    # for modules that will assist with testing resource with BULKIO ports
if __name__ == "__main__":
    ossie.utils.testing.main("../FileReader.spd.xml") # By default tests all implementations
