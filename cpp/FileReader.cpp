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

/**************************************************************************

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "FileReader.h"

PREPARE_LOGGING(FileReader_i)

FileReader_i::FileReader_i(const char *uuid, const char *label) :
FileReader_base(uuid, label) {
    packet_size = DEF_PACKET_SIZE;
    buffer_size = DEF_PACKET_SIZE * 10;
    buffer_thread = NULL;
    throttle_usleep = 0;
    throttle_rate_Bps = 0;
    sample_rate_d = 0;
    center_frequency_d = 0;
    current_sample_rate = 0;
    current_data_format = SUPPORTED_DATA_TYPE::UNKNOWN;
    current_sri.streamID = "";
    data_tstamp = get_current_timestamp();
    //sets the buffer to the max size of the possible of a transfer that is going to converted to a max data type
    port_buffer.reserve(CORBA_MAX_TRANSFER_BYTES*sizeof(double)*2);
}

FileReader_i::~FileReader_i() {
    stop_cache_thread();

    delete metadataQueue;
    delete MetaDataParser_i;

}

void FileReader_i::constructor()
{
    /***********************************************************************************
     This is the RH constructor. All properties are properly initialized before this function is called
    ***********************************************************************************/

    // In metadata file mode, we need a queue for the metadata and the metadata parser
    // The metadata queue uses a bulkio port because it contains a queue for packets and SRI, this is just used as an internal data helper and is not actually a bulkio port
    metadataQueue = new bulkio::InLongPort("metadataQueue");
    metadataQueue->setMaxQueueDepth(1000000);
    MetaDataParser_i = new MetaDataParser(metadataQueue,&packetSizeQueue);

    //properties are not updated until callback function is called and they are explicitly
    addPropertyChangeListener("advanced_properties", this, &FileReader_i::advanced_propertiesChanged);
    addPropertyChangeListener("source_uri", this, &FileReader_i::source_uriChanged);
    addPropertyChangeListener("file_format", this, &FileReader_i::file_formatChanged);
    addPropertyChangeListener("sample_rate", this, &FileReader_i::sample_rateChanged);
    addPropertyChangeListener("center_frequency", this, &FileReader_i::center_frequencyChanged);
    addPropertyChangeListener("playback_state", this, &FileReader_i::playback_stateChanged);
    addPropertyChangeListener("default_timestamp", this, &FileReader_i::default_timestampChanged);
    addPropertyChangeListener("default_sri", this, &FileReader_i::default_sriChanged);
    addPropertyChangeListener("default_sri_keywords", this, &FileReader_i::default_sri_keywordsChanged);

    // Determine host byte order
    switch(BYTE_ORDER) {
    case LITTLE_ENDIAN:
        host_byte_order = "little_endian";
        break;
    case BIG_ENDIAN:
        host_byte_order = "big_endian";
        break;
    default:
        host_byte_order = "little_endian";
        LOG_ERROR(FileReader_i,"Could not determine host byte order ["<<BYTE_ORDER<<"], defaulting to Little Endian");
    }

    // Determine BulkIO output byte order
    if (output_bulkio_byte_order == "little_endian") {
        BULKIO_BYTE_ORDER = LITTLE_ENDIAN;
    } else if (output_bulkio_byte_order == "big_endian") {
        BULKIO_BYTE_ORDER = BIG_ENDIAN;
    } else { // host_order
        BULKIO_BYTE_ORDER = BYTE_ORDER;
    }
}

void FileReader_i::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
    LOG_TRACE(FileReader_i,"FileReader_i::initialize - ENTER");
    FileReader_base::initialize();
    try {
        if(!filesystem.is_sca_file_manager_valid()){
            if (getDomainManager() && !CORBA::is_nil(getDomainManager()->getRef())) {
                std::string dom_id = ossie::corba::returnString(getDomainManager()->getRef()->identifier());
                CF::DomainManager_var dm = FILE_READER_DOMAIN_MGR_HELPERS::domainManager_id_to_var(dom_id);
                if (!CORBA::is_nil(dm)){
                    filesystem.update_sca_file_manager(dm->fileMgr());
                    component_status.domain_name = ossie::corba::returnString(dm->name());
                }
            }
        }
    } catch (...) {
        LOG_DEBUG(FileReader_i,"Exception caught while attempting to update sca file manager");
        //component_status.domain_name = "(domainless)"; // leave as default value
    };

    // Setup based on initial property values
    sample_rate_d = STD_STRING_HELPER::SPS_string_to_number(sample_rate);
    current_sample_rate = sample_rate_d;
    current_data_format = file_format;
    center_frequency_d = STD_STRING_HELPER::HZ_string_to_number(center_frequency);
    reconstruct_property_sri(current_sample_rate);
    reconstruct_property_timestamp();

    // Call reset_throttle() to calculate throttle_rate_Bps, needed by restart_read_ahead_caching()
    // The second call properly sets throttle_usleep based on packet_size, which is not set until restart_read_ahead_caching()
    reset_throttle();
    restart_read_ahead_caching();
    reset_throttle();
}

void FileReader_i::start() throw (CF::Resource::StartError, CORBA::SystemException) {

    if(source_uri != FILE_READER::DEF_SOURCE_URI && file_status.size() == 0){
        throw CF::Resource::StartError(CF::CF_NOTSET,"NO FILES TO PROCESS");
    }

    FileReader_base::start();


}

void FileReader_i::stop() throw (CF::Resource::StopError, CORBA::SystemException) {
    FileReader_base::stop();

    try{
        CF::Properties props;
        props.length(1);
        props[0].id = "playback_state";
        props[0].value <<= CORBA::string_dup("STOP");
        configure(props);
    } catch(...){};
}

void FileReader_i::advanced_propertiesChanged(const advanced_properties_struct *oldValue, const advanced_properties_struct *newValue) {
    exclusive_lock lock(service_thread_lock);
    LOG_TRACE(FileReader_i,"FileReader_i::advanced_propertiesChanged - ENTER");

    // These properties affect the manner in which the file is read, so the restart_read_ahead_caching
    // function should be called
    if (oldValue->buffer_size != newValue->buffer_size || oldValue->packet_size != newValue->packet_size ||
            oldValue->looping != newValue->looping || oldValue->looping_suppress_eos_until_stop != newValue->looping_suppress_eos_until_stop
            || oldValue->use_metadata_file != newValue->use_metadata_file) {
        restart_read_ahead_caching();
    }
    // This property affects the SRI, so the reconstruct_property_sri function should be called
    if (oldValue->center_frequency_keywords != newValue->center_frequency_keywords) {
        reconstruct_property_sri(current_sample_rate);
    }
    // This property affects the throttle, so the reset_throttle function should be called
    if (oldValue->throttle_rate != newValue->throttle_rate) {
        reset_throttle();
    }
}

void FileReader_i::source_uriChanged(const std::string *oldValue, const std::string *newValue) {
    exclusive_lock lock(service_thread_lock);
    LOG_TRACE(FileReader_i,"FileReader_i::source_uriChanged - ENTER");

    if (*oldValue != *newValue) {
        try {
            restart_read_ahead_caching();
        } catch(...) {
            source_uri = *oldValue;
            throw;
        }
    }
}

void FileReader_i::file_formatChanged(const std::string *oldValue, const std::string *newValue) {
    exclusive_lock lock(service_thread_lock);
    LOG_TRACE(FileReader_i,"FileReader_i::file_formatChanged - ENTER");

    if (*oldValue != *newValue) {
        restart_read_ahead_caching();
    }
    reconstruct_property_sri(current_sample_rate);
}

void FileReader_i::sample_rateChanged(const std::string *oldValue, const std::string *newValue) {
    exclusive_lock lock(service_thread_lock);
    LOG_TRACE(FileReader_i,"FileReader_i::sample_rateChanged - ENTER");

    if (*oldValue != *newValue) {
        if (file_format == "BLUEFILE") {
            LOG_WARN(FileReader_i, "Ignoring attempt to set sample rate while reading blue file");
            sample_rate = *oldValue;
        } else {
            sample_rate_d = STD_STRING_HELPER::SPS_string_to_number(*newValue);
            current_sample_rate = sample_rate_d;
            reset_throttle();
            reconstruct_property_sri(current_sample_rate);
            restart_read_ahead_caching();
        }
    }
}

void FileReader_i::center_frequencyChanged(const std::string *oldValue, const std::string *newValue) {
    exclusive_lock lock(service_thread_lock);

    if (*oldValue != *newValue) {
        center_frequency_d = STD_STRING_HELPER::HZ_string_to_number(*newValue);
        reconstruct_property_sri(current_sample_rate);
    }
}

void FileReader_i::playback_stateChanged(const std::string *oldValue, const std::string *newValue) {
    exclusive_lock lock(service_thread_lock);
    if (oldValue && newValue) {
        LOG_TRACE(FileReader_i,"FileReader_i::playback_stateChanged - ENTER - oldValue="<<*oldValue<<"  newValue="<<*newValue);
    } else if (newValue) {
        LOG_TRACE(FileReader_i,"FileReader_i::playback_stateChanged - ENTER - newValue="<<*newValue);
    } else {
        LOG_TRACE(FileReader_i,"FileReader_i::playback_stateChanged - ENTER");
    }

    if (*oldValue != *newValue) {
        if (playback_state == "STOP") {
            std::vector<char> empty_vector;
            restart_read_ahead_caching();
            std::string curStreamID = std::string(current_sri.streamID);
            if (!curStreamID.empty()) // if currently playing, send EOS
            {
                if (advanced_properties.debug_output) {
                    std::cout << __PRETTY_FUNCTION__ << " pushPacket:: Data Type: " << file_format << ", Packet Address: " << (void*) &curStreamID[0] << ", Number Bytes: " << 0 <<
                            ", Timestamp(m/s/o/w/f): " << data_tstamp.tcmode << "/" << data_tstamp.tcstatus << "/" << data_tstamp.toff << "/" <<
                            data_tstamp.twsec << "/" << data_tstamp.tfsec << ", EOS: " << true << ", Stream ID: " << curStreamID << std::endl;
                }
                pushPacket((std::vector<CORBA::Char> *) & empty_vector, data_tstamp, true, curStreamID);
                outstanding_streams.erase(curStreamID);
            }
            while(!outstanding_streams.empty()){
                std::map<std::string,loop_info>::iterator os_iter = outstanding_streams.begin();
                pushPacket((std::vector<CORBA::Char> *) & empty_vector, os_iter->second.tstamp, true, os_iter->second.stream_id);
                outstanding_streams.erase(os_iter);
            }
        } else if (playback_state == "PLAY") {
            reset_throttle();
        }
    }
}

