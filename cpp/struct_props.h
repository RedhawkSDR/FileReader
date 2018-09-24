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

#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>
#include <CF/cf.h>
#include <ossie/PropertyMap.h>

struct advanced_properties_struct {
    advanced_properties_struct ()
    {
        debug_output = false;
        packet_size = "0";
        buffer_size = "20MB";
        looping = false;
        looping_suppress_eos_until_stop = true;
        transition_time = 0;
        throttle_rate = "%SAMPLE_RATE%";
        ignore_header_metadata = false;
        append_default_sri_keywords = false;
        data_conversion_normalization = true;
        data_type_conversion = true;
        max_sleep_time = 5.0;
        center_frequency_keywords = 0;
        enable_time_filtering = false;
        start_time = 0.0;
        stop_time = -1.0;
        use_metadata_file = false;
    };

    static std::string getId() {
        return std::string("advanced_properties");
    };

    bool debug_output;
    std::string packet_size;
    std::string buffer_size;
    bool looping;
    bool looping_suppress_eos_until_stop;
    CORBA::Long transition_time;
    std::string throttle_rate;
    bool ignore_header_metadata;
    bool append_default_sri_keywords;
    bool data_conversion_normalization;
    bool data_type_conversion;
    double max_sleep_time;
    short center_frequency_keywords;
    bool enable_time_filtering;
    double start_time;
    double stop_time;
    bool use_metadata_file;
};

