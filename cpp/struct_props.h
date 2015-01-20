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

#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>
#include <CF/cf.h>

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
};

inline bool operator>>= (const CORBA::Any& a, advanced_properties_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("debug_output", props[idx].id)) {
            if (!(props[idx].value >>= s.debug_output)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("packet_size", props[idx].id)) {
            if (!(props[idx].value >>= s.packet_size)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("buffer_size", props[idx].id)) {
            if (!(props[idx].value >>= s.buffer_size)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("looping", props[idx].id)) {
            if (!(props[idx].value >>= s.looping)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("looping_suppress_eos_until_stop", props[idx].id)) {
            if (!(props[idx].value >>= s.looping_suppress_eos_until_stop)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("transition_time", props[idx].id)) {
            if (!(props[idx].value >>= s.transition_time)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("throttle_rate", props[idx].id)) {
            if (!(props[idx].value >>= s.throttle_rate)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("ignore_header_metadata", props[idx].id)) {
            if (!(props[idx].value >>= s.ignore_header_metadata)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("append_default_sri_keywords", props[idx].id)) {
            if (!(props[idx].value >>= s.append_default_sri_keywords)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("data_conversion_normalization", props[idx].id)) {
            if (!(props[idx].value >>= s.data_conversion_normalization)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("data_type_conversion", props[idx].id)) {
            if (!(props[idx].value >>= s.data_type_conversion)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("max_sleep_time", props[idx].id)) {
            if (!(props[idx].value >>= s.max_sleep_time)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("center_frequency_keywords", props[idx].id)) {
            if (!(props[idx].value >>= s.center_frequency_keywords)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("enable_time_filtering", props[idx].id)) {
            if (!(props[idx].value >>= s.enable_time_filtering)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("start_time", props[idx].id)) {
            if (!(props[idx].value >>= s.start_time)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("stop_time", props[idx].id)) {
            if (!(props[idx].value >>= s.stop_time)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const advanced_properties_struct& s) {
    CF::Properties props;
    props.length(16);
    props[0].id = CORBA::string_dup("debug_output");
    props[0].value <<= s.debug_output;
    props[1].id = CORBA::string_dup("packet_size");
    props[1].value <<= s.packet_size;
    props[2].id = CORBA::string_dup("buffer_size");
    props[2].value <<= s.buffer_size;
    props[3].id = CORBA::string_dup("looping");
    props[3].value <<= s.looping;
    props[4].id = CORBA::string_dup("looping_suppress_eos_until_stop");
    props[4].value <<= s.looping_suppress_eos_until_stop;
    props[5].id = CORBA::string_dup("transition_time");
    props[5].value <<= s.transition_time;
    props[6].id = CORBA::string_dup("throttle_rate");
    props[6].value <<= s.throttle_rate;
    props[7].id = CORBA::string_dup("ignore_header_metadata");
    props[7].value <<= s.ignore_header_metadata;
    props[8].id = CORBA::string_dup("append_default_sri_keywords");
    props[8].value <<= s.append_default_sri_keywords;
    props[9].id = CORBA::string_dup("data_conversion_normalization");
    props[9].value <<= s.data_conversion_normalization;
    props[10].id = CORBA::string_dup("data_type_conversion");
    props[10].value <<= s.data_type_conversion;
    props[11].id = CORBA::string_dup("max_sleep_time");
    props[11].value <<= s.max_sleep_time;
    props[12].id = CORBA::string_dup("center_frequency_keywords");
    props[12].value <<= s.center_frequency_keywords;
    props[13].id = CORBA::string_dup("enable_time_filtering");
    props[13].value <<= s.enable_time_filtering;
    props[14].id = CORBA::string_dup("start_time");
    props[14].value <<= s.start_time;
    props[15].id = CORBA::string_dup("stop_time");
    props[15].value <<= s.stop_time;
    a <<= props;
};

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
    return true;
};

inline bool operator!= (const advanced_properties_struct& s1, const advanced_properties_struct& s2) {
    return !(s1==s2);
};

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
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("tcmode", props[idx].id)) {
            if (!(props[idx].value >>= s.tcmode)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("tcstatus", props[idx].id)) {
            if (!(props[idx].value >>= s.tcstatus)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("toff", props[idx].id)) {
            if (!(props[idx].value >>= s.toff)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("twsec", props[idx].id)) {
            if (!(props[idx].value >>= s.twsec)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("tfsec", props[idx].id)) {
            if (!(props[idx].value >>= s.tfsec)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const default_timestamp_struct& s) {
    CF::Properties props;
    props.length(5);
    props[0].id = CORBA::string_dup("tcmode");
    props[0].value <<= s.tcmode;
    props[1].id = CORBA::string_dup("tcstatus");
    props[1].value <<= s.tcstatus;
    props[2].id = CORBA::string_dup("toff");
    props[2].value <<= s.toff;
    props[3].id = CORBA::string_dup("twsec");
    props[3].value <<= s.twsec;
    props[4].id = CORBA::string_dup("tfsec");
    props[4].value <<= s.tfsec;
    a <<= props;
};

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
};

inline bool operator!= (const default_timestamp_struct& s1, const default_timestamp_struct& s2) {
    return !(s1==s2);
};

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
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("hversion", props[idx].id)) {
            if (!(props[idx].value >>= s.hversion)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("xstart", props[idx].id)) {
            if (!(props[idx].value >>= s.xstart)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("xdelta", props[idx].id)) {
            if (!(props[idx].value >>= s.xdelta)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("xunits", props[idx].id)) {
            if (!(props[idx].value >>= s.xunits)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("subsize", props[idx].id)) {
            if (!(props[idx].value >>= s.subsize)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("ystart", props[idx].id)) {
            if (!(props[idx].value >>= s.ystart)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("ydelta", props[idx].id)) {
            if (!(props[idx].value >>= s.ydelta)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("yunits", props[idx].id)) {
            if (!(props[idx].value >>= s.yunits)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("mode", props[idx].id)) {
            if (!(props[idx].value >>= s.mode)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("blocking", props[idx].id)) {
            if (!(props[idx].value >>= s.blocking)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("streamID", props[idx].id)) {
            if (!(props[idx].value >>= s.streamID)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const default_sri_struct& s) {
    CF::Properties props;
    props.length(11);
    props[0].id = CORBA::string_dup("hversion");
    props[0].value <<= s.hversion;
    props[1].id = CORBA::string_dup("xstart");
    props[1].value <<= s.xstart;
    props[2].id = CORBA::string_dup("xdelta");
    props[2].value <<= s.xdelta;
    props[3].id = CORBA::string_dup("xunits");
    props[3].value <<= s.xunits;
    props[4].id = CORBA::string_dup("subsize");
    props[4].value <<= s.subsize;
    props[5].id = CORBA::string_dup("ystart");
    props[5].value <<= s.ystart;
    props[6].id = CORBA::string_dup("ydelta");
    props[6].value <<= s.ydelta;
    props[7].id = CORBA::string_dup("yunits");
    props[7].value <<= s.yunits;
    props[8].id = CORBA::string_dup("mode");
    props[8].value <<= s.mode;
    props[9].id = CORBA::string_dup("blocking");
    props[9].value <<= s.blocking;
    props[10].id = CORBA::string_dup("streamID");
    props[10].value <<= s.streamID;
    a <<= props;
};

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
};

inline bool operator!= (const default_sri_struct& s1, const default_sri_struct& s2) {
    return !(s1==s2);
};

struct component_status_struct {
    component_status_struct ()
    {
        estimated_output_rate_Bps = 0;
        domain_name = "(domainless)";
    };

    static std::string getId() {
        return std::string("component_status");
    };

    CORBA::Long estimated_output_rate_Bps;
    std::string domain_name;
};

inline bool operator>>= (const CORBA::Any& a, component_status_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("estimated_output_rate_Bps", props[idx].id)) {
            if (!(props[idx].value >>= s.estimated_output_rate_Bps)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("domain_name", props[idx].id)) {
            if (!(props[idx].value >>= s.domain_name)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const component_status_struct& s) {
    CF::Properties props;
    props.length(2);
    props[0].id = CORBA::string_dup("estimated_output_rate_Bps");
    props[0].value <<= s.estimated_output_rate_Bps;
    props[1].id = CORBA::string_dup("domain_name");
    props[1].value <<= s.domain_name;
    a <<= props;
};

inline bool operator== (const component_status_struct& s1, const component_status_struct& s2) {
    if (s1.estimated_output_rate_Bps!=s2.estimated_output_rate_Bps)
        return false;
    if (s1.domain_name!=s2.domain_name)
        return false;
    return true;
};

inline bool operator!= (const component_status_struct& s1, const component_status_struct& s2) {
    return !(s1==s2);
};

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
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("id", props[idx].id)) {
            if (!(props[idx].value >>= s.id)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("value", props[idx].id)) {
            if (!(props[idx].value >>= s.value)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("value_type", props[idx].id)) {
            if (!(props[idx].value >>= s.value_type)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const sri_keywords_struct_struct& s) {
    CF::Properties props;
    props.length(3);
    props[0].id = CORBA::string_dup("id");
    props[0].value <<= s.id;
    props[1].id = CORBA::string_dup("value");
    props[1].value <<= s.value;
    props[2].id = CORBA::string_dup("value_type");
    props[2].value <<= s.value_type;
    a <<= props;
};

inline bool operator== (const sri_keywords_struct_struct& s1, const sri_keywords_struct_struct& s2) {
    if (s1.id!=s2.id)
        return false;
    if (s1.value!=s2.value)
        return false;
    if (s1.value_type!=s2.value_type)
        return false;
    return true;
};

inline bool operator!= (const sri_keywords_struct_struct& s1, const sri_keywords_struct_struct& s2) {
    return !(s1==s2);
};

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
};

inline bool operator>>= (const CORBA::Any& a, file_status_struct_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("DCE:b6256a27-33fd-41f4-b302-943a41a35cd3", props[idx].id)) {
            if (!(props[idx].value >>= s.filename)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("DCE:4baa9718-1f21-4f46-865c-aec82b00df91", props[idx].id)) {
            if (!(props[idx].value >>= s.file_basename)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("DCE:13fd6762-6b17-4a94-a594-9a492e804382", props[idx].id)) {
            if (!(props[idx].value >>= s.file_size)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("DCE:6fb595e7-de50-4e17-b5e7-c4b8d87de624", props[idx].id)) {
            if (!(props[idx].value >>= s.filesystem_type)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("DCE:addfe635-bbef-4c9d-83c2-03dd149a3b49", props[idx].id)) {
            if (!(props[idx].value >>= s.format)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("DCE:ebc0a4de-958f-4785-bbe4-03693c34f879", props[idx].id)) {
            if (!(props[idx].value >>= s.error_msg)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const file_status_struct_struct& s) {
    CF::Properties props;
    props.length(6);
    props[0].id = CORBA::string_dup("DCE:b6256a27-33fd-41f4-b302-943a41a35cd3");
    props[0].value <<= s.filename;
    props[1].id = CORBA::string_dup("DCE:4baa9718-1f21-4f46-865c-aec82b00df91");
    props[1].value <<= s.file_basename;
    props[2].id = CORBA::string_dup("DCE:13fd6762-6b17-4a94-a594-9a492e804382");
    props[2].value <<= s.file_size;
    props[3].id = CORBA::string_dup("DCE:6fb595e7-de50-4e17-b5e7-c4b8d87de624");
    props[3].value <<= s.filesystem_type;
    props[4].id = CORBA::string_dup("DCE:addfe635-bbef-4c9d-83c2-03dd149a3b49");
    props[4].value <<= s.format;
    props[5].id = CORBA::string_dup("DCE:ebc0a4de-958f-4785-bbe4-03693c34f879");
    props[5].value <<= s.error_msg;
    a <<= props;
};

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
    return true;
};

inline bool operator!= (const file_status_struct_struct& s1, const file_status_struct_struct& s2) {
    return !(s1==s2);
};

#endif // STRUCTPROPS_H
