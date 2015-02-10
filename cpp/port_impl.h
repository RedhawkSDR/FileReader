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

#ifndef PORT_H
#define PORT_H

#include <boost/thread/locks.hpp>
#include <ossie/Port_impl.h>
#include <CF/cf.h>
#include <vector>
#include <utility>
#include <ossie/CF/QueryablePort.h>

class FileReader_base;
class FileReader_i;

#define CORBA_MAX_TRANSFER_BYTES omniORB::giopMaxMsgSize()

// ----------------------------------------------------------------------------------------
// CF_DomainManager_Out_i declaration
// ----------------------------------------------------------------------------------------
class CF_DomainManager_Out_i : public Port_Uses_base_impl, public POA_ExtendedCF::QueryablePort
{
    ENABLE_LOGGING
    public:
        CF_DomainManager_Out_i(std::string port_name, FileReader_base *_parent);
        ~CF_DomainManager_Out_i();

        void configure(const CF::Properties& configProperties);
        void query(CF::Properties& configProperties);
        void registerDevice(CF::Device_ptr registeringDevice, CF::DeviceManager_ptr registeredDeviceMgr);
        void registerDeviceManager(CF::DeviceManager_ptr deviceMgr);
        void unregisterDeviceManager(CF::DeviceManager_ptr deviceMgr);
        void unregisterDevice(CF::Device_ptr unregisteringDevice);
        void installApplication(const char* profileFileName);
        void uninstallApplication(const char* applicationId);
        void registerService(CORBA::Object_ptr registeringService, CF::DeviceManager_ptr registeredDeviceMgr, const char* name);
        void unregisterService(CORBA::Object_ptr unregisteringService, const char* name);
        void registerWithEventChannel(CORBA::Object_ptr registeringObject, const char* registeringId, const char* eventChannelName);
        void unregisterFromEventChannel(const char* unregisteringId, const char* eventChannelName);
        void registerRemoteDomainManager(CF::DomainManager_ptr registeringDomainManager);
        void unregisterRemoteDomainManager(CF::DomainManager_ptr unregisteringDomainManager);
        char* domainManagerProfile();
        CF::DomainManager::DeviceManagerSequence* deviceManagers();
        CF::DomainManager::ApplicationSequence* applications();
        CF::DomainManager::ApplicationFactorySequence* applicationFactories();
        CF::FileManager_ptr fileMgr();
        CF::AllocationManager_ptr allocationMgr();
        char* identifier();
        char* name();
        CF::DomainManager::DomainManagerSequence* remoteDomainManagers();

        ExtendedCF::UsesConnectionSequence * connections() 
        {
            boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
            if (recConnectionsRefresh) {
                recConnections.length(outConnections.size());
                for (unsigned int i = 0; i < outConnections.size(); i++) {
                    recConnections[i].connectionId = CORBA::string_dup(outConnections[i].second.c_str());
                    recConnections[i].port = CORBA::Object::_duplicate(outConnections[i].first);
                }
                recConnectionsRefresh = false;
            }
            ExtendedCF::UsesConnectionSequence_var retVal = new ExtendedCF::UsesConnectionSequence(recConnections);
            // NOTE: You must delete the object that this function returns!
            return retVal._retn();
        };
        void connectPort(CORBA::Object_ptr connection, const char* connectionId)
        {
            boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
            CF::DomainManager_var port = CF::DomainManager::_narrow(connection);
            outConnections.push_back(std::make_pair(port, connectionId));
            active = true;
            recConnectionsRefresh = true;
        };

        void disconnectPort(const char* connectionId)
        {
            boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
            for (unsigned int i = 0; i < outConnections.size(); i++) {
                if (outConnections[i].second == connectionId) {
                    outConnections.erase(outConnections.begin() + i);
                    break;
                }
            }

            if (outConnections.size() == 0) {
                active = false;
            }
            recConnectionsRefresh = true;
        };
        std::vector< std::pair<CF::DomainManager_var, std::string> > _getConnections()
        {
            return outConnections;
        };

    protected:
        FileReader_i *parent;
        std::vector < std::pair<CF::DomainManager_var, std::string> > outConnections;
        ExtendedCF::UsesConnectionSequence recConnections;
        bool recConnectionsRefresh;
};
#endif
