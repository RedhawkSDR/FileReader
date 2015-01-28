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

#ifndef FILEREADER_IMPL_H
#define FILEREADER_IMPL_H

#include "FileReader_base.h"
#include <vector>
#include <queue>
#include <complex>
#include <set>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <errno.h>
#include <string.h>
#include <uuid/uuid.h>
#include <ossie/prop_helpers.h>
#include <ossie/Resource_impl.h>
#include <omniORB4/CORBA.h>
#include <omniORB4/omniURI.h>
#include <omniORB4/omniORB.h>
//#include <BULKIO/bulkioDataTypes.h>
#include "boost/thread.hpp"
#include <iostream>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <stdio.h>
#include <stdlib.h>

#include <blue/HeaderControlBlock.h>
#include <blue/ExtendedHeader.h>
#include <abstracted_file_io.h>
#include <data_type_descriptor.h>
#include <dataTypeTransform.h>
#include <byte_swap.h>
#include <vectorMagic.h>
#include <rh_utils.h>
#include "Queue.hpp"

//#define _DEBUG_
#define BLUEFILE_BLOCK_SIZE 512   // Exact size of fixed header
#define WAV_HEADER_READ_SIZE 512        // Taking more than I need incase the RIFF headers are out of order
#define PACKET_HEADER_RESERVED 512
#define DEF_PACKET_SIZE 512*1024 + PACKET_HEADER_RESERVED
#define TWENTY_YEARS_S 631152000
#define CORBA_MAX_TRANSFER_BYTES omniORB::giopMaxMsgSize()

namespace FILE_READER {
	const std::string DEF_SOURCE_URI = "file://[path_to_local_file_or_dir] OR sca://[path_to_sca_file_or_dir]";
    enum FILESYSTEM_TYPE {
        LOCAL_FILESYSTEM = 0,
        SCA_FILESYSTEM = 1,
    };

};

namespace CF_KW_OPERATIONS {
	const short NONE = -1;
	const short COL_CHAN = 0;
	const short COL  = 1;
	const short CHAN = 2;
};

namespace WAV_HELPERS {

    struct base_chunk_header {
        char chunk_name[4];
        uint32_t chunk_length;
    };

    struct wav_riff_header : base_chunk_header {
        char riff_type[4];
    };

    struct wav_format_header : base_chunk_header {
        uint16_t format_type;
        uint16_t channel_numbers;
        uint32_t sample_rate;
        uint32_t bytes_per_second;
        uint16_t bytes_per_sample;
        uint16_t bits_per_sample;
    };

    struct wav_data_header : base_chunk_header {
    };

    struct wav_file_header {
        wav_riff_header riff;
        wav_format_header format;
        wav_data_header data;
    };

    inline bool is_waveFileHeader_valid(struct wav_file_header & wfh) {
        if (!strncmp(wfh.riff.chunk_name, "RIFF", 4) && !strncmp(wfh.format.chunk_name, "fmt ", 4) && !strncmp(wfh.data.chunk_name, "data ", 4)) {
            return true;
        }
        return false;
    }

};


namespace FILE_READER_DOMAIN_MGR_HELPERS {

inline CF::DomainManager_var domainManager_id_to_var(std::string id) {
		CF::DomainManager_var domainMgr_var = CF::DomainManager::_nil();
		CosNaming::BindingIterator_var it;
		CosNaming::BindingList_var bl;
		CosNaming::NamingContext_var context = CosNaming::NamingContext::_narrow(ossie::corba::InitialNamingContext());
		context->list(100, bl, it);
		for (unsigned int ii = 0; ii < bl->length(); ++ii) {
			try {
				std::string domString = std::string(bl[ii].binding_name[0].id) + "/" + std::string(bl[ii].binding_name[0].id);
				CosNaming::Name_var cosName = omni::omniURI::stringToName(domString.c_str());
				CORBA::Object_var bobj = context->resolve(cosName);
				domainMgr_var = CF::DomainManager::_narrow(bobj);
				if (id.empty() || id == std::string(domainMgr_var->identifier())){
					return domainMgr_var;
				}
			} catch (...) {};
		}
		return domainMgr_var;
	}

};

