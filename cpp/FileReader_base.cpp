/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of FilterDecimate.
 *
 * FilterDecimate is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * FilterDecimate is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#include "FileReader_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

FileReader_base::FileReader_base(const char *uuid, const char *label) :
    Resource_impl(uuid, label),
    serviceThread(0)
{
    construct();
}

void FileReader_base::construct()
{
    Resource_impl::_started = false;
    loadProperties();
    serviceThread = 0;
    
    PortableServer::ObjectId_var oid;
    DomainManager_out = new CF_DomainManager_Out_i("DomainManager_out", this);
    oid = ossie::corba::RootPOA()->activate_object(DomainManager_out);
    dataChar_out = new bulkio::OutCharPort("dataChar_out");
    oid = ossie::corba::RootPOA()->activate_object(dataChar_out);
    dataDouble_out = new bulkio::OutDoublePort("dataDouble_out");
    oid = ossie::corba::RootPOA()->activate_object(dataDouble_out);
    dataFloat_out = new bulkio::OutFloatPort("dataFloat_out");
    oid = ossie::corba::RootPOA()->activate_object(dataFloat_out);
    dataLong_out = new bulkio::OutLongPort("dataLong_out");
    oid = ossie::corba::RootPOA()->activate_object(dataLong_out);
    dataLongLong_out = new bulkio::OutLongLongPort("dataLongLong_out");
    oid = ossie::corba::RootPOA()->activate_object(dataLongLong_out);
    dataOctet_out = new bulkio::OutOctetPort("dataOctet_out");
    oid = ossie::corba::RootPOA()->activate_object(dataOctet_out);
    dataShort_out = new bulkio::OutShortPort("dataShort_out");
    oid = ossie::corba::RootPOA()->activate_object(dataShort_out);
    dataUlong_out = new bulkio::OutULongPort("dataUlong_out");
    oid = ossie::corba::RootPOA()->activate_object(dataUlong_out);
    dataUlongLong_out = new bulkio::OutULongLongPort("dataUlongLong_out");
    oid = ossie::corba::RootPOA()->activate_object(dataUlongLong_out);
    dataUshort_out = new bulkio::OutUShortPort("dataUshort_out");
    oid = ossie::corba::RootPOA()->activate_object(dataUshort_out);
    dataXML_out = new bulkio::OutXMLPort("dataXML_out");
    oid = ossie::corba::RootPOA()->activate_object(dataXML_out);

    registerOutPort(DomainManager_out, DomainManager_out->_this());
    registerOutPort(dataChar_out, dataChar_out->_this());
    registerOutPort(dataDouble_out, dataDouble_out->_this());
    registerOutPort(dataFloat_out, dataFloat_out->_this());
    registerOutPort(dataLong_out, dataLong_out->_this());
    registerOutPort(dataLongLong_out, dataLongLong_out->_this());
    registerOutPort(dataOctet_out, dataOctet_out->_this());
    registerOutPort(dataShort_out, dataShort_out->_this());
    registerOutPort(dataUlong_out, dataUlong_out->_this());
    registerOutPort(dataUlongLong_out, dataUlongLong_out->_this());
    registerOutPort(dataUshort_out, dataUshort_out->_this());
    registerOutPort(dataXML_out, dataXML_out->_this());
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void FileReader_base::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
}

void FileReader_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    if (serviceThread == 0) {
        serviceThread = new ProcessThread<FileReader_base>(this, 0.1);
        serviceThread->start();
    }
    
    if (!Resource_impl::started()) {
    	Resource_impl::start();
    }
}

void FileReader_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    // release the child thread (if it exists)
    if (serviceThread != 0) {
        if (!serviceThread->release(2)) {
            throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
        }
        serviceThread = 0;
    }
    
    if (Resource_impl::started()) {
    	Resource_impl::stop();
    }
}

CORBA::Object_ptr FileReader_base::getPort(const char* _id) throw (CORBA::SystemException, CF::PortSupplier::UnknownPort)
{

    std::map<std::string, Port_Provides_base_impl *>::iterator p_in = inPorts.find(std::string(_id));
    if (p_in != inPorts.end()) {
    }

    std::map<std::string, CF::Port_var>::iterator p_out = outPorts_var.find(std::string(_id));
    if (p_out != outPorts_var.end()) {
        return CF::Port::_duplicate(p_out->second);
    }

    throw (CF::PortSupplier::UnknownPort());
}

void FileReader_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    // deactivate ports
    releaseInPorts();
    releaseOutPorts();

    delete(DomainManager_out);
    delete(dataChar_out);
    delete(dataDouble_out);
    delete(dataFloat_out);
    delete(dataLong_out);
    delete(dataLongLong_out);
    delete(dataOctet_out);
    delete(dataShort_out);
    delete(dataUlong_out);
    delete(dataUlongLong_out);
    delete(dataUshort_out);
    delete(dataXML_out);

    Resource_impl::releaseObject();
}

void FileReader_base::loadProperties()
{
    addProperty(source_uri,
                "file://[path_to_local_file_or_dir] OR sca://[path_to_sca_file_or_dir]",
                "source_uri",
                "source_uri",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(file_format,
                "SHORT",
                "file_format",
                "file_format",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(sample_rate,
                "25Msps",
                "sample_rate",
                "sample_rate",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(center_frequency,
                "0.0",
                "center_frequency",
                "center_frequency",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(playback_state,
                "STOP",
                "playback_state",
                "playback_state",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(advanced_properties,
                advanced_properties_struct(),
                "advanced_properties",
                "advanced_properties",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(default_timestamp,
                default_timestamp_struct(),
                "default_timestamp",
                "default_timestamp",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(default_sri,
                default_sri_struct(),
                "default_sri",
                "default_sri",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(component_status,
                component_status_struct(),
                "component_status",
                "component_status",
                "readonly",
                "",
                "external",
                "configure");

    addProperty(default_sri_keywords,
                "default_sri_keywords",
                "default_sri_keywords",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(file_status,
                "file_status",
                "file_status",
                "readonly",
                "",
                "external",
                "configure");

}