void FileReader_i::default_timestampChanged(const default_timestamp_struct *oldValue, const default_timestamp_struct *newValue) {
    exclusive_lock lock(service_thread_lock);
    reconstruct_property_timestamp();
}

void FileReader_i::default_sriChanged(const default_sri_struct *oldValue, const default_sri_struct *newValue) {
    exclusive_lock lock(service_thread_lock);
    reconstruct_property_sri(current_sample_rate);
}

void FileReader_i::default_sri_keywordsChanged(const std::vector<sri_keywords_struct_struct> *oldValue, const std::vector<sri_keywords_struct_struct> *newValue) {
    exclusive_lock lock(service_thread_lock);
    reconstruct_property_sri(current_sample_rate);
}

void FileReader_i::output_bulkio_byte_orderChanged(std::string oldValue, std::string newValue){
    exclusive_lock lock(service_thread_lock);
    if (oldValue != newValue) {
        if (newValue == "little_endian") {
            LOG_DEBUG(FileReader_i,"input_bulkio_byte_order changed from "<<oldValue<<" to "<<newValue);
            BULKIO_BYTE_ORDER = LITTLE_ENDIAN;
        } else if (newValue ==  "big_endian") {
            LOG_DEBUG(FileReader_i,"input_bulkio_byte_order changed from "<<oldValue<<" to "<<newValue);
            BULKIO_BYTE_ORDER = BIG_ENDIAN;
        } else if (newValue == "host_order") {
            LOG_DEBUG(FileReader_i,"input_bulkio_byte_order changed from "<<oldValue<<" to "<<newValue);
            BULKIO_BYTE_ORDER = BYTE_ORDER;
        } else {
            LOG_ERROR(FileReader_i,"Configured with invalid output_bulkio_byte_order value: "<<newValue<<"; Reverting back to previous value: "<<oldValue);
            output_bulkio_byte_order = oldValue;
        }
    }
}

void FileReader_i::restart_read_ahead_caching() {
    LOG_TRACE(FileReader_i,"FileReader_i::restart_read_ahead_caching - ENTER");
    //Buffer Size
    packet_size = size_t(STD_STRING_HELPER::generic_string_to_number(advanced_properties.packet_size,"B",1024));

    if(packet_size <= 0){
        if(throttle_rate_Bps > 0)
            packet_size = size_t(std::max(1.0, current_sample_rate) * dth.get_dt_descriptor(current_data_format)->bytes_per_sample);
        else {
            packet_size = CORBA_MAX_TRANSFER_BYTES;
        }
    }

    // Keep on a 2B boundary - Note: it's safe to cast packet_size because previous conditional block ensures it's >= 0
    if (static_cast<unsigned long>(packet_size) + PACKET_HEADER_RESERVED > CORBA_MAX_TRANSFER_BYTES ){
        int mult = std::max(int(std::floor((CORBA_MAX_TRANSFER_BYTES - PACKET_HEADER_RESERVED)/16)),1);
        packet_size = mult*16;
    }

    buffer_size = size_t(STD_STRING_HELPER::generic_string_to_number(advanced_properties.buffer_size,"B",1024));
    if (buffer_size < 0) {
        buffer_size = 0;
    }

    stop_cache_thread();

    if (!populate_file_listing(source_uri)) {
        if (!populate_file_listing(source_uri+'/')) {
            LOG_ERROR(FileReader_i, "SOURCE_URI IS INVALID!");
            throw CF::PropertySet::InvalidConfiguration();
        }
        source_uri +='/';
    }

    start_cache_thread();
}

void FileReader_i::stop_cache_thread() {
    if (buffer_thread != NULL) {
        buffer_thread->interrupt();
        buffer_thread->join();
        delete buffer_thread;
        buffer_thread = NULL;
    }
}

void FileReader_i::start_cache_thread() {
    LOG_TRACE(FileReader_i,"FileReader_i::start_cache_thread - ENTER");
    stop_cache_thread();

    size_t num_packets_in_buffer = std::max(size_t(1), size_t(buffer_size / packet_size));

    // Empty used queue, and fill available queue (somewhat efficiently)
    while (used_file_packets.getUsage() > 0)
        available_file_packets.push(used_file_packets.pop());
    while (num_packets_in_buffer < available_file_packets.getUsage())
        available_file_packets.pop();
    for (size_t i = 0; i < available_file_packets.getUsage(); i++) {
        shared_ptr_file_packet pkt = available_file_packets.pop();
        pkt->dataBuffer.reserve(packet_size);
        available_file_packets.push(pkt);
    }
    while (num_packets_in_buffer > available_file_packets.getUsage()) {
        shared_ptr_file_packet pkt = shared_ptr_file_packet(new file_packet);
        pkt->dataBuffer.reserve(packet_size);
        available_file_packets.push(pkt);
    }

    //Reset Packet Queue
    if (metadataQueue) {
    	delete metadataQueue;
    }
    if (MetaDataParser_i) {
    	delete MetaDataParser_i;
    }
    metadataQueue = new bulkio::InLongPort("metadataQueue");
    metadataQueue->setMaxQueueDepth(1000000);
    MetaDataParser_i = new MetaDataParser(metadataQueue,&packetSizeQueue);

    buffer_thread = new boost::thread(&FileReader_i::read_ahead_thread, this);

}

void FileReader_i::reset_throttle() {
    throttle_rate_Bps = 0;

    if (advanced_properties.throttle_rate.find("%SAMPLE_RATE%") != std::string::npos)
        throttle_rate_Bps = size_t(current_sample_rate * dth.get_dt_descriptor(current_data_format)->bytes_per_sample);
    else
        throttle_rate_Bps = size_t(STD_STRING_HELPER::generic_string_to_number(advanced_properties.throttle_rate,"BPS",1024));


    throttle_usleep = 0;
    if (throttle_rate_Bps > 0)
        throttle_usleep = size_t((double(packet_size) / double(throttle_rate_Bps)) * 0.98 * 1e6);
    throttle_tstamp = get_current_timestamp();

}

void FileReader_i::reconstruct_property_timestamp() {
    // Use timestamp from properties
    property_tstamp.tcmode = default_timestamp.tcmode;
    property_tstamp.tcstatus = default_timestamp.tcstatus;
    property_tstamp.tfsec = default_timestamp.tfsec;
    property_tstamp.toff = default_timestamp.toff;
    property_tstamp.twsec = default_timestamp.twsec;
}

void FileReader_i::reconstruct_property_sri(const double &sample_rate) {
    property_sri.hversion = default_sri.hversion;
    property_sri.xstart = default_sri.xstart;
    property_sri.xdelta = default_sri.xdelta;
    property_sri.xunits = default_sri.xunits;
    property_sri.subsize = default_sri.subsize;
    property_sri.ystart = default_sri.ystart;
    property_sri.ydelta = default_sri.ydelta;
    property_sri.yunits = default_sri.yunits;
    property_sri.mode = default_sri.mode;
    property_sri.streamID = default_sri.streamID.c_str();
    property_sri.blocking = default_sri.blocking;

    // Autodetermine mode from data type
    if (property_sri.mode < 0){
        // COMPLEX_OCTET is not mapped properly in dth... Fixing for now
        std::string dataFormat(current_data_format);
        if (dataFormat == "COMPLEX_OCTET") dataFormat = SUPPORTED_DATA_TYPE::COMPLEX_OCTET;
        property_sri.mode = short(dth.get_dt_descriptor(dataFormat)->mode == SUPPORTED_DATA_TYPE::_complex_);
    }

    // If the sample rate passed in is valid, it will override
    // the default SRI sample rate and the property
    if (sample_rate > 0) {
        LOG_INFO(FileReader_i, "Using sample rate of " << sample_rate << " Sps");
        property_sri.xdelta = 1.0 / sample_rate;
    } else if (property_sri.xdelta <= 0) {
        LOG_INFO(FileReader_i, "Default SRI xdelta invalid, using sample rate property");
        property_sri.xdelta = 1.0 / sample_rate_d;
    } else {
        LOG_INFO(FileReader_i, "Using default SRI xdelta");
    }

    bool has_col_rf = false;
    bool has_chan_rf = false;
    property_sri.keywords.length(default_sri_keywords.size());
    for (size_t i = 0; i < default_sri_keywords.size(); i++) {
        if( default_sri_keywords[i].id == "COL_RF")
            has_col_rf = true;
        if( default_sri_keywords[i].id == "CHAN_RF")
            has_chan_rf = true;
        property_sri.keywords[i].id = default_sri_keywords[i].id.c_str();
        CORBA::Any value;
        if (default_sri_keywords[i].value_type == "STRING") {
            value <<= CORBA::string_dup(default_sri_keywords[i].value.c_str());
        } else if (default_sri_keywords[i].value_type == "BOOLEAN") {
            if (std::isdigit(default_sri_keywords[i].value[0]))
                value <<= CORBA::Boolean(std::atoi(default_sri_keywords[i].value.c_str()));
            else {
                toUpper(default_sri_keywords[i].value);
                value <<= CORBA::Boolean(true);
                if (default_sri_keywords[i].value == "FALSE")
                    value <<= CORBA::Boolean(false);
            }
        } else if (default_sri_keywords[i].value_type == "SHORT") {
            value <<= CORBA::Short(std::atoi(default_sri_keywords[i].value.c_str()));
        } else if (default_sri_keywords[i].value_type == "CHAR") {
            value <<= CORBA::Any::from_char(default_sri_keywords[i].value.at(0));
        } else if (default_sri_keywords[i].value_type == "FLOAT") {
            value <<= CORBA::Float(std::atof(default_sri_keywords[i].value.c_str()));
        } else if (default_sri_keywords[i].value_type == "DOUBLE") {
            value <<= CORBA::Double(std::atof(default_sri_keywords[i].value.c_str()));
        } else if (default_sri_keywords[i].value_type == "LONG") {
            value <<= CORBA::Long(std::atol(default_sri_keywords[i].value.c_str()));
        } else if (default_sri_keywords[i].value_type == "OCTET") {
            value <<= CORBA::Any::from_octet(default_sri_keywords[i].value[0]);
        } else if (default_sri_keywords[i].value_type == "USHORT") {
            value <<= CORBA::UShort(std::atoi(default_sri_keywords[i].value.c_str()));
        } else {
            value <<= CORBA::string_dup(default_sri_keywords[i].value.c_str());
        }
        property_sri.keywords[i].value = value;
    }

    // Update Keywords
    if(!has_col_rf && (advanced_properties.center_frequency_keywords == CF_KW_OPERATIONS::COL_CHAN || advanced_properties.center_frequency_keywords == CF_KW_OPERATIONS::COL)){
        size_t cl  =  property_sri.keywords.length();
        property_sri.keywords.length(cl+1);
        property_sri.keywords[cl].id = "COL_RF";
        property_sri.keywords[cl].value <<= CORBA::Double(center_frequency_d);
    }
    if(!has_chan_rf && (advanced_properties.center_frequency_keywords == CF_KW_OPERATIONS::COL_CHAN || advanced_properties.center_frequency_keywords == CF_KW_OPERATIONS::CHAN)){
        size_t cl  =  property_sri.keywords.length();
        property_sri.keywords.length(cl+1);
        property_sri.keywords[cl].id = "CHAN_RF";
        property_sri.keywords[cl].value <<= CORBA::Double(center_frequency_d);
    }

    sriChanged = true;
}

