#
# This file is protected by Copyright. Please refer to the COPYRIGHT file distributed with this
# source distribution.
#
# This file is part of REDHAWK Basic Components FileReader.
#
# REDHAWK Basic Components FileReader is free software: you can redistribute it and/or modify it under the terms of
# the GNU Lesser General Public License as published by the Free Software Foundation, either
# version 3 of the License, or (at your option) any later version.
#
# REDHAWK Basic Components FileReader is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
# PURPOSE.  See the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License along with this
# program.  If not, see http://www.gnu.org/licenses/.
#

# By default, the RPM will install to the standard REDHAWK SDR root location (/var/redhawk/sdr)
# You can override this at install time using --prefix /new/sdr/root when invoking rpm (preferred method, if you must)
%{!?_sdrroot: %global _sdrroot /var/redhawk/sdr}
%define _prefix %{_sdrroot}
Prefix:         %{_prefix}

# Point install paths to locations within our target SDR root
%define _sysconfdir    %{_prefix}/etc
%define _localstatedir %{_prefix}/var
%define _mandir        %{_prefix}/man
%define _infodir       %{_prefix}/info

Name:           rh.FileReader
Version:        4.1.0
Release:        1%{?dist}
Summary:        Component %{name}

Group:          REDHAWK/Components
License:        GPLv3+
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:  redhawk-devel >= 2.0
Requires:       redhawk >= 2.0

BuildRequires:  rh.RedhawkDevUtils-devel >= 4.0
Requires:       rh.RedhawkDevUtils >= 4.0
BuildRequires:  rh.blueFileLib-devel >= 2.1
Requires:       rh.blueFileLib >= 2.1

# Interface requirements
BuildRequires:  bulkioInterfaces >= 2.0
Requires:       bulkioInterfaces >= 2.0

# Allow upgrades from previous package name
Obsoletes:      FileReader < 4.0.0

%description
Component %{name}
 * Commit: __REVISION__
 * Source Date/Time: __DATETIME__


%prep
%setup -q


%build
# Implementation cpp
pushd cpp
./reconf
%define _bindir %{_prefix}/dom/components/rh/FileReader/cpp
%configure
make %{?_smp_mflags}
popd


%install
rm -rf $RPM_BUILD_ROOT
# Implementation cpp
pushd cpp
%define _bindir %{_prefix}/dom/components/rh/FileReader/cpp
make install DESTDIR=$RPM_BUILD_ROOT
popd


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,redhawk,redhawk,-)
%dir %{_sdrroot}/dom/components/rh
%dir %{_sdrroot}/dom/components/rh/FileReader
%{_prefix}/dom/components/rh/FileReader/FileReader.scd.xml
%{_prefix}/dom/components/rh/FileReader/FileReader.prf.xml
%{_prefix}/dom/components/rh/FileReader/FileReader.spd.xml
%{_prefix}/dom/components/rh/FileReader/cpp

%changelog
* Wed Jun 21 2017 Ryan Bauman <rbauman@lgsinnovations.com> - 4.0.3-2
- Mass rebuild for REDHAWK 2.1.1

