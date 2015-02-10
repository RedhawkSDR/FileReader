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
#include <ossie/Resource_impl.h>

#include "port_impl.h"
#include <bulkio/bulkio.h>
#include "struct_props.h"

#define NOOP 0
#define FINISH -1
#define NORMAL 1

class FileReader_base;

template < typename TargetClass >
class ProcessThread
{
    public:
        ProcessThread(TargetClass *_target, float _delay) :
            target(_target)
        {
            _mythread = 0;
            _thread_running = false;
            _udelay = (__useconds_t)(_delay * 1000000);
        };

        // kick off the thread
        void start() {
            if (_mythread == 0) {
                _thread_running = true;
                _mythread = new boost::thread(&ProcessThread::run, this);
            }
        };

        // manage calls to target's service function
        void run() {
            int state = NORMAL;
            while (_thread_running and (state != FINISH)) {
                state = target->serviceFunction();
                if (state == NOOP) usleep(_udelay);
            }
        };

        // stop thread and wait for termination
        bool release(unsigned long secs = 0, unsigned long usecs = 0) {
            _thread_running = false;
            if (_mythread)  {
                if ((secs == 0) and (usecs == 0)){
                    _mythread->join();
                } else {
                    boost::system_time waitime= boost::get_system_time() + boost::posix_time::seconds(secs) +  boost::posix_time::microseconds(usecs) ;
                    if (!_mythread->timed_join(waitime)) {
                        return 0;
                    }
                }
                delete _mythread;
                _mythread = 0;
            }
    
            return 1;
        };

        virtual ~ProcessThread(){
            if (_mythread != 0) {
                release(0);
                _mythread = 0;
            }
        };

        void updateDelay(float _delay) { _udelay = (__useconds_t)(_delay * 1000000); };


    private:
        boost::thread *_mythread;
        bool _thread_running;
        TargetClass *target;
        __useconds_t _udelay;
        boost::condition_variable _end_of_run;
        boost::mutex _eor_mutex;
};

class FileReader_base : public Resource_impl
{
    friend class CF_DomainManager_Out_i;

    public:
        FileReader_base(const char *uuid, const char *label);

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        CORBA::Object_ptr getPort(const char* _id) throw (CF::PortSupplier::UnknownPort, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException);

        void loadProperties();

        virtual int serviceFunction() = 0;

    protected:
        ProcessThread<FileReader_base> *serviceThread; 
        boost::mutex serviceThreadLock;

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
        CF_DomainManager_Out_i *DomainManager_out;
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
        void construct();

};
#endif