////////////////////////////////////////////////////////////////////////////////
//   SOURCE URI VALIDATION -- ALSO POPULATION OF FILE PROCESSING STRUCTURE    // 
////////////////////////////////////////////////////////////////////////////////

bool FileReader_i::populate_file_listing(const std::string& source) {
    file_status.clear();
    if(source_uri == FILE_READER::DEF_SOURCE_URI)
        return true;
    std::vector<ABSTRACTED_FILE_IO::file_listing> fl;
    try{
        fl = filesystem.create_listing_from_source_uri(source);
    } catch (...) {};
    std::string metadata_suffix = ".metadata.xml";
    for (std::vector<ABSTRACTED_FILE_IO::file_listing>::iterator iter = fl.begin(); iter != fl.end(); iter++) {
        // Check if metadata file and ignore if so
        if (advanced_properties.use_metadata_file && iter->filename_full.length() >= metadata_suffix.length() ) {
            if (iter->filename_full.compare(iter->filename_full.length()-metadata_suffix.length(), metadata_suffix.length(), metadata_suffix) == 0)
                continue;
        }
        file_status_struct_struct new_fs;
        new_fs.filename = iter->filename_full;
        new_fs.file_basename = iter->filename_basename;
        new_fs.file_size = iter->file_size;
        new_fs.filesystem_type = iter->file_system_type;
        new_fs.format.clear();
        new_fs.error_msg.clear();
        file_status.push_back(new_fs);
    }
    return !fl.empty();
}


////////////////////////////////////////////////////////////////////////////////
//                     READ AHEAD (CACHING) THREADS                           // 
////////////////////////////////////////////////////////////////////////////////

void FileReader_i::read_ahead_thread() {
    LOG_TRACE(FileReader_i,"FileReader_i::read_ahead_thread - ENTER");

    std::string opened_file;

    try{
        // Buffer File
        do {
            LOG_TRACE(FileReader_i,"FileReader_i::read_ahead_thread do-loop while looping");
            exclusive_lock lock(file_listing_lock);
            // Main Thread Loop
            boost::this_thread::interruption_point();
            for (std::vector<file_status_struct_struct>::iterator fs_iter = file_status.begin(); fs_iter != file_status.end(); fs_iter++) {
                LOG_TRACE(FileReader_i,"FileReader_i::read_ahead_thread for loop with file: "<<fs_iter->filename<<" and error: "<<fs_iter->error_msg);

                //File Loop
                boost::this_thread::interruption_point();
                if (!fs_iter->error_msg.empty()) {
                    LOG_TRACE(FileReader_i,"FileReader_i::read_ahead_thread for loop, file has error, continuing!");
                    continue;
                }
                if (!filesystem.open_file(fs_iter->filename, false)) {
                    std::string error_msg = "ERROR: Cannot open data file:  " + std::string(fs_iter->filename);
                    //std::cout << error_msg << std::endl;
                    LOG_ERROR(FileReader_i,error_msg);
                    fs_iter->error_msg = error_msg;
                    continue;
                }
                opened_file = fs_iter->filename;
                if (advanced_properties.use_metadata_file) {
                    // Check that Metadata file is present
                    fs_iter->metadata_filename = fs_iter->filename+ ".metadata.xml";
                    if (!filesystem.open_file(fs_iter->metadata_filename, false)) {
                        std::string error_msg = "ERROR: Cannot open metadata file:  " + std::string(fs_iter->metadata_filename);
                        //std::cout << error_msg << std::endl;
                        LOG_ERROR(FileReader_i,error_msg);
                        fs_iter->error_msg = error_msg;
                        filesystem.close_file(fs_iter->filename);
                        opened_file = "";
                        continue;
                    }
                    std::vector<char> metadata_xml_buffer;
                    unsigned int metadata_file_size = filesystem.file_size(fs_iter->metadata_filename);
                    if (metadata_file_size < 10000000) {
                        // Read Metdata file and parse
                        metadata_xml_buffer.reserve(metadata_file_size);
                        bool success = filesystem.read(fs_iter->metadata_filename, &metadata_xml_buffer, metadata_file_size);
                        LOG_TRACE(FileReader_i,"FileReader_i::read_ahead_thread - parsing metadata - read success="<<success<<" file size="<<metadata_file_size<<" xml buffer size="<<metadata_xml_buffer.size());
                        MetaDataParser_i->parseData(metadata_xml_buffer);
                        //MetaDataParser(metadataQueue,&packetSizeQueue).parseData(metadata_xml_buffer);
                        filesystem.close_file(fs_iter->metadata_filename);
                    } else {
                        //TODO - If metadata file is large read and parse in chucks.
                        std::string error_msg = "ERROR: Currently don't support metadata files larger than 10 Megabytes:  " + std::string(fs_iter->metadata_filename);
                        //std::cout << error_msg << std::endl;
                        LOG_ERROR(FileReader_i,error_msg);
                        fs_iter->error_msg = error_msg;
                        filesystem.close_file(fs_iter->metadata_filename);
                        filesystem.close_file(fs_iter->filename);
                        opened_file = "";
                        continue;
                    }
                } //Done with metdatafile

                bool first_packet = true;
                bool last_packet = false;
                unsigned long long read_bytes = fs_iter->file_size;

                SUPPORTED_DATA_TYPE::data_description* dd_ptr = dth.get_dt_descriptor(file_format);
                do {
                    boost::this_thread::interruption_point();
                    shared_ptr_file_packet pkt = available_file_packets.pop();
                    pkt->NO_MORE_DATA = false;
                    pkt->file_name = fs_iter->filename;
                    pkt->file_basename = fs_iter->file_basename;
                    pkt->file_size = fs_iter->file_size;
                    pkt->data_format = "";
                    // If first packet, process any headers
                    pkt->first_packet = first_packet;
                    pkt->valid_sri = false;
                    pkt->sri.keywords.length(0);
                    pkt->valid_timestamp = false;
                    pkt->sample_rate = 0;
                    if (first_packet) {
                        filesystem.file_seek(fs_iter->filename,0);
                        running_read_total = 0;

                        if (file_format == "BLUEFILE") {
                            filesystem.read(fs_iter->filename, &pkt->dataBuffer, BLUEFILE_BLOCK_SIZE);
                            blue::HeaderControlBlock hdr;

                            if (!process_bluefile_fixedheader(pkt, &hdr)) {
                                std::string error_msg = "ERROR: BLUE FILE FIXED HEADER IS INVALID FOR FILE:  " + std::string(fs_iter->filename);
                                //std::cout << error_msg << std::endl;
                                LOG_ERROR(FileReader_i,error_msg);
                                fs_iter->error_msg = error_msg;
                                available_file_packets.push(pkt); // recycle pkt
                                break;
                            }

                            // Check whether or not the data is real for determining
                            // COL_RF and IF to adhere to FS/4 rule
                            bool isReal = true;

                            if (hdr.getFormatCode()[0] == 'C') {
                                isReal = false;
                            }

                            filesystem.file_seek(fs_iter->filename, (unsigned long long) hdr.getExtStart() * BLUEFILE_BLOCK_SIZE);
                            filesystem.read(fs_iter->filename, &pkt->dataBuffer, hdr.getExtSize());
                            blue::ExtendedHeader e_hdr;
                            bool byteSwap = hdr.isHeaderEndianceReversed();
                            if (!process_bluefile_extendedheader(pkt, &e_hdr, isReal, byteSwap)) {
                                std::string error_msg = "ERROR: BLUE FILE EXTENDED HEADER IS INVALID FOR FILE:  " + std::string(fs_iter->filename);
                                //std::cout << error_msg << std::endl;
                                LOG_ERROR(FileReader_i,error_msg);
                                fs_iter->error_msg = error_msg;
                                available_file_packets.push(pkt); // recycle pkt
                                break;
                            }

                            filesystem.file_seek(fs_iter->filename, hdr.getDataStart());
                            read_bytes = hdr.getDataSize();
                        } else if (file_format == "WAV") {
                            filesystem.read(fs_iter->filename, & pkt->dataBuffer, WAV_HEADER_READ_SIZE);
                            WAV_HELPERS::wav_file_header wfh;
                            if (!process_wav_header(pkt, &wfh)) {
                                std::string error_msg = "ERROR: WAV FILE HEADER IS INVALID FOR FILE:  " + std::string(fs_iter->filename);
                                //std::cout << error_msg << std::endl;
                                LOG_ERROR(FileReader_i,error_msg);
                                fs_iter->error_msg = error_msg;
                                available_file_packets.push(pkt); // recycle pkt
                                break;
                            }
                            filesystem.file_seek(fs_iter->filename, sizeof (wfh));
                            read_bytes = wfh.data.chunk_length;
                        }
                        first_packet = false;
                        if (!pkt->data_format.empty())
                            dd_ptr = dth.get_dt_descriptor(pkt->data_format);
                    }
                    if (dd_ptr == NULL) {
                        std::string error_msg = "ERROR: DATA FORMAT IS INVALID FOR FILE:  " + std::string(fs_iter->filename);
                        //std::cout << error_msg << std::endl;
                        LOG_ERROR(FileReader_i,error_msg);
                        fs_iter->error_msg = error_msg;
                        available_file_packets.push(pkt); // recycle pkt
                        break;
                    }
                    fs_iter->format = dd_ptr->id;

                    pkt->file_position = filesystem.file_tell(fs_iter->filename);

                    size_t pkt_size = std::max(dd_ptr->bytes_per_sample,size_t(dd_ptr->bytes_per_sample*std::floor(packet_size/dd_ptr->bytes_per_sample)));
                    size_t read_size = size_t(std::min((unsigned long long )pkt_size,read_bytes));
                    // Adjust read_size if metadata mode and not reading BLUE or WAV file
                    if (advanced_properties.use_metadata_file && !(file_format=="BLUEFILE" || file_format=="WAV")) {
                        if (!packetSizeQueue.empty()) {
                            read_size = packetSizeQueue.front();
                            packetSizeQueue.pop();
                            if (read_bytes < read_size) {
                                LOG_ERROR(FileReader_i,"Metadata and data files do not match! Not enough data remaining for all metadata packets. ("<<std::string(fs_iter->filename)<<")");
                                std::string error_msg = "ERROR: Metadata and data files do not match! Not enough data remaining for all metadata packets." + std::string(fs_iter->filename);
                                //std::cout << error_msg << std::endl;
                                fs_iter->error_msg = error_msg;
                                //read_size = read_bytes;
                                available_file_packets.push(pkt); // recycle pkt
                                break;
                            }
                            if (read_size > (unsigned long)packet_size) {
                                pkt->dataBuffer.reserve(read_size);
                            }
                        } else {
                            LOG_ERROR(FileReader_i,"Metadata and data files do not match! Data remains after processing all metadata packets. ("<<std::string(fs_iter->filename)<<")");
                            std::string error_msg = "ERROR: Metadata and data files do not match! Data remains after processing all metadata packets." + std::string(fs_iter->filename);
                            //std::cout << error_msg << std::endl;
                            fs_iter->error_msg = error_msg;
                            available_file_packets.push(pkt); // recycle pkt
                            break;
                        }
                    }

                    bool eos = ! filesystem.read(fs_iter->filename, & pkt->dataBuffer, read_size);

                    pkt->start_sample = running_read_total/dd_ptr->bytes_per_sample;
                    running_read_total += read_size;
                    pkt->stop_sample = running_read_total/dd_ptr->bytes_per_sample - 1;

                    read_bytes -= pkt->dataBuffer.size();
                    last_packet = eos || (read_bytes <= 0);
                    pkt->last_packet = last_packet;
                    if(advanced_properties.looping && advanced_properties.looping_suppress_eos_until_stop)
                        pkt->last_packet = false;

                    // Convert to BULKIO Data format

                    // Byte swap
                    //   - DO NOT: if single-byte data type
                    //   - DO:     if explicitly set to byte swap (Not used in FileReader, but here for completeness)
                    //   - DO:     if File IS (true) Big Endian and BulkIO output IS NOT (false) Big Endian (true^false=true)
                    //   - DO      if File IS NOT (false) Big Endian and BulkIO output IS (true) Big Endian (false^true=true)
                    if (dd_ptr->endian == SUPPORTED_DATA_TYPE::_keep_endianess_) {
                    	// Do nothing -- this is for single-byte data types where byte swapping is N/A
                    } else if (dd_ptr->endian == SUPPORTED_DATA_TYPE::_byte_swap_ ||
                    	      ((dd_ptr->endian == SUPPORTED_DATA_TYPE::_big_endian_) ^ (BULKIO_BYTE_ORDER == BIG_ENDIAN))) {
                        pkt->dataBuffer.resize(2.0 * std::ceil(float(pkt->dataBuffer.size())/2.0));
                        if (dd_ptr->bytes_per_element == sizeof (uint16_t)) {
                            std::vector<uint16_t> *svp = (std::vector<uint16_t> *) & pkt->dataBuffer;
                            std::transform(svp->begin(), svp->end(), svp->begin(), Byte_Swap16<uint16_t>);
                        } else if (dd_ptr->bytes_per_element == sizeof (uint32_t)) {
                            std::vector<uint32_t> *svp = (std::vector<uint32_t> *) & pkt->dataBuffer;
                            std::transform(svp->begin(), svp->end(), svp->begin(), Byte_Swap32<uint32_t>);
                        } else if (dd_ptr->bytes_per_element == sizeof (uint64_t)) {
                            std::vector<uint64_t> *svp = (std::vector<uint64_t> *) & pkt->dataBuffer;
                            std::transform(svp->begin(), svp->end(), svp->begin(), Byte_Swap32<uint64_t>);
                        }
                    }
                    used_file_packets.push(pkt);
                } while (!last_packet);
                opened_file = "";
                filesystem.close_file(fs_iter->filename);
            }
        } while (advanced_properties.looping);

        shared_ptr_file_packet pkt = available_file_packets.pop();
        pkt->NO_MORE_DATA = true;
        used_file_packets.push(pkt);
    } catch (boost::thread_interrupted&){
        if (!opened_file.empty())
            filesystem.close_file(opened_file);
    };
}


