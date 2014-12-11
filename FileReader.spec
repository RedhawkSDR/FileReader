#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK.
#
# REDHAWK is free software: you can redistribute it and/or modify it
# under the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# REDHAWK is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
# for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

# By default, the RPM will install to the standard REDHAWK SDR root location (/var/redhawk/sdr)
# You can override this at install time using --prefix /new/sdr/root when invoking rpm (preferred method, if you must)
%{!?_sdrroot: %define _sdrroot /var/redhawk/sdr}
%define _prefix %{_sdrroot}
Prefix:         %{_prefix}

Name:           FileReader
Version:        3.0.0
Release:        1%{?dist}
Summary:        Component %{name}

Group:          REDHAWK/Components
License:        None
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  redhawk-devel >= 1.10
Requires:       redhawk >= 1.10

# Interface requirements
BuildRequires:  bulkioInterfaces >= 1.10
Requires:       bulkioInterfaces >= 1.10

BuildRequires:  RedhawkDevUtils_v1-devel >= 3.0.0
Requires:       RedhawkDevUtils_v1 >= 3.0.0

BuildRequires:  blueFileLib_v1-devel >= 1.0.0
Requires:       blueFileLib_v1 >= 1.0.0

%description
Component %{name}
%prep
%setup -q
%build
# Implementation cpp
pushd cpp
./reconf
%define _bindir %{_prefix}/dom/components/FileReader/cpp
%configure
make %{?_smp_mflags}
popd


%install
rm -rf $RPM_BUILD_ROOT
# Implementation cpp
pushd cpp
%define _bindir %{_prefix}/dom/components/FileReader/cpp
make install DESTDIR=$RPM_BUILD_ROOT
popd


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,redhawk,redhawk,-)
%dir %{_prefix}/dom/components/%{name}
%{_prefix}/dom/components/%{name}/FileReader.scd.xml
%{_prefix}/dom/components/%{name}/FileReader.prf.xml
%{_prefix}/dom/components/%{name}/FileReader.spd.xml
%{_prefix}/dom/components/%{name}/cpp

