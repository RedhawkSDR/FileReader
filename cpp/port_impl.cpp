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

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    Source: FileReader.spd.xml

*******************************************************************************************/

#include "FileReader.h"

// ----------------------------------------------------------------------------------------
// CF_DomainManager_Out_i definition
// ----------------------------------------------------------------------------------------
PREPARE_ALT_LOGGING(CF_DomainManager_Out_i,FileReader_i)
CF_DomainManager_Out_i::CF_DomainManager_Out_i(std::string port_name, FileReader_base *_parent) :
Port_Uses_base_impl(port_name)
{
    parent = static_cast<FileReader_i *> (_parent);
    recConnectionsRefresh = false;
    recConnections.length(0);
}

CF_DomainManager_Out_i::~CF_DomainManager_Out_i()
{
}

void CF_DomainManager_Out_i::configure(const CF::Properties& configProperties)
{
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->configure(configProperties);
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to configure by CF_DomainManager_Out_i failed");
            }
        }
    }

    return;
}

void CF_DomainManager_Out_i::query(CF::Properties& configProperties)
{
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->query(configProperties);
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to query by CF_DomainManager_Out_i failed");
            }
        }
    }

    return;
}

void CF_DomainManager_Out_i::registerDevice(CF::Device_ptr registeringDevice, CF::DeviceManager_ptr registeredDeviceMgr)
{
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->registerDevice(registeringDevice, registeredDeviceMgr);
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to registerDevice by CF_DomainManager_Out_i failed");
            }
        }
    }

    return;
}

void CF_DomainManager_Out_i::registerDeviceManager(CF::DeviceManager_ptr deviceMgr)
{
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->registerDeviceManager(deviceMgr);
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to registerDeviceManager by CF_DomainManager_Out_i failed");
            }
        }
    }

    return;
}

void CF_DomainManager_Out_i::unregisterDeviceManager(CF::DeviceManager_ptr deviceMgr)
{
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->unregisterDeviceManager(deviceMgr);
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to unregisterDeviceManager by CF_DomainManager_Out_i failed");
            }
        }
    }

    return;
}

void CF_DomainManager_Out_i::unregisterDevice(CF::Device_ptr unregisteringDevice)
{
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->unregisterDevice(unregisteringDevice);
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to unregisterDevice by CF_DomainManager_Out_i failed");
            }
        }
    }

    return;
}

void CF_DomainManager_Out_i::installApplication(const char* profileFileName)
{
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->installApplication(profileFileName);
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to installApplication by CF_DomainManager_Out_i failed");
            }
        }
    }

    return;
}

void CF_DomainManager_Out_i::uninstallApplication(const char* applicationId)
{
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->uninstallApplication(applicationId);
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to uninstallApplication by CF_DomainManager_Out_i failed");
            }
        }
    }

    return;
}

void CF_DomainManager_Out_i::registerService(CORBA::Object_ptr registeringService, CF::DeviceManager_ptr registeredDeviceMgr, const char* name)
{
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->registerService(registeringService, registeredDeviceMgr, name);
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to registerService by CF_DomainManager_Out_i failed");
            }
        }
    }

    return;
}

void CF_DomainManager_Out_i::unregisterService(CORBA::Object_ptr unregisteringService, const char* name)
{
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->unregisterService(unregisteringService, name);
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to unregisterService by CF_DomainManager_Out_i failed");
            }
        }
    }

    return;
}

void CF_DomainManager_Out_i::registerWithEventChannel(CORBA::Object_ptr registeringObject, const char* registeringId, const char* eventChannelName)
{
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->registerWithEventChannel(registeringObject, registeringId, eventChannelName);
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to registerWithEventChannel by CF_DomainManager_Out_i failed");
            }
        }
    }

    return;
}

void CF_DomainManager_Out_i::unregisterFromEventChannel(const char* unregisteringId, const char* eventChannelName)
{
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->unregisterFromEventChannel(unregisteringId, eventChannelName);
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to unregisterFromEventChannel by CF_DomainManager_Out_i failed");
            }
        }
    }

    return;
}

void CF_DomainManager_Out_i::registerRemoteDomainManager(CF::DomainManager_ptr registeringDomainManager)
{
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->registerRemoteDomainManager(registeringDomainManager);
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to registerRemoteDomainManager by CF_DomainManager_Out_i failed");
            }
        }
    }

    return;
}

void CF_DomainManager_Out_i::unregisterRemoteDomainManager(CF::DomainManager_ptr unregisteringDomainManager)
{
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                ((*i).first)->unregisterRemoteDomainManager(unregisteringDomainManager);
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to unregisterRemoteDomainManager by CF_DomainManager_Out_i failed");
            }
        }
    }

    return;
}

char* CF_DomainManager_Out_i::domainManagerProfile()
{
    char* retval;
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                retval = ((*i).first)->domainManagerProfile();
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to domainManagerProfile by CF_DomainManager_Out_i failed");
            }
        }
    }

    return retval;
}

CF::DomainManager::DeviceManagerSequence* CF_DomainManager_Out_i::deviceManagers()
{
    CF::DomainManager::DeviceManagerSequence* retval;
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                retval = ((*i).first)->deviceManagers();
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to deviceManagers by CF_DomainManager_Out_i failed");
            }
        }
    }

    return retval;
}

CF::DomainManager::ApplicationSequence* CF_DomainManager_Out_i::applications()
{
    CF::DomainManager::ApplicationSequence* retval;
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                retval = ((*i).first)->applications();
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to applications by CF_DomainManager_Out_i failed");
            }
        }
    }

    return retval;
}

CF::DomainManager::ApplicationFactorySequence* CF_DomainManager_Out_i::applicationFactories()
{
    CF::DomainManager::ApplicationFactorySequence* retval;
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                retval = ((*i).first)->applicationFactories();
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to applicationFactories by CF_DomainManager_Out_i failed");
            }
        }
    }

    return retval;
}

CF::FileManager_ptr CF_DomainManager_Out_i::fileMgr()
{
    CF::FileManager_ptr retval;
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                retval = ((*i).first)->fileMgr();
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to fileMgr by CF_DomainManager_Out_i failed");
            }
        }
    }

    return retval;
}

CF::AllocationManager_ptr CF_DomainManager_Out_i::allocationMgr()
{
    CF::AllocationManager_ptr retval;
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                retval = ((*i).first)->allocationMgr();
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to allocationMgr by CF_DomainManager_Out_i failed");
            }
        }
    }

    return retval;
}

char* CF_DomainManager_Out_i::identifier()
{
    char* retval;
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                retval = ((*i).first)->identifier();
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to identifier by CF_DomainManager_Out_i failed");
            }
        }
    }

    return retval;
}

char* CF_DomainManager_Out_i::name()
{
    char* retval;
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                retval = ((*i).first)->name();
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to name by CF_DomainManager_Out_i failed");
            }
        }
    }

    return retval;
}

CF::DomainManager::DomainManagerSequence* CF_DomainManager_Out_i::remoteDomainManagers()
{
    CF::DomainManager::DomainManagerSequence* retval;
    std::vector < std::pair < CF::DomainManager_var, std::string > >::iterator i;

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    if (active) {
        for (i = outConnections.begin(); i != outConnections.end(); ++i) {
            try {
                retval = ((*i).first)->remoteDomainManagers();
            } catch(...) {
                LOG_ERROR(CF_DomainManager_Out_i,"Call to remoteDomainManagers by CF_DomainManager_Out_i failed");
            }
        }
    }

    return retval;
}


