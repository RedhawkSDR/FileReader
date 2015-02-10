#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file distributed with this 
# source distribution.
# 
# This file is part of REDHAWK Basic Components FileReader.
# 
# REDHAWK Basic Components FileReader is free software: you can redistribute it and/or modify it under the terms of 
# the GNU General Public License as published by the Free Software Foundation, either 
# version 3 of the License, or (at your option) any later version.
# 
# REDHAWK Basic Components FileReader is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
# PURPOSE.  See the GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along with this 
# program.  If not, see http://www.gnu.org/licenses/.
#

import unittest
import ossie.utils.testing
import os
from omniORB import any
from ossie.utils.bulkio import bulkio_data_helpers
import sys
import time
import threading
from new import classobj
from omniORB import any, CORBA
from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA
from ossie.cf import CF, CF__POA
from ossie.utils import uuid
from ossie.resource import Resource
from ossie.properties import simple_property
import subprocess
from ossie.utils import sb
import struct
from math import isnan

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
    
    def testShortPort(self):
        #######################################################################
        # Test SHORT Functionality
        print "\n**TESTING SHORT PORT"
        
        #Define test files
        dataFileIn = './data.in'
        
        #Create Test Data File if it doesn't exist
        if not os.path.isfile(dataFileIn):
            with open(dataFileIn, 'wb') as dataIn:
                dataIn.write(os.urandom(1024))
        
        #Read in Data from Test File
        size = os.path.getsize(dataFileIn)
        with open (dataFileIn, 'rb') as dataIn:
            data = list(struct.unpack('h'*(size/2), dataIn.read(size)))
            
        #Create Components and Connections
        comp = sb.launch('../FileReader.spd.xml')
        comp.source_uri = dataFileIn
        comp.file_format = 'SHORT'
        
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
        self.setupBasicSriTest(dataFileIn, 'dataShort_out', 'SHORT')
        self.assertSRIOutputMode(0)
        self.tearDownFlow(dataFileIn)
        print "........ PASSED\n"
    
    def testShortPortSriComplex(self):
        dataFileIn = './data.in'
        self.setupBasicSriTest(dataFileIn, 'dataShort_out', 'COMPLEX_SHORT')
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
    
    def testUshortPortSriScalar(self):
        dataFileIn = './data.in'
        self.setupBasicSriTest(dataFileIn, 'dataUshort_out', 'USHORT')
        self.assertSRIOutputMode(0)
        self.tearDownFlow(dataFileIn)
        print "........ PASSED\n"
    
    def testShortPortSriComplex(self):
        dataFileIn = './data.in'
        self.setupBasicSriTest(dataFileIn, 'dataUshort_out', 'COMPLEX_USHORT')
        self.assertSRIOutputMode(1)
        self.tearDownFlow(dataFileIn)
        print "........ PASSED\n"

    def testFloatPort(self):
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
                    dataIn.write(struct.pack('f', floatNum))
        
        #Read in Data from Test File
        size = os.path.getsize(dataFileIn)
        with open (dataFileIn, 'rb') as dataIn:
            data = list(struct.unpack('f'*(size/4), dataIn.read(size)))
            
        #Create Components and Connections
        comp = sb.launch('../FileReader.spd.xml')
        comp.source_uri = dataFileIn
        comp.file_format = 'FLOAT'
        
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
        self.setupBasicSriTest(dataFileIn, 'dataFloat_out', 'FLOAT')
        self.assertSRIOutputMode(0)
        self.tearDownFlow(dataFileIn)
        print "........ PASSED\n"
    
    def testFloatPortSriComplex(self):
        dataFileIn = './data.in'
        self.setupBasicSriTest(dataFileIn, 'dataFloat_out', 'COMPLEX_FLOAT')
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
        comp.file_format = 'DOUBLE'
        
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
        self.setupBasicSriTest(dataFileIn, 'dataDouble_out', 'DOUBLE')
        self.assertSRIOutputMode(0)
        self.tearDownFlow(dataFileIn)
        print "........ PASSED\n"
    
    def testDoublePortSriComplex(self):
        dataFileIn = './data.in'
        self.setupBasicSriTest(dataFileIn, 'dataDouble_out', 'COMPLEX_DOUBLE')
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

if __name__ == "__main__":
    
    ossie.utils.testing.main("../FileReader.spd.xml") # By default tests all implementations
