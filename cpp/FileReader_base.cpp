/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK Basic Components FileReader.
 *
 * REDHAWK Basic Components FileReader is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK Basic Components FileReader is distributed in the hope that it will be useful, but WITHOUT
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
    Component(uuid, label),
    ThreadedComponent()
{
#ifdef BEGIN_AUTOCOMPLETE_IGNORE
    loadProperties();

    dataChar_out = new bulkio::OutCharPort("dataChar_out");
    addPort("dataChar_out", "Char output port for data. ", dataChar_out);
    dataDouble_out = new bulkio::OutDoublePort("dataDouble_out");
    addPort("dataDouble_out", "Double output port for data. ", dataDouble_out);
    dataFloat_out = new bulkio::OutFloatPort("dataFloat_out");
    addPort("dataFloat_out", "Float output port for data. ", dataFloat_out);
    dataLong_out = new bulkio::OutLongPort("dataLong_out");
    addPort("dataLong_out", "Long (4-Byte) output port for data. ", dataLong_out);
    dataLongLong_out = new bulkio::OutLongLongPort("dataLongLong_out");
    addPort("dataLongLong_out", "Long Long (8-Byte) output port for data. ", dataLongLong_out);
    dataOctet_out = new bulkio::OutOctetPort("dataOctet_out");
    addPort("dataOctet_out", "Octet (byte data) output port for data.  ", dataOctet_out);
    dataShort_out = new bulkio::OutShortPort("dataShort_out");
    addPort("dataShort_out", "Short (2 Byte) output port for data. ", dataShort_out);
    dataUlong_out = new bulkio::OutULongPort("dataUlong_out");
    addPort("dataUlong_out", "Unsigned Long output port for data. ", dataUlong_out);
    dataUlongLong_out = new bulkio::OutULongLongPort("dataUlongLong_out");
    addPort("dataUlongLong_out", "Unsigned Long Long output port for data. ", dataUlongLong_out);
    dataUshort_out = new bulkio::OutUShortPort("dataUshort_out");
    addPort("dataUshort_out", "Unsigned Short output port for data. ", dataUshort_out);
    dataXML_out = new bulkio::OutXMLPort("dataXML_out");
    addPort("dataXML_out", "XML output port for data. ", dataXML_out);
#endif
}

FileReader_base::~FileReader_base()
{
    delete dataChar_out;
    dataChar_out = 0;
    delete dataDouble_out;
    dataDouble_out = 0;
    delete dataFloat_out;
    dataFloat_out = 0;
    delete dataLong_out;
    dataLong_out = 0;
    delete dataLongLong_out;
    dataLongLong_out = 0;
    delete dataOctet_out;
    dataOctet_out = 0;
    delete dataShort_out;
    dataShort_out = 0;
    delete dataUlong_out;
    dataUlong_out = 0;
    delete dataUlongLong_out;
    dataUlongLong_out = 0;
    delete dataUshort_out;
    dataUshort_out = 0;
    delete dataXML_out;
    dataXML_out = 0;
}

#ifdef BEGIN_AUTOCOMPLETE_IGNORE
/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void FileReader_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    Component::start();
    ThreadedComponent::startThread();
}

void FileReader_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    Component::stop();
    if (!ThreadedComponent::stopThread()) {
        throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }
}

void FileReader_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    Component::releaseObject();
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
                "property");

    addProperty(file_format,
                "SHORT",
                "file_format",
                "file_format",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(sample_rate,
                "25Msps",
                "sample_rate",
                "sample_rate",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(center_frequency,
                "0.0",
                "center_frequency",
                "center_frequency",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(playback_state,
                "STOP",
                "playback_state",
                "playback_state",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(advanced_properties,
                advanced_properties_struct(),
                "advanced_properties",
                "advanced_properties",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(default_timestamp,
                default_timestamp_struct(),
                "default_timestamp",
                "default_timestamp",
                "readwrite",
                "",
                "external",
                "property");

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
                "property");

    addProperty(default_sri_keywords,
                "default_sri_keywords",
                "default_sri_keywords",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(file_status,
                "file_status",
                "file_status",
                "readonly",
                "",
                "external",
                "property");

}
#endif