bool FileReader_i::process_bluefile_fixedheader(shared_ptr_file_packet current_packet, blue::HeaderControlBlock* hcb) {
    *hcb = blue::HeaderControlBlock((const blue::hcb_s *) & current_packet->dataBuffer[0]);
    if (hcb->validate(false) != 0) {
        return false;
    }

    std::string format_code = hcb->getFormatCode();
    // mode
    SUPPORTED_DATA_TYPE::mode_enum mode = SUPPORTED_DATA_TYPE::_scalar_;
    if (format_code[0] == 'C')
        mode = SUPPORTED_DATA_TYPE::_complex_;

    // byte order of input file
    // Assume little endian, unless:
    //  - File IS (true) same as host and host IS NOT (false) little endian (true^false=true)
    //  - File IS NOT (false) same as host and host IS (true) little endian (false^true=true)
    SUPPORTED_DATA_TYPE::endian_enum endian = SUPPORTED_DATA_TYPE::_little_endian_;
    if ((hcb->getDataEndiance() == blue::IEEE) ^ (BYTE_ORDER == LITTLE_ENDIAN))
        endian = SUPPORTED_DATA_TYPE::_big_endian_;

    // format
    SUPPORTED_DATA_TYPE::data_type_enum dte = SUPPORTED_DATA_TYPE::_16t_;
    if (format_code[1] == 'B' || format_code[1] == 'A') {
        dte = SUPPORTED_DATA_TYPE::_8t_;
        // byte-order N/A to single-byte types
        endian = SUPPORTED_DATA_TYPE::_keep_endianess_;
    } else if (format_code[1] == 'I') {
        dte = SUPPORTED_DATA_TYPE::_16t_;
    } else if (format_code[1] == 'U') {
        dte = SUPPORTED_DATA_TYPE::_16o_;
    } else if (format_code[1] == 'L') {
        dte = SUPPORTED_DATA_TYPE::_32t_;
    } else if (format_code[1] == 'V') {
        dte = SUPPORTED_DATA_TYPE::_32o_;
    } else if (format_code[1] == 'X') {
        dte = SUPPORTED_DATA_TYPE::_64t_;
    } else if (format_code[1] == 'F') {
        dte = SUPPORTED_DATA_TYPE::_32f_;
    } else if (format_code[1] == 'D') {
        dte = SUPPORTED_DATA_TYPE::_64f_;
    }

    current_packet->data_format = dth.get_identifier(dte, mode, endian);

    current_packet->valid_sri = true;
    current_packet->sri.hversion = std::atol(hcb->getHdrVersion().c_str());
    current_packet->sri.mode = short(mode == SUPPORTED_DATA_TYPE::_complex_);
    current_packet->sri.streamID = current_packet->file_basename.c_str();
    current_packet->sri.subsize = hcb->getColRecs();
    current_packet->sri.xstart = hcb->getXstart();
    current_packet->sri.xdelta = hcb->getXdelta();
    current_packet->sri.xunits = hcb->getXunits();
    current_packet->sri.ystart = hcb->getColStart();
    current_packet->sri.ydelta = hcb->getColDelta();
    current_packet->sri.yunits = hcb->getColUnits();
    current_packet->sri.blocking = default_sri.blocking;

    current_packet->valid_timestamp = false;
    // TO DO: Convert from J1950 -- hcb->getTimeCode()

    if (current_packet->valid_sri) {
        current_packet->sample_rate = 1.0 / current_packet->sri.xdelta;
    }

    std::vector<std::string> kwn = hcb->getKeywordNames();
    double tc_prec = 0;
    for (std::vector<std::string>::iterator keyIter = kwn.begin(); keyIter != kwn.end(); keyIter++) {
        // if TC_PREC, store value for timestamp creation and DO NOT add to SRI keywords
        if ( *keyIter == "TC_PREC") {
            tc_prec = std::atof(hcb->getKeywordValue(*keyIter).c_str());
            LOG_DEBUG(FileReader_i,"Using TC_PREC keyword in BLUE file header for extra timecode precision ("<<tc_prec<<".");
            LOG_DEBUG(FileReader_i,"TC_PREC keyword in BLUE file header is NOT added to SRI keywords.");
            continue;
        }
        size_t cur_kw_size = current_packet->sri.keywords.length();
        current_packet->sri.keywords.length(cur_kw_size + 1);
        current_packet->sri.keywords[cur_kw_size].id = keyIter->c_str();
        std::string val = hcb->getKeywordValue(*keyIter);
        current_packet->sri.keywords[cur_kw_size].value <<= CORBA::string_dup(val.c_str());
    }

    double tc =  hcb->getTimeCode();
    //No other way to validate time.  tc is in terms of Jan 1 1950
    if(tc > double(TWENTY_YEARS_S) && tc < double(10.0*TWENTY_YEARS_S)){
        current_packet->tstamp =  get_utc_time(2,tc);
        current_packet->tstamp.tfsec += tc_prec;
        current_packet->valid_timestamp = true;
    }




//    size_t block_size = sizeof(hcb);
//    std::cout << "\n\nBLUE FILE HEADER FOR FILE: " <<current_packet->file_name << "\n";
//    std::cout << "\tsizeof(hcb): " << block_size << std::endl;
//    std::cout << "\tHeader Rep: " << hcb->getHeaderRep() << std::endl;
//    std::cout << "\tHeader Endianess (LITTLE): " << (hcb->getHeaderEndiance() == blue::EEEI) << std::endl;
//    std::cout << "\tData Rep: " << hcb->getDataRep() << std::endl;
//    std::cout << "\tData Endianess  (LITTLE): " << (hcb->getDataEndiance() == blue::EEEI) << std::endl;
//    std::cout << "\tTimecode: " << hcb->getTimeCode() << std::endl;
//    std::cout << "\tgetExtStart (Block): " << hcb->getExtStart() << std::endl;
//    std::cout << "\tgetExtStart (Bytes): " << hcb->getExtStart() * block_size << std::endl;
//    std::cout << "\tgetExtSize: " << hcb->getExtSize() << std::endl;
//    std::cout << "\tgetDataSize: " << hcb->getDataSize() << std::endl;
//    std::cout << "\tgetDataStart: " << hcb->getDataStart() << std::endl;
//    std::cout << "\tgetFormatCode: " << hcb->getFormatCode() << std::endl;
//    std::cout << "\tgetElementSize: " << hcb->getElementSize() << std::endl;
//    std::cout << "\tgetKeywordLength: " << hcb->getKeywordLength() << std::endl;
//    std::cout << "\tgetXstart: " << hcb->getXstart() << std::endl;
//    std::cout << "\tgetXdelta: " << hcb->getXdelta() << std::endl;
//    std::cout << "\tgetXunits: " << hcb->getXunits() << std::endl;
//    std::cout << "\tgetColStart: " << hcb->getColStart() << std::endl;
//    std::cout << "\tgetColDelta: " << hcb->getColDelta() << std::endl;
//    std::cout << "\tgetColUnits: " << hcb->getColUnits() << std::endl;
//    std::cout << "\tgetColRecs: " << hcb->getColRecs() << std::endl;
//    std::cout << "\tgetHdrVersion: " << hcb->getHdrVersion() << std::endl;


    return true;

}

