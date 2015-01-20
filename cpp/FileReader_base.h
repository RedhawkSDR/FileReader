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

#ifndef FILEREADER_IMPL_BASE_H
#define FILEREADER_IMPL_BASE_H

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
        std::string source_uri;
        std::string file_format;
        std::string sample_rate;
        std::string center_frequency;
        std::string playback_state;
        advanced_properties_struct advanced_properties;
        default_timestamp_struct default_timestamp;
        default_sri_struct default_sri;
        component_status_struct component_status;
        std::vector<sri_keywords_struct_struct> default_sri_keywords;
        std::vector<file_status_struct_struct> file_status;

        // Ports
        bulkio::OutCharPort *dataChar_out;
        bulkio::OutDoublePort *dataDouble_out;
        bulkio::OutFloatPort *dataFloat_out;
        bulkio::OutLongPort *dataLong_out;
        bulkio::OutLongLongPort *dataLongLong_out;
        bulkio::OutOctetPort *dataOctet_out;
        bulkio::OutShortPort *dataShort_out;
        bulkio::OutULongPort *dataUlong_out;
        bulkio::OutULongLongPort *dataUlongLong_out;
        bulkio::OutUShortPort *dataUshort_out;
        bulkio::OutXMLPort *dataXML_out;

    private:
};
#endif // FILEREADER_IMPL_BASE_H