struct file_packet {
	bool NO_MORE_DATA;
    // Identifiers for first and last packets (sos and eos)
    bool first_packet;
    bool last_packet;

    // Must be filled in if (first_packet == true) 
    std::string file_name;
    std::string file_basename;
    unsigned long long file_size; // in bytes

    std::string data_format; // Only need to fill in if extracting this information from file header, else set to empty string
    double sample_rate; // Only need to fill in if extracting this information from file header, else set to 0

    // Buffer Information (must be filled in for every buffer read)
    long file_position;
    std::vector<char> dataBuffer;

    // Fill in if extracting sri from file headers. If valid_sri == false, defaults will be used (ie - the sri variable will be ignored)
    bool valid_sri;
    BULKIO::StreamSRI sri;

    // Fill in if extracting timestamp from file headers. If valid_timestamp == false, defaults will be used (ie - the tstamp variable will be ignored)
    bool valid_timestamp;
    BULKIO::PrecisionUTCTime tstamp;

    long start_sample;
    long stop_sample;
};

struct loop_info{
	loop_info(){};
	loop_info(const std::string &  _stream_id, const  BULKIO::PrecisionUTCTime& _tstamp ){
		stream_id = _stream_id;
		tstamp = _tstamp;
	};
	std::string stream_id;
	BULKIO::PrecisionUTCTime tstamp;
};


class FileReader_i;

class FileReader_i : public FileReader_base {
	ENABLE_LOGGING

    typedef boost::mutex::scoped_lock exclusive_lock;
    typedef boost::shared_ptr<file_packet> shared_ptr_file_packet;
    typedef threadsafe::Queue<shared_ptr_file_packet> ts_queue_file_packet;
    friend class BULKIO_dataUber_Out_i;
    friend class BULKIO_dataChar_Out_i;

public:
    FileReader_i(const char *uuid, const char *label);
    ~FileReader_i();
    void start() throw (CF::Resource::StartError, CORBA::SystemException);
    void stop() throw (CF::Resource::StopError, CORBA::SystemException);
    //void configure(const CF::Properties&) throw (CORBA::SystemException, CF::PropertySet::InvalidConfiguration, CF::PropertySet::PartialConfiguration);
    void initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException);


    int serviceFunction();