bool FileReader_i::process_bluefile_extendedheader(shared_ptr_file_packet current_packet, blue::ExtendedHeader* e_hdr, bool isReal, bool byteSwap) {
    int ret = e_hdr->loadFromBuffer(& current_packet->dataBuffer[0], current_packet->dataBuffer.size(), byteSwap);
    if (ret != 0)
        return false;

    size_t COL_RFIndex = 0;
    CORBA::Double IF = 0;

    bool foundCOL_RF = false;
    bool foundIF = false;

    std::vector<blue::Keyword> blue_keywords;
    e_hdr->getKeywords(&blue_keywords);

    for (std::vector<blue::Keyword>::iterator keyIter = blue_keywords.begin(); keyIter != blue_keywords.end(); keyIter++) {
        // Don't insert IF into SRI keywords
        if (not foundIF && keyIter->getName() == "IF") {
            double tmp;
            keyIter->getValue(&tmp);
            IF = tmp;

            foundIF = true;
            continue;
        }

        size_t cur_kw_size = current_packet->sri.keywords.length();
        current_packet->sri.keywords.length(cur_kw_size + 1);

        if (keyIter->getName() == "COL_RF") {
            COL_RFIndex = cur_kw_size;
            foundCOL_RF = true;
        }

        current_packet->sri.keywords[cur_kw_size].id = keyIter->getName().c_str();
        blue::FormatEnum kw_format = keyIter->getFormat();
        CORBA::Any value;
        if (kw_format == blue::BYTE) {
            char tmp;
            keyIter->getValue(&tmp);
            value <<= CORBA::Any::from_char(tmp);
        } else if (kw_format == blue::OFFSET) {
            CORBA::Octet tmp;
            keyIter->getValue(&tmp);
            value <<= CORBA::Any::from_octet(tmp);
        } else if (kw_format == blue::INTEGER) {
            short tmp;
            keyIter->getValue(&tmp);
            value <<= CORBA::Short(tmp);
        } else if (kw_format == blue::LONG) {
            long tmp;
            keyIter->getValue(&tmp);
            value <<= CORBA::Long(tmp);
        } else if (kw_format == blue::XTENDED) {
            long long tmp;
            keyIter->getValue(&tmp);
            value <<= CORBA::LongLong(tmp);
        } else if (kw_format == blue::FLOAT) {
            float tmp;
            keyIter->getValue(&tmp);
            value <<= CORBA::Float(tmp);
        } else if (kw_format == blue::DOUBLE) {
            double tmp;
            keyIter->getValue(&tmp);
            value <<= CORBA::Double(tmp);
        } else if (kw_format == blue::PACKED || kw_format == blue::ASCII) {
            std::string tmp;
            keyIter->getValue(&tmp);

            // Some Blue/Platinum Files have COL_RF as a string
            // but for SRI, it should be a CORBA::Double
            if (keyIter->getName() != "COL_RF") {
                value <<= CORBA::string_dup(tmp.c_str());
            } else {
                double tmpDouble;
                std::stringstream ss;
                ss << tmp;
                ss >> tmpDouble;
                value <<= CORBA::Double(tmpDouble);
            }
        }
        current_packet->sri.keywords[cur_kw_size].value = value;

        if(std::string(current_packet->sri.keywords[cur_kw_size].id) == "TIME_EPOCH"){
            double time;
            keyIter->getValue(&time);
            current_packet->tstamp =  get_utc_time(1,time);
            current_packet->valid_timestamp = true;
        }
    }

    // Check for adherence to FS/4 rule and if the check
    // fails, enforce it
    if (isReal && foundCOL_RF && foundIF) {
        // Check if the two numbers are within 0.5 Hz of one another
        if (std::abs(IF - (this->sample_rate_d / 4.0)) > 0.5) {
            LOG_INFO(FileReader_i, "The IF center frequency does not adhere to the Fs/4 rule.  Adjusting COL_RF");

            CORBA::Double COL_RF;

            current_packet->sri.keywords[COL_RFIndex].value >>= COL_RF;

            CORBA::Double newCOL_RF = COL_RF - IF + this->sample_rate_d / 4.0;

            current_packet->sri.keywords[COL_RFIndex].value <<= newCOL_RF;
        }
    }
    // Otherwise, if the IF keyword is found, include it in the
    // SRI keywords
    else if (foundIF) {
        size_t cur_kw_size = current_packet->sri.keywords.length();
        current_packet->sri.keywords.length(cur_kw_size + 1);
        current_packet->sri.keywords[cur_kw_size].id = "IF";
        current_packet->sri.keywords[cur_kw_size].value <<= IF;
    }

    return true;
}

bool FileReader_i::process_wav_header(shared_ptr_file_packet current_packet, WAV_HELPERS::wav_file_header *wfh) {
    memcpy(wfh, & current_packet->dataBuffer[0], sizeof (WAV_HELPERS::wav_file_header));
    if (!WAV_HELPERS::is_waveFileHeader_valid(*wfh)) // Note: only accepts little-endian RIFF WAVE files
        return false;

    // Extract Data Type
    current_packet->data_format = SUPPORTED_DATA_TYPE::CHAR;
    if (wfh->format.bits_per_sample == 8 && wfh->format.format_type == 6)
        current_packet->data_format = SUPPORTED_DATA_TYPE::ALAW;
    else if (wfh->format.bits_per_sample == 8 && wfh->format.format_type == 7)
        current_packet->data_format = SUPPORTED_DATA_TYPE::MULAW;
    else if (wfh->format.bits_per_sample == 16)
        current_packet->data_format = SUPPORTED_DATA_TYPE::SHORT_LITTLE_ENDIAN; // Default byte ordering for WAVE files is little endian


    //Extract Format
    current_packet->sample_rate = double(wfh->format.sample_rate);

    // Extract SRI & Keywords
    current_packet->valid_sri = true;
    current_packet->sri.hversion = 1;
    current_packet->sri.mode = 0;
    current_packet->sri.streamID = current_packet->file_basename.c_str();
    current_packet->sri.subsize = 0;
    current_packet->sri.xstart = 0;
    current_packet->sri.xdelta = 1.0 / double(wfh->format.sample_rate);
    current_packet->sri.xunits = 1;
    current_packet->sri.ystart = 0;
    current_packet->sri.ydelta = 0;
    current_packet->sri.yunits = 1;
    current_packet->sri.blocking = default_sri.blocking;
    return true;


}