inline bool operator>>= (const CORBA::Any& a, advanced_properties_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("debug_output")) {
        if (!(props["debug_output"] >>= s.debug_output)) return false;
    }
    if (props.contains("packet_size")) {
        if (!(props["packet_size"] >>= s.packet_size)) return false;
    }
    if (props.contains("buffer_size")) {
        if (!(props["buffer_size"] >>= s.buffer_size)) return false;
    }
    if (props.contains("looping")) {
        if (!(props["looping"] >>= s.looping)) return false;
    }
    if (props.contains("looping_suppress_eos_until_stop")) {
        if (!(props["looping_suppress_eos_until_stop"] >>= s.looping_suppress_eos_until_stop)) return false;
    }
    if (props.contains("transition_time")) {
        if (!(props["transition_time"] >>= s.transition_time)) return false;
    }
    if (props.contains("throttle_rate")) {
        if (!(props["throttle_rate"] >>= s.throttle_rate)) return false;
    }
    if (props.contains("ignore_header_metadata")) {
        if (!(props["ignore_header_metadata"] >>= s.ignore_header_metadata)) return false;
    }
    if (props.contains("append_default_sri_keywords")) {
        if (!(props["append_default_sri_keywords"] >>= s.append_default_sri_keywords)) return false;
    }
    if (props.contains("data_conversion_normalization")) {
        if (!(props["data_conversion_normalization"] >>= s.data_conversion_normalization)) return false;
    }
    if (props.contains("data_type_conversion")) {
        if (!(props["data_type_conversion"] >>= s.data_type_conversion)) return false;
    }
    if (props.contains("max_sleep_time")) {
        if (!(props["max_sleep_time"] >>= s.max_sleep_time)) return false;
    }
    if (props.contains("center_frequency_keywords")) {
        if (!(props["center_frequency_keywords"] >>= s.center_frequency_keywords)) return false;
    }
    if (props.contains("enable_time_filtering")) {
        if (!(props["enable_time_filtering"] >>= s.enable_time_filtering)) return false;
    }
    if (props.contains("start_time")) {
        if (!(props["start_time"] >>= s.start_time)) return false;
    }
    if (props.contains("stop_time")) {
        if (!(props["stop_time"] >>= s.stop_time)) return false;
    }
    if (props.contains("use_metadata_file")) {
        if (!(props["use_metadata_file"] >>= s.use_metadata_file)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const advanced_properties_struct& s) {
    redhawk::PropertyMap props;
 
    props["debug_output"] = s.debug_output;
 
    props["packet_size"] = s.packet_size;
 
    props["buffer_size"] = s.buffer_size;
 
    props["looping"] = s.looping;
 
    props["looping_suppress_eos_until_stop"] = s.looping_suppress_eos_until_stop;
 
    props["transition_time"] = s.transition_time;
 
    props["throttle_rate"] = s.throttle_rate;
 
    props["ignore_header_metadata"] = s.ignore_header_metadata;
 
    props["append_default_sri_keywords"] = s.append_default_sri_keywords;
 
    props["data_conversion_normalization"] = s.data_conversion_normalization;
 
    props["data_type_conversion"] = s.data_type_conversion;
 
    props["max_sleep_time"] = s.max_sleep_time;
 
    props["center_frequency_keywords"] = s.center_frequency_keywords;
 
    props["enable_time_filtering"] = s.enable_time_filtering;
 
    props["start_time"] = s.start_time;
 
    props["stop_time"] = s.stop_time;
 
    props["use_metadata_file"] = s.use_metadata_file;
    a <<= props;
}

inline bool operator== (const advanced_properties_struct& s1, const advanced_properties_struct& s2) {
    if (s1.debug_output!=s2.debug_output)
        return false;
    if (s1.packet_size!=s2.packet_size)
        return false;
    if (s1.buffer_size!=s2.buffer_size)
        return false;
    if (s1.looping!=s2.looping)
        return false;
    if (s1.looping_suppress_eos_until_stop!=s2.looping_suppress_eos_until_stop)
        return false;
    if (s1.transition_time!=s2.transition_time)
        return false;
    if (s1.throttle_rate!=s2.throttle_rate)
        return false;
    if (s1.ignore_header_metadata!=s2.ignore_header_metadata)
        return false;
    if (s1.append_default_sri_keywords!=s2.append_default_sri_keywords)
        return false;
    if (s1.data_conversion_normalization!=s2.data_conversion_normalization)
        return false;
    if (s1.data_type_conversion!=s2.data_type_conversion)
        return false;
    if (s1.max_sleep_time!=s2.max_sleep_time)
        return false;
    if (s1.center_frequency_keywords!=s2.center_frequency_keywords)
        return false;
    if (s1.enable_time_filtering!=s2.enable_time_filtering)
        return false;
    if (s1.start_time!=s2.start_time)
        return false;
    if (s1.stop_time!=s2.stop_time)
        return false;
    if (s1.use_metadata_file!=s2.use_metadata_file)
        return false;
    return true;
}

inline bool operator!= (const advanced_properties_struct& s1, const advanced_properties_struct& s2) {
    return !(s1==s2);
}

struct default_timestamp_struct {
    default_timestamp_struct ()
    {
        tcmode = -1;
        tcstatus = 0;
        toff = 0;
        twsec = 0;
        tfsec = 0;
    };

    static std::string getId() {
        return std::string("default_timestamp");
    };

    short tcmode;
    short tcstatus;
    double toff;
    double twsec;
    double tfsec;
};

inline bool operator>>= (const CORBA::Any& a, default_timestamp_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("tcmode")) {
        if (!(props["tcmode"] >>= s.tcmode)) return false;
    }
    if (props.contains("tcstatus")) {
        if (!(props["tcstatus"] >>= s.tcstatus)) return false;
    }
    if (props.contains("toff")) {
        if (!(props["toff"] >>= s.toff)) return false;
    }
    if (props.contains("twsec")) {
        if (!(props["twsec"] >>= s.twsec)) return false;
    }
    if (props.contains("tfsec")) {
        if (!(props["tfsec"] >>= s.tfsec)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const default_timestamp_struct& s) {
    redhawk::PropertyMap props;
 
    props["tcmode"] = s.tcmode;
 
    props["tcstatus"] = s.tcstatus;
 
    props["toff"] = s.toff;
 
    props["twsec"] = s.twsec;
 
    props["tfsec"] = s.tfsec;
    a <<= props;
}

inline bool operator== (const default_timestamp_struct& s1, const default_timestamp_struct& s2) {
    if (s1.tcmode!=s2.tcmode)
        return false;
    if (s1.tcstatus!=s2.tcstatus)
        return false;
    if (s1.toff!=s2.toff)
        return false;
    if (s1.twsec!=s2.twsec)
        return false;
    if (s1.tfsec!=s2.tfsec)
        return false;
    return true;
}

inline bool operator!= (const default_timestamp_struct& s1, const default_timestamp_struct& s2) {
    return !(s1==s2);
}

struct default_sri_struct {
    default_sri_struct ()
    {
        hversion = 1;
        xstart = 0.0;
        xdelta = 0.0;
        xunits = 1;
        subsize = 0;
        ystart = 0.0;
        ydelta = 0.0;
        yunits = 1;
        mode = -1;
        blocking = true;
        streamID = "%FILE_BASENAME%";
    };

    static std::string getId() {
        return std::string("default_sri");
    };

    CORBA::Long hversion;
    double xstart;
    double xdelta;
    short xunits;
    CORBA::Long subsize;
    double ystart;
    double ydelta;
    short yunits;
    short mode;
    bool blocking;
    std::string streamID;
};

inline bool operator>>= (const CORBA::Any& a, default_sri_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("hversion")) {
        if (!(props["hversion"] >>= s.hversion)) return false;
    }
    if (props.contains("xstart")) {
        if (!(props["xstart"] >>= s.xstart)) return false;
    }
    if (props.contains("xdelta")) {
        if (!(props["xdelta"] >>= s.xdelta)) return false;
    }
    if (props.contains("xunits")) {
        if (!(props["xunits"] >>= s.xunits)) return false;
    }
    if (props.contains("subsize")) {
        if (!(props["subsize"] >>= s.subsize)) return false;
    }
    if (props.contains("ystart")) {
        if (!(props["ystart"] >>= s.ystart)) return false;
    }
    if (props.contains("ydelta")) {
        if (!(props["ydelta"] >>= s.ydelta)) return false;
    }
    if (props.contains("yunits")) {
        if (!(props["yunits"] >>= s.yunits)) return false;
    }
    if (props.contains("mode")) {
        if (!(props["mode"] >>= s.mode)) return false;
    }
    if (props.contains("blocking")) {
        if (!(props["blocking"] >>= s.blocking)) return false;
    }
    if (props.contains("streamID")) {
        if (!(props["streamID"] >>= s.streamID)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const default_sri_struct& s) {
    redhawk::PropertyMap props;
 
    props["hversion"] = s.hversion;
 
    props["xstart"] = s.xstart;
 
    props["xdelta"] = s.xdelta;
 
    props["xunits"] = s.xunits;
 
    props["subsize"] = s.subsize;
 
    props["ystart"] = s.ystart;
 
    props["ydelta"] = s.ydelta;
 
    props["yunits"] = s.yunits;
 
    props["mode"] = s.mode;
 
    props["blocking"] = s.blocking;
 
    props["streamID"] = s.streamID;
    a <<= props;
}

inline bool operator== (const default_sri_struct& s1, const default_sri_struct& s2) {
    if (s1.hversion!=s2.hversion)
        return false;
    if (s1.xstart!=s2.xstart)
        return false;
    if (s1.xdelta!=s2.xdelta)
        return false;
    if (s1.xunits!=s2.xunits)
        return false;
    if (s1.subsize!=s2.subsize)
        return false;
    if (s1.ystart!=s2.ystart)
        return false;
    if (s1.ydelta!=s2.ydelta)
        return false;
    if (s1.yunits!=s2.yunits)
        return false;
    if (s1.mode!=s2.mode)
        return false;
    if (s1.blocking!=s2.blocking)
        return false;
    if (s1.streamID!=s2.streamID)
        return false;
    return true;
}

inline bool operator!= (const default_sri_struct& s1, const default_sri_struct& s2) {
    return !(s1==s2);
}

struct component_status_struct {
    component_status_struct ()
    {
        estimated_output_rate = 0;
        domain_name = "(domainless)";
    };

    static std::string getId() {
        return std::string("component_status");
    };

    CORBA::Long estimated_output_rate;
    std::string domain_name;
};

inline bool operator>>= (const CORBA::Any& a, component_status_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("estimated_output_rate")) {
        if (!(props["estimated_output_rate"] >>= s.estimated_output_rate)) return false;
    }
    if (props.contains("domain_name")) {
        if (!(props["domain_name"] >>= s.domain_name)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const component_status_struct& s) {
    redhawk::PropertyMap props;
 
    props["estimated_output_rate"] = s.estimated_output_rate;
 
    props["domain_name"] = s.domain_name;
    a <<= props;
}

inline bool operator== (const component_status_struct& s1, const component_status_struct& s2) {
    if (s1.estimated_output_rate!=s2.estimated_output_rate)
        return false;
    if (s1.domain_name!=s2.domain_name)
        return false;
    return true;
}

inline bool operator!= (const component_status_struct& s1, const component_status_struct& s2) {
    return !(s1==s2);
}

struct sri_keywords_struct_struct {
    sri_keywords_struct_struct ()
    {
        value_type = "STRING";
    };

    static std::string getId() {
        return std::string("sri_keywords_struct");
    };

    std::string id;
    std::string value;
    std::string value_type;
};

inline bool operator>>= (const CORBA::Any& a, sri_keywords_struct_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("id")) {
        if (!(props["id"] >>= s.id)) return false;
    }
    if (props.contains("value")) {
        if (!(props["value"] >>= s.value)) return false;
    }
    if (props.contains("value_type")) {
        if (!(props["value_type"] >>= s.value_type)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const sri_keywords_struct_struct& s) {
    redhawk::PropertyMap props;
 
    props["id"] = s.id;
 
    props["value"] = s.value;
 
    props["value_type"] = s.value_type;
    a <<= props;
}

inline bool operator== (const sri_keywords_struct_struct& s1, const sri_keywords_struct_struct& s2) {
    if (s1.id!=s2.id)
        return false;
    if (s1.value!=s2.value)
        return false;
    if (s1.value_type!=s2.value_type)
        return false;
    return true;
}

inline bool operator!= (const sri_keywords_struct_struct& s1, const sri_keywords_struct_struct& s2) {
    return !(s1==s2);
}

struct file_status_struct_struct {
    file_status_struct_struct ()
    {
    };

    static std::string getId() {
        return std::string("file_status_struct");
    };

    std::string filename;
    std::string file_basename;
    CORBA::LongLong file_size;
    short filesystem_type;
    std::string format;
    std::string error_msg;
    std::string metadata_filename;
};

inline bool operator>>= (const CORBA::Any& a, file_status_struct_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("DCE:b6256a27-33fd-41f4-b302-943a41a35cd3")) {
        if (!(props["DCE:b6256a27-33fd-41f4-b302-943a41a35cd3"] >>= s.filename)) return false;
    }
    if (props.contains("DCE:4baa9718-1f21-4f46-865c-aec82b00df91")) {
        if (!(props["DCE:4baa9718-1f21-4f46-865c-aec82b00df91"] >>= s.file_basename)) return false;
    }
    if (props.contains("DCE:13fd6762-6b17-4a94-a594-9a492e804382")) {
        if (!(props["DCE:13fd6762-6b17-4a94-a594-9a492e804382"] >>= s.file_size)) return false;
    }
    if (props.contains("DCE:6fb595e7-de50-4e17-b5e7-c4b8d87de624")) {
        if (!(props["DCE:6fb595e7-de50-4e17-b5e7-c4b8d87de624"] >>= s.filesystem_type)) return false;
    }
    if (props.contains("DCE:addfe635-bbef-4c9d-83c2-03dd149a3b49")) {
        if (!(props["DCE:addfe635-bbef-4c9d-83c2-03dd149a3b49"] >>= s.format)) return false;
    }
    if (props.contains("DCE:ebc0a4de-958f-4785-bbe4-03693c34f879")) {
        if (!(props["DCE:ebc0a4de-958f-4785-bbe4-03693c34f879"] >>= s.error_msg)) return false;
    }
    if (props.contains("file_status_struct::metadata_filename")) {
        if (!(props["file_status_struct::metadata_filename"] >>= s.metadata_filename)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const file_status_struct_struct& s) {
    redhawk::PropertyMap props;
 
    props["DCE:b6256a27-33fd-41f4-b302-943a41a35cd3"] = s.filename;
 
    props["DCE:4baa9718-1f21-4f46-865c-aec82b00df91"] = s.file_basename;
 
    props["DCE:13fd6762-6b17-4a94-a594-9a492e804382"] = s.file_size;
 
    props["DCE:6fb595e7-de50-4e17-b5e7-c4b8d87de624"] = s.filesystem_type;
 
    props["DCE:addfe635-bbef-4c9d-83c2-03dd149a3b49"] = s.format;
 
    props["DCE:ebc0a4de-958f-4785-bbe4-03693c34f879"] = s.error_msg;
 
    props["file_status_struct::metadata_filename"] = s.metadata_filename;
    a <<= props;
}

inline bool operator== (const file_status_struct_struct& s1, const file_status_struct_struct& s2) {
    if (s1.filename!=s2.filename)
        return false;
    if (s1.file_basename!=s2.file_basename)
        return false;
    if (s1.file_size!=s2.file_size)
        return false;
    if (s1.filesystem_type!=s2.filesystem_type)
        return false;
    if (s1.format!=s2.format)
        return false;
    if (s1.error_msg!=s2.error_msg)
        return false;
    if (s1.metadata_filename!=s2.metadata_filename)
        return false;
    return true;
}

inline bool operator!= (const file_status_struct_struct& s1, const file_status_struct_struct& s2) {
    return !(s1==s2);
}

#endif // STRUCTPROPS_H