private:
    // Property change listener methods
    void advanced_propertiesChanged(const advanced_properties_struct *oldValue, const advanced_properties_struct *newValue);
    void source_uriChanged(const std::string *oldValue, const std::string *newValue);
    void file_formatChanged(const std::string *oldValue, const std::string *newValue);
    void sample_rateChanged(const std::string *oldValue, const std::string *newValue);
    void center_frequencyChanged(const std::string *oldValue, const std::string *newValue);
    void playback_stateChanged(const std::string *oldValue, const std::string *newValue);
    void default_timestampChanged(const default_timestamp_struct *oldValue, const default_timestamp_struct *newValue);
    void default_sriChanged(const default_sri_struct *oldValue, const default_sri_struct *newValue);
    void default_sri_keywordsChanged(const std::vector<sri_keywords_struct_struct> *oldValue, const std::vector<sri_keywords_struct_struct> *newValue);

    // Property change listener helper methods
    void restart_read_ahead_caching();

    std::map<std::string,loop_info> outstanding_streams;

    // Ensure that configure() and serviceFunction() are thread safe
    boost::mutex service_thread_lock;

    // On every configure, automatically regenerate sri and timestamp structures based on properties
    BULKIO::StreamSRI property_sri;
    BULKIO::PrecisionUTCTime property_tstamp;

    // Will either be using the property sri/timestamp or ones based on a packet header
    bool sriChanged;
    std::string current_data_format;
    double current_sample_rate;
    BULKIO::StreamSRI current_sri;
    BULKIO::PrecisionUTCTime data_tstamp;

    // Buffer/packet size definitions
    long packet_size; //memory space rounded down to a 128B boundry
    size_t buffer_size;
    double sample_rate_d;
    double center_frequency_d;
    // Throttling - NOTE: Can change on per file basis if reading sample rate from file header (wav)
    BULKIO::PrecisionUTCTime throttle_tstamp;
    size_t throttle_rate_Bps;
    size_t throttle_usleep;

    // Packet Buffer Queues 
    ts_queue_file_packet used_file_packets;
    ts_queue_file_packet available_file_packets;

    // Buffer Thread
    boost::thread *buffer_thread;
    ABSTRACTED_FILE_IO::abstracted_file_io filesystem;
    SUPPORTED_DATA_TYPE::data_type_helper dth;

    long running_read_total;
    std::vector<char> empty_packet_data;

    //////////////////////
    // HELPER FUNCTIONS //
    //////////////////////
    boost::mutex file_listing_lock;
    bool populate_file_listing(const std::string& source);
    void read_ahead_thread();

    void create_data_type_mapping();
    void start_cache_thread();
    void stop_cache_thread();
    void reconstruct_property_sri();
    void reconstruct_property_timestamp();
    void reset_throttle();
    
    bool process_bluefile_fixedheader(shared_ptr_file_packet current_packet, blue::HeaderControlBlock* hcb);
    bool process_bluefile_extendedheader(shared_ptr_file_packet current_packet, blue::ExtendedHeader* e_hdr);
    bool process_wav_header(shared_ptr_file_packet current_packet, WAV_HELPERS::wav_file_header *wfh);


    std::vector<char> port_buffer;
    //template <typename BIO_PORT_TYPE, typename BIO_OUTPUT_SEQ, typename DATA_IN_TYPE, typename DATA_OUT_TYPE, typename NATIVE_DATA_OUT_TYPE>
    //void port_push_packet(BIO_PORT_TYPE *port, const std::vector<DATA_IN_TYPE> *data, BULKIO::PrecisionUTCTime& T, bool EOS, std::string& streamID);
    //template <typename BIO_PORT_TYPE, typename DATA_IN_TYPE, typename DATA_OUT_TYPE>
    //void port_push_packet(BIO_PORT_TYPE *port, const std::vector<DATA_IN_TYPE> *data, bool EOS, std::string& streamID);

    template <typename DATA_IN_TYPE>
    void pushPacket(const std::vector<DATA_IN_TYPE> *data, BULKIO::PrecisionUTCTime& T, bool EOS, std::string& streamID);
    void pushSRI(const BULKIO::StreamSRI& H);


    bool is_packet_in_range(shared_ptr_file_packet);
    bool packet_time_in_range(double);


    inline char _to_upper(const char x) {
        return std::toupper(x);
    }

    inline std::string replace_string(std::string whole_string, const std::string& cur_substr, const std::string& new_substr) {
        try {
            size_t substring_pos = whole_string.find(cur_substr);
            whole_string.erase(substring_pos, cur_substr.size());
            whole_string.insert(substring_pos, new_substr);
        } catch (...) {
        }
        return whole_string;
    }

    inline BULKIO::PrecisionUTCTime get_current_timestamp() {
        struct timeval tmp_time;
        struct timezone tmp_tz;
        gettimeofday(&tmp_time, &tmp_tz);
        double wsec = tmp_time.tv_sec;
        double fsec = tmp_time.tv_usec / 1e6;
        BULKIO::PrecisionUTCTime tstamp = BULKIO::PrecisionUTCTime();
        tstamp.tcmode = BULKIO::TCM_CPU;
        tstamp.tcstatus = (short) 1;
        tstamp.toff = 0.0;
        tstamp.twsec = wsec;
        tstamp.tfsec = fsec;
        return tstamp;
    }

    inline double get_timestamp_difference(BULKIO::PrecisionUTCTime time_1, BULKIO::PrecisionUTCTime time_2) {
        return (time_2.twsec + time_2.tfsec) - (time_1.twsec + time_1.tfsec);
    }

    void toUpper(std::string& s) {
        for (std::string::iterator p = s.begin(); p != s.end(); ++p) {
            *p = toupper(*p);
        }
    }

    inline void debug_print_sri(const BULKIO::StreamSRI &sri) {
        std::cout << "SRI:: " <<
                "  hversion: " << sri.hversion <<
                ", xstart: " << sri.xstart <<
                ", xdelta: " << sri.xdelta <<
                ", xunits: " << sri.xunits <<
                ", subsize: " << sri.subsize <<
                ", ystart: " << sri.ystart <<
                ", ydelta: " << sri.ydelta <<
                ", yunits: " << sri.yunits <<
                ", mode: " << sri.mode <<
                ", streamID: " << sri.streamID <<
                ", blocking: " << sri.blocking <<
                ", KEYWORDS: ";
        for (size_t i = 0; i < sri.keywords.length(); i++) {
            std::cout << "[ID: " << sri.keywords[i].id << ", VALUE: " << ossie::any_to_string(sri.keywords[i].value) << "],";
        }
        std::cout << "\n";
    }

    // Helper functions for keywords

    template <typename CORBAXX> bool addModifyKeyword(BULKIO::StreamSRI *sri, CORBA::String_member id, CORBAXX myValue, bool addOnly = false) {
        CORBA::Any value;
        value <<= myValue;
        unsigned long keySize = sri->keywords.length();
        if (!addOnly) {
            for (unsigned int i = 0; i < keySize; i++) {
                if (!strcmp(sri->keywords[i].id, id)) {
                    sri->keywords[i].value = value;
                    return true;
                }
            }
        }
        sri->keywords.length(keySize + 1);
        if (sri->keywords.length() != keySize + 1)
            return false;
        sri->keywords[keySize].id = CORBA::string_dup(id);
        sri->keywords[keySize].value = value;
        return true;
    }


    // System Time = 0, J170 = 1, J1950 = 2

    inline BULKIO::PrecisionUTCTime get_utc_time(short from_type = 0, double from_time = 0.0) {
        // Tstamp Init
        BULKIO::PrecisionUTCTime tstamp;
        tstamp.tcmode = BULKIO::TCM_CPU;
        tstamp.tcstatus = BULKIO::TCS_INVALID;

        // Might use, depending on type
        double fract, whole;
        fract = modf(from_time, &whole);

        
        if (from_type == 1) {
            tstamp.tcmode = BULKIO::TCM_OFF;
            tstamp.tcstatus = BULKIO::TCS_VALID;
            tstamp.toff = 0.0;
            tstamp.twsec = whole;
            tstamp.tfsec = fract;
        } else if (from_type == 2) {
            tstamp.tcmode = BULKIO::TCM_OFF;
            tstamp.tcstatus = BULKIO::TCS_VALID;
            tstamp.toff = 0.0;
            tstamp.twsec = whole - double(631152000);
            tstamp.tfsec = fract;
        } else {
            struct timeval tmp_time;
            struct timezone tmp_tz;
            gettimeofday(&tmp_time, &tmp_tz);
            double wsec = tmp_time.tv_sec;
            double fsec = tmp_time.tv_usec / 1e6;
            tstamp.tcmode = BULKIO::TCM_CPU;
            tstamp.tcstatus = (short) 1;
            tstamp.toff = 0.0;
            tstamp.twsec = wsec;
            tstamp.tfsec = fsec;
        }
        return tstamp;
    }
};

#endif // FILEREADER_IMPL_H