/***********************************************************************************************

    Basic functionality:

        The service function is called by the serviceThread object (of type ProcessThread).
        This call happens immediately after the previous call if the return value for
        the previous call was NORMAL.
        If the return value for the previous call was NOOP, then the serviceThread waits
        an amount of time defined in the serviceThread's constructor.

    SRI:
        To create a StreamSRI object, use the following code:
                std::string stream_id = "testStream";
                BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);

    Time:
        To create a PrecisionUTCTime object, use the following code:
                BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();


    Ports:

        Data is passed to the serviceFunction through the getPacket call (BULKIO only).
        The dataTransfer class is a port-specific class, so each port implementing the
        BULKIO interface will have its own type-specific dataTransfer.

        The argument to the getPacket function is a floating point number that specifies
        the time to wait in seconds. A zero value is non-blocking. A negative value
        is blocking.  Constants have been defined for these values, bulkio::Const::BLOCKING and
        bulkio::Const::NON_BLOCKING.

        Each received dataTransfer is owned by serviceFunction and *MUST* be
        explicitly deallocated.

        To send data using a BULKIO interface, a convenience interface has been added 
        that takes a std::vector as the data input

        NOTE: If you have a BULKIO dataSDDS port, you must manually call 
              "port->updateStats()" to update the port statistics when appropriate.

        Example:
            // this example assumes that the component has two ports:
            //  A provides (input) port of type bulkio::InShortPort called short_in
            //  A uses (output) port of type bulkio::OutFloatPort called float_out
            // The mapping between the port and the class is found
            // in the component base class header file

            bulkio::InShortPort::dataTransfer *tmp = short_in->getPacket(bulkio::Const::BLOCKING);
            if (not tmp) { // No data is available
                return NOOP;
            }

            std::vector<float> outputData;
            outputData.resize(tmp->dataBuffer.size());
            for (unsigned int i=0; i<tmp->dataBuffer.size(); i++) {
                outputData[i] = (float)tmp->dataBuffer[i];
            }

            // NOTE: You must make at least one valid pushSRI call
            if (tmp->sriChanged) {
                float_out->pushSRI(tmp->SRI);
            }
            float_out->pushPacket(outputData, tmp->T, tmp->EOS, tmp->streamID);

            delete tmp; // IMPORTANT: MUST RELEASE THE RECEIVED DATA BLOCK
            return NORMAL;

        If working with complex data (i.e., the "mode" on the SRI is set to
        true), the std::vector passed from/to BulkIO can be typecast to/from
        std::vector< std::complex<dataType> >.  For example, for short data:

            bulkio::InShortPort::dataTransfer *tmp = myInput->getPacket(bulkio::Const::BLOCKING);
            std::vector<std::complex<short> >* intermediate = (std::vector<std::complex<short> >*) &(tmp->dataBuffer);
            // do work here
            std::vector<short>* output = (std::vector<short>*) intermediate;
            myOutput->pushPacket(*output, tmp->T, tmp->EOS, tmp->streamID);

        Interactions with non-BULKIO ports are left up to the component developer's discretion

    Properties:

        Properties are accessed directly as member variables. For example, if the
        property name is "baudRate", it may be accessed within member functions as
        "baudRate". Unnamed properties are given a generated name of the form
        "prop_n", where "n" is the ordinal number of the property in the PRF file.
        Property types are mapped to the nearest C++ type, (e.g. "string" becomes
        "std::string"). All generated properties are declared in the base class
        (FileReader_base).

        Simple sequence properties are mapped to "std::vector" of the simple type.
        Struct properties, if used, are mapped to C++ structs defined in the
        generated file "struct_props.h". Field names are taken from the name in
        the properties file; if no name is given, a generated name of the form
        "field_n" is used, where "n" is the ordinal number of the field.

        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            //  - A boolean called scaleInput

            if (scaleInput) {
                dataOut[i] = dataIn[i] * scaleValue;
            } else {
                dataOut[i] = dataIn[i];
            }

        Callback methods can be associated with a property so that the methods are
        called each time the property value changes.  This is done by calling 
        addPropertyChangeListener(<property name>, this, &FileReader_i::<callback method>)
        in the constructor.

        Callback methods should take two arguments, both const pointers to the value
        type (e.g., "const float *"), and return void.

        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue

        //Add to FileReader.cpp
        FileReader_i::FileReader_i(const char *uuid, const char *label) :
            FileReader_base(uuid, label)
        {
            addPropertyChangeListener("scaleValue", this, &FileReader_i::scaleChanged);
        }

        void FileReader_i::scaleChanged(const float *oldValue, const float *newValue)
        {
            std::cout << "scaleValue changed from" << *oldValue << " to " << *newValue
                      << std::endl;
        }

        //Add to FileReader.h
        void scaleChanged(const float* oldValue, const float* newValue);


************************************************************************************************/
int FileReader_i::serviceFunction() {
    // Ensure that the configure() and serviceFunction() are thread safe
    exclusive_lock st_lock(service_thread_lock);

    // No need to perform serviceFunction duties if no data is available or the playing state is not selected
    if (playback_state != "PLAY" || used_file_packets.getUsage() <= 0) {
        st_lock.unlock();
        usleep(std::max(size_t(10),std::min(throttle_usleep / 4, size_t(250e3)))); // instead of returning NOOP, i want to control the amount of sleep
        return NORMAL;
    }

    // Retrieve cached data block -- this is a blocking call
    shared_ptr_file_packet pkt = used_file_packets.pop();

    if(pkt->NO_MORE_DATA){
        st_lock.unlock();
        try{
            CF::Properties props;
            props.length(1);
            props[0].id = "playback_state";
            props[0].value <<= CORBA::string_dup("STOP");
            configure(props);
        } catch(...){};
        available_file_packets.push(pkt);
        return NOOP;
    }

    // Grab Metadata
    bool eos = pkt->last_packet;
    bulkio::InLongPort::dataTransfer *metadataPkt = 0;
    size_t metaDataPacketSize=0;
    if (advanced_properties.use_metadata_file) {
        metadataPkt = metadataQueue->getPacket(bulkio::Const::NON_BLOCKING);
        if (not metadataPkt) { // No metadata is available
            LOG_ERROR(FileReader_i,"Configured to use metadata, but got data packet without associated metadata. Discarding data.");
            st_lock.unlock();
            available_file_packets.push(pkt);
            return NORMAL;
        }

        metaDataPacketSize = metadataPkt->dataBuffer[0]; // Bytes of data associated with metadata

        if (pkt->dataBuffer.size() != metaDataPacketSize) {
            LOG_ERROR(FileReader_i, "Size of data associated with metadata ("<<metaDataPacketSize<<" Bytes) does not equal the size of the incoming data packet ("<<pkt->dataBuffer.size()<<" Bytes). Discarding both.");
            st_lock.unlock();
            available_file_packets.push(pkt);
            delete metadataPkt;
            return NORMAL;
        }

        eos = eos || metadataPkt->EOS;
        sriChanged = metadataPkt->sriChanged;
        data_tstamp = metadataPkt->T;
        //current_sri = metadataPkt->SRI; // done below to keep sri code together
    }


    // New stream: every time a file has changed or when the state goes to PLAY
    if (pkt->first_packet) {

        // Always send SRI when files change
        sriChanged = true;

        // File Format
        current_data_format = pkt->data_format;
        if (current_data_format.empty()) {
            current_data_format = file_format;
        }

        // If we are using metadata file mode then the timecode and sri are provided and will be used
        if (!advanced_properties.use_metadata_file) {
            // Need to reconstruct here...
            //    constructed sri depends on current_data_format and
            //    will not reflect current_data_format unless called
            //    here
            reconstruct_property_sri(pkt->sample_rate);

            // TIMESTAMP //
            // Determine timestamp to use: either (1) system time (2) from properties (3) from file header
            //std::string currentSID = std::string(current_sri.streamID);
            //std::map<std::string,loop_info>::iterator os_iter = outstanding_streams.find(currentSID);

            if (pkt->valid_timestamp && !advanced_properties.ignore_header_metadata) {
                data_tstamp = pkt->tstamp;
            } else if (property_tstamp.tcmode >= 0) {
                data_tstamp = property_tstamp;
            } else {
                data_tstamp = get_current_timestamp();
            }
        }
        throttle_tstamp = get_current_timestamp();
    }
    // sriChanged: every time the SRI needs to be updated
    if (sriChanged) {
        sriChanged = false;

        if (advanced_properties.use_metadata_file) {
            //If using Metadata file, the SRI comes from that file
            current_sri = metadataPkt->SRI;
        } else if (pkt->valid_sri && !advanced_properties.ignore_header_metadata) {
            current_sri = pkt->sri;
            if (advanced_properties.append_default_sri_keywords) {
                size_t cur_kw_len = current_sri.keywords.length();
                size_t prop_kw_len = property_sri.keywords.length();
                current_sri.keywords.length(cur_kw_len + prop_kw_len);
                for (size_t cur_pos = cur_kw_len, prop_pos = 0; cur_pos < cur_kw_len + prop_kw_len; cur_pos++, prop_pos++) {
                    current_sri.keywords[cur_pos] = property_sri.keywords[prop_pos];
                }
            }
        } else {
            current_sri = property_sri;
        }

        current_sri.streamID = replace_string(std::string(current_sri.streamID), "%FILE_BASENAME%", pkt->file_basename).c_str();
        if (advanced_properties.debug_output) {
            std::cout << __PRETTY_FUNCTION__ << " pushSRI:: Data Type: ";
            debug_print_sri(current_sri);
        }
        pushSRI(current_sri);

        // Sample rate update
        current_sample_rate = 1.0 / current_sri.xdelta;
        sample_rate_d = current_sample_rate;
        std::ostringstream sampleRateConverter;
        sampleRateConverter << sample_rate_d;
        sample_rate = sampleRateConverter.str();

        // Update throttle
        reset_throttle();
    }


    // Storing some variables for use after pushing the packet back on the available queue (which I want to do before the usleep commands)
    std::string streamID = std::string(current_sri.streamID);
    size_t sent_bytes = pkt->dataBuffer.size();
    //bool eos = pkt->last_packet || metadataPkt->EOS; // done above
    // Pushing data out of the port and placing that packet on the available queue for caching
    if (advanced_properties.debug_output) {
        std::cout << __PRETTY_FUNCTION__ << " pushPacket::  Packet Address: " << (void*) &pkt->dataBuffer[0] << ", Number Bytes: " << pkt->dataBuffer.size() <<
                ", Timestamp(m/s/o/w/f): " << data_tstamp.tcmode << "/" << data_tstamp.tcstatus << "/" << data_tstamp.toff << "/" << (long) data_tstamp.twsec << "/" << data_tstamp.tfsec <<
                ", EOS: " << eos << ", Stream ID: " << streamID << std::endl;
    }
    LOG_DEBUG(FileReader_i, " pushPacket::  Packet Address: " << (void*) &pkt->dataBuffer[0] << ", Number Bytes: " << pkt->dataBuffer.size() <<
            ", Timestamp(m/s/o/w/f): " << data_tstamp.tcmode << "/" << data_tstamp.tcstatus << "/" << data_tstamp.toff << "/" << (long) data_tstamp.twsec << "/" << (double) data_tstamp.tfsec <<
            ", EOS: " << eos << ", Stream ID: " << streamID);
    bool packet_in_range = is_packet_in_range(pkt); // TODO - update this method to be more precise. Currently operates on packet boundaries.
    if (packet_in_range) {
        switch (dth.get_dt_descriptor(current_data_format)->data_type) {
        case SUPPORTED_DATA_TYPE::_8a_: case SUPPORTED_DATA_TYPE::_8u_: case SUPPORTED_DATA_TYPE::_8t_:
            pushPacket((std::vector<char> *) & pkt->dataBuffer, data_tstamp, eos, streamID);
            break;
        case SUPPORTED_DATA_TYPE::_8o_:
            pushPacket((std::vector<unsigned char> *) & pkt->dataBuffer, data_tstamp, eos, streamID);
            break;
        case SUPPORTED_DATA_TYPE::_16o_:
            pushPacket((std::vector<CORBA::UShort> *) & pkt->dataBuffer, data_tstamp, eos, streamID);
            break;
        case SUPPORTED_DATA_TYPE::_16t_:
            pushPacket((std::vector<CORBA::Short> *) & pkt->dataBuffer, data_tstamp, eos, streamID);
            break;
        case SUPPORTED_DATA_TYPE::_32o_:
            pushPacket((std::vector<CORBA::ULong> *) & pkt->dataBuffer, data_tstamp, eos, streamID);
            break;
        case SUPPORTED_DATA_TYPE::_32t_:
            pushPacket((std::vector<CORBA::Long> *) & pkt->dataBuffer, data_tstamp, eos, streamID);
            break;
        case SUPPORTED_DATA_TYPE::_32f_:
            pushPacket((std::vector<CORBA::Float> *) & pkt->dataBuffer, data_tstamp, eos, streamID);
            break;
        case SUPPORTED_DATA_TYPE::_64f_:
            pushPacket((std::vector<CORBA::Double> *) & pkt->dataBuffer, data_tstamp, eos, streamID);
            break;
        case SUPPORTED_DATA_TYPE::_64t_:
            pushPacket((std::vector<CORBA::LongLong> *) & pkt->dataBuffer, data_tstamp, eos, streamID);
            break;
        }
    }
    else if (!eos) {
        // Since this packet did not fall into our desired time range, skip ahead until we find a packet in our range, or the queue is empty
        bool popped = true;
        while (popped) {
            shared_ptr_file_packet newPkt;
            bulkio::InLongPort::dataTransfer *newMPkt = 0;
            popped = used_file_packets.tryPop(newPkt);
            if (popped) {
                if (pkt->NO_MORE_DATA || is_packet_in_range(newPkt)) {
                    used_file_packets.pushFront(newPkt);
                    break;
                } else if (advanced_properties.use_metadata_file) {
                    // Must prune metadata as well
                    newMPkt = metadataQueue->getPacket(bulkio::Const::NON_BLOCKING);
                    if (newMPkt) {
                        eos = newMPkt->EOS || newPkt->last_packet;
                        available_file_packets.push(newPkt);
                        delete newMPkt;
                        newMPkt = 0;
                        if (eos)
                            break;
                    } else {
                        // couldn't prune associated metadata packet, so put data back and break
                        used_file_packets.pushFront(newPkt);
                        break;
                    }
                } else {
                    eos = newPkt->last_packet;
                    available_file_packets.push(newPkt);
                    if (eos)
                        break;
                }
            }
        }
    }

    //Updating the timestamp for coherency
    double whole_sec, fract_sec, frac_whole_sec;
    fract_sec = modf(pkt->dataBuffer.size() / dth.get_dt_descriptor(current_data_format)->bytes_per_sample * current_sri.xdelta, &whole_sec);
    data_tstamp.tfsec += fract_sec;
    data_tstamp.tfsec = modf(data_tstamp.tfsec, &frac_whole_sec);
    data_tstamp.twsec += whole_sec + frac_whole_sec;

    if(eos) {
        outstanding_streams.erase(streamID);
        if (!packet_in_range) {
            // Received a packet with an EOS, however we did not push it out since the packet was not within the time of the
            // desired playback window, so instead push out a packet with no data and an EOS
            pushPacket((std::vector<char> *) &empty_packet_data, data_tstamp, eos, streamID);
        }
    } else
        outstanding_streams.insert(std::make_pair(streamID,loop_info(streamID,data_tstamp)));

    available_file_packets.push(pkt);
    delete metadataPkt;
    st_lock.unlock();

    if (eos) {
        current_sri.streamID = "";
        if (advanced_properties.transition_time > 0)
            usleep(advanced_properties.transition_time * size_t(1e3));
    } else {
        // Throttling management
        if (throttle_usleep > 0 && throttle_rate_Bps > 0 && sent_bytes > 0) {
            // Current sleep
            usleep(std::min(size_t(advanced_properties.max_sleep_time*1e6), throttle_usleep));

            // Update sleep amount
            double timeDiff_from_last_pkt = get_timestamp_difference(throttle_tstamp, get_current_timestamp());
            component_status.estimated_output_rate = long(double(sent_bytes) / timeDiff_from_last_pkt);
            // need a minimum value for usleep becasue if it gets to 0, it will never be able to recover until the next file starts to play
            throttle_usleep = std::max(size_t(10), size_t(double(throttle_usleep) * component_status.estimated_output_rate / double(throttle_rate_Bps)));
            throttle_tstamp = get_current_timestamp();
            if (advanced_properties.debug_output) {
                std::cout << __PRETTY_FUNCTION__ << " output stats:: Throttle rate (Bps): " << throttle_rate_Bps << ", Throttle usleep: " << throttle_usleep <<
                        ", Seconds since last Tx: " << timeDiff_from_last_pkt << ", Estimated Rate (Bps) : " << component_status.estimated_output_rate << std::endl;
            }
        }
    }

    return NORMAL;
}

