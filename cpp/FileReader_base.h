/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK Basic Components FileReader.
 *
 * REDHAWK Basic Components FileReader is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK Basic Components FileReader is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef FILEREADER_BASE_IMPL_BASE_H
#define FILEREADER_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>

#include <bulkio/bulkio.h>
#include "struct_props.h"

class FileReader_base : public Component, protected ThreadedComponent
{
    public:
        FileReader_base(const char *uuid, const char *label);
        ~FileReader_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        /// Property: source_uri
        std::string source_uri;
        /// Property: file_format
        std::string file_format;
        /// Property: sample_rate
        std::string sample_rate;
        /// Property: center_frequency
        std::string center_frequency;
        /// Property: playback_state
        std::string playback_state;
        /// Property: advanced_properties
        advanced_properties_struct advanced_properties;
        /// Property: default_timestamp
        default_timestamp_struct default_timestamp;
        /// Property: default_sri
        default_sri_struct default_sri;
        /// Property: component_status
        component_status_struct component_status;
        /// Property: default_sri_keywords
        std::vector<sri_keywords_struct_struct> default_sri_keywords;
        /// Property: file_status
        std::vector<file_status_struct_struct> file_status;

        // Ports
        /// Port: dataChar_out
        bulkio::OutCharPort *dataChar_out;
        /// Port: dataDouble_out
        bulkio::OutDoublePort *dataDouble_out;
        /// Port: dataFloat_out
        bulkio::OutFloatPort *dataFloat_out;
        /// Port: dataLong_out
        bulkio::OutLongPort *dataLong_out;
        /// Port: dataLongLong_out
        bulkio::OutLongLongPort *dataLongLong_out;
        /// Port: dataOctet_out
        bulkio::OutOctetPort *dataOctet_out;
        /// Port: dataShort_out
        bulkio::OutShortPort *dataShort_out;
        /// Port: dataUlong_out
        bulkio::OutULongPort *dataUlong_out;
        /// Port: dataUlongLong_out
        bulkio::OutULongLongPort *dataUlongLong_out;
        /// Port: dataUshort_out
        bulkio::OutUShortPort *dataUshort_out;
        /// Port: dataXML_out
        bulkio::OutXMLPort *dataXML_out;

    private:
};
#endif // FILEREADER_BASE_IMPL_BASE_H