void  FileReader_i::pushSRI(const BULKIO::StreamSRI& H){
    dataChar_out->pushSRI(H);
    dataOctet_out->pushSRI(H);
    dataUshort_out->pushSRI(H);
    dataShort_out->pushSRI(H);
    dataUlong_out->pushSRI(H);
    dataLong_out->pushSRI(H);
    dataFloat_out->pushSRI(H);
    dataDouble_out->pushSRI(H);
    dataLongLong_out->pushSRI(H);
    dataUlongLong_out->pushSRI(H);
    dataXML_out->pushSRI(H);
}



template <typename DATA_IN_TYPE>
void  FileReader_i::pushPacket(const std::vector<DATA_IN_TYPE> *data, BULKIO::PrecisionUTCTime& T, bool EOS, std::string& streamID)
{
    if (dataChar_out->isActive()) {
        std::vector<char> *ptr = (std::vector<char> *) ((void*) &port_buffer);
        if (typeid (DATA_IN_TYPE) != typeid (CORBA::Char) && advanced_properties.data_type_conversion) {
            dataTypeTransform::convertVectorDataType((std::vector<DATA_IN_TYPE> *) data, ptr,advanced_properties.data_conversion_normalization);
            dataChar_out->pushPacket(*ptr, T, EOS, streamID);
        } else {
            dataChar_out->pushPacket(*((std::vector<char> *) data), T, EOS, streamID);
        }
    }

    if (dataOctet_out->isActive()) {
        std::vector<unsigned char> *ptr = (std::vector<unsigned char> *) ((void*) &port_buffer);
        if (typeid(DATA_IN_TYPE) != typeid(unsigned char) && advanced_properties.data_type_conversion) {
            dataTypeTransform::convertVectorDataType((std::vector<DATA_IN_TYPE> *) data, ptr,advanced_properties.data_conversion_normalization);
            dataOctet_out->pushPacket(*ptr, T, EOS, streamID);
        } else {
            dataOctet_out->pushPacket(*((std::vector<unsigned char> *) data), T, EOS, streamID);
        }
    }

    if (dataShort_out->isActive()) {
        std::vector<short> *ptr = (std::vector<short> *) ((void*) &port_buffer);
        if (typeid (DATA_IN_TYPE) != typeid (short) && advanced_properties.data_type_conversion) {
            dataTypeTransform::convertVectorDataType((std::vector<DATA_IN_TYPE> *)data, ptr, advanced_properties.data_conversion_normalization);
            dataShort_out->pushPacket(*ptr, T, EOS, streamID);
        } else {
            dataShort_out->pushPacket(*((std::vector<short> *) data), T, EOS, streamID);
        }
    }

    if (dataUshort_out->isActive()) {
        std::vector<unsigned short> *ptr = (std::vector<unsigned short> *) ((void*) &port_buffer);
        if (typeid(DATA_IN_TYPE) != typeid(unsigned short) && advanced_properties.data_type_conversion) {
            dataTypeTransform::convertVectorDataType((std::vector<DATA_IN_TYPE> *) data, ptr, advanced_properties.data_conversion_normalization);
            dataUshort_out->pushPacket(*ptr, T, EOS, streamID);
        } else {
            dataUshort_out->pushPacket(*((std::vector<unsigned short> *) data), T, EOS, streamID);
        }
    }

    if (dataLong_out->isActive()) {
        std::vector<CORBA::Long> *ptr = (std::vector<CORBA::Long> *) ((void*) &port_buffer);
        if (typeid(DATA_IN_TYPE) != typeid(CORBA::Long) && advanced_properties.data_type_conversion) {
            dataTypeTransform::convertVectorDataType((std::vector<DATA_IN_TYPE> *) data, ptr, advanced_properties.data_conversion_normalization);
            dataLong_out->pushPacket(*ptr, T, EOS, streamID);
        } else {
            dataLong_out->pushPacket(*((std::vector<CORBA::Long> *) data), T, EOS, streamID);
        }
    }

    if (dataUlong_out->isActive()) {
        std::vector<CORBA::ULong> *ptr = (std::vector<CORBA::ULong> *) ((void*) &port_buffer);
        if (typeid(DATA_IN_TYPE) != typeid(CORBA::ULong) && advanced_properties.data_type_conversion) {
            dataTypeTransform::convertVectorDataType((std::vector<DATA_IN_TYPE> *) data, ptr, advanced_properties.data_conversion_normalization);
            dataUlong_out->pushPacket(*ptr, T, EOS, streamID);
        } else {
            dataUlong_out->pushPacket(*((std::vector<CORBA::ULong> *) data), T, EOS, streamID);
        }
    }

    if (dataLongLong_out->isActive()) {
        std::vector<CORBA::LongLong> *ptr = (std::vector<CORBA::LongLong> *) ((void*) &port_buffer);
        if (typeid(DATA_IN_TYPE) != typeid(CORBA::LongLong) && advanced_properties.data_type_conversion) {
            dataTypeTransform::convertVectorDataType((std::vector<DATA_IN_TYPE> *) data, ptr, advanced_properties.data_conversion_normalization);
            dataLongLong_out->pushPacket(*ptr, T, EOS, streamID);
        } else {
            dataLongLong_out->pushPacket(*((std::vector<CORBA::LongLong> *) data), T, EOS, streamID);
        }
    }

    if (dataUlongLong_out->isActive()) {
        std::vector<CORBA::ULongLong> *ptr = (std::vector<CORBA::ULongLong> *) ((void*) &port_buffer);
        if (typeid(DATA_IN_TYPE) != typeid(CORBA::ULongLong) && advanced_properties.data_type_conversion) {
            dataTypeTransform::convertVectorDataType((std::vector<DATA_IN_TYPE> *) data, ptr, advanced_properties.data_conversion_normalization);
            dataUlongLong_out->pushPacket(*ptr, T, EOS, streamID);
        } else {
            dataUlongLong_out->pushPacket(*((std::vector<CORBA::ULongLong> *) data), T, EOS, streamID);
        }
    }

    if (dataFloat_out->isActive()) {
        std::vector<float> *ptr = (std::vector<float> *) ((void*) &port_buffer);
        if (typeid(DATA_IN_TYPE) != typeid(float) && advanced_properties.data_type_conversion) {
            dataTypeTransform::convertVectorDataType((std::vector<DATA_IN_TYPE> *) data, ptr, advanced_properties.data_conversion_normalization);
            dataFloat_out->pushPacket(*ptr, T, EOS, streamID);
        } else {
            dataFloat_out->pushPacket(*((std::vector<float> *) data), T, EOS, streamID);
        }
    }

    if (dataDouble_out->isActive()) {
        std::vector<double> *ptr = (std::vector<double> *) ((void*) &port_buffer);
        if (typeid(DATA_IN_TYPE) != typeid(double) && advanced_properties.data_type_conversion) {
            dataTypeTransform::convertVectorDataType((std::vector<DATA_IN_TYPE> *) data, ptr, advanced_properties.data_conversion_normalization);
            dataDouble_out->pushPacket(*ptr, T, EOS, streamID);
        } else {
            dataDouble_out->pushPacket(*((std::vector<double> *) data), T, EOS, streamID);
        }
    }

    if (dataXML_out->isActive()) {
        std::vector<char> *ptr = (std::vector<char> *) ((void*) &port_buffer);
        if (typeid(DATA_IN_TYPE) != typeid(char) && advanced_properties.data_type_conversion) {
            dataTypeTransform::convertVectorDataType((std::vector<DATA_IN_TYPE> *) data, ptr, advanced_properties.data_conversion_normalization);
            dataXML_out->pushPacket((const char *) ptr->data(), T, EOS, streamID);
        } else {
            dataXML_out->pushPacket((const char *) data->data(), T, EOS, streamID);
        }
    }
}


/*
template <typename BIO_PORT_TYPE, typename BIO_OUTPUT_SEQ, typename DATA_IN_TYPE, typename DATA_OUT_TYPE, typename NATIVE_DATA_OUT_TYPE>
void FileReader_i::port_push_packet(BIO_PORT_TYPE *port, const std::vector<DATA_IN_TYPE> *data, BULKIO::PrecisionUTCTime& T, bool EOS, std::string& streamID) {
    try {
        size_t maxCorbaXferSizeData_Elements = (CORBA_MAX_TRANSFER_BYTES - 128 - sizeof (BIO_OUTPUT_SEQ) - sizeof (BULKIO::PrecisionUTCTime) - sizeof (CORBA::Boolean) - sizeof (CORBA::WChar) * streamID.length()) / sizeof (DATA_OUT_TYPE);
        maxCorbaXferSizeData_Elements = std::max(1, int(std::floor(maxCorbaXferSizeData_Elements / 2))*2);
        std::vector<NATIVE_DATA_OUT_TYPE> *ptr = (std::vector<NATIVE_DATA_OUT_TYPE> *) ((void*) &port_buffer);
        if (typeid (DATA_IN_TYPE) != typeid (NATIVE_DATA_OUT_TYPE) && advanced_properties.data_type_conversion) {
            dataTypeTransform::convertVectorDataType((std::vector<DATA_IN_TYPE> *) data, ptr, advanced_properties.data_conversion_normalization);
        } else {
            ptr->clear();
            for (size_t i = 0; i < data->size(); i++) {
                ptr->push_back((NATIVE_DATA_OUT_TYPE) data->at(i));
            }
            //            ptr = (std::vector<NATIVE_DATA_OUT_TYPE> *) ((void*) data);
        }
        if (ptr->size() == 0) {
            port->pushPacket(*ptr, T, EOS, streamID.c_str());
        } else {
            BULKIO::PrecisionUTCTime ts = T;
            for (size_t prevPos = 0, xfer_len = 0; prevPos < ptr->size(); prevPos += xfer_len) {
                xfer_len = std::min(ptr->size() - prevPos, maxCorbaXferSizeData_Elements);
                bool lastXfer = prevPos + xfer_len >= ptr->size();
                port->pushPacket(&((*ptr)[prevPos]), xfer_len, ts, EOS && lastXfer, streamID.c_str());
                ts.tcstatus  = BULKIO::TCS_INVALID;
            }
        }
    } catch (...) {
        std::cout << "Call to port_push_packet by BULKIO_dataUber_Out_i failed" << std::endl;
    }
}


template <typename BIO_PORT_TYPE, typename DATA_IN_TYPE, typename DATA_OUT_TYPE>
void FileReader_i::port_push_packet(BIO_PORT_TYPE *port, const std::vector<DATA_IN_TYPE> *data, bool EOS, std::string& streamID) {
    try {
        if((*data)[data->size()] != 0){
            std::string tmp;
            tmp.resize(data->size());
            if(data->size() > 0)
                memcpy(&tmp[0],&(*data)[0],data->size());
            port->pushPacket(tmp.c_str(), EOS, streamID.c_str());
        }
        else
            port->pushPacket((const char*)&(*data)[0], EOS, streamID.c_str());
    } catch (...) {
        std::cout << "** Call to port_push_packet (XML) by BULKIO_dataUber_Out_i failed" << std::endl;
    }
}
*/
bool FileReader_i::is_packet_in_range (shared_ptr_file_packet pkt) {
    double pkt_start_time_relative = pkt->start_sample * current_sri.xdelta;
    double pkt_stop_time_relative = pkt->stop_sample * current_sri.xdelta;
    bool in_range = false;

    if (advanced_properties.enable_time_filtering && current_sri.xdelta > 0. && pkt_start_time_relative <= pkt_stop_time_relative) {
        bool start_time_in_range = packet_time_in_range(pkt_start_time_relative);
        bool stop_time_in_range = packet_time_in_range(pkt_stop_time_relative);

        if (start_time_in_range) {
            if (stop_time_in_range) {
                in_range = true;
            }
            else if (advanced_properties.start_time <= advanced_properties.stop_time) {
                // This packet contains the data associated with the desired stop time (and beyond) allow it for now.
                // TODO in the future resize pkt->dataBuffer to only include desired data
                in_range = true;
            }
        }
        else if (stop_time_in_range) {
            // Packet start time is before desired time range, but end of packet falls into range, so allow it for now
            in_range = true;
        }
        else if (pkt_start_time_relative < advanced_properties.start_time && pkt_stop_time_relative >= advanced_properties.stop_time
                && advanced_properties.stop_time > 0 && advanced_properties.start_time < advanced_properties.stop_time) {
            // TODO in the future resize pkt->dataBuffer to only include desired data
            in_range = true;
        }
    }
    else {
        // Time filtered playback is disabled or the state of a basic variable does not make sense and could throw off all range calculations
        in_range = true;
    }

    return in_range;
}


bool FileReader_i::packet_time_in_range(double pkt_time) {
    // Packet times are in seconds relative to the start of the file
    bool in_range = false;
    if (advanced_properties.start_time <= pkt_time) {
        if (pkt_time <= advanced_properties.stop_time || advanced_properties.stop_time < 0.) {
            in_range = true;
        }
    }
    return in_range;
}
