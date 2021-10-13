Name:		btrfs-balancer
Summary:	Automatic balancing service for btrfs filesystem
Version:	1.2.8
Release:	1
License:	BSD
URL:		https://github.com/sailfishos/btrfs-balancer
Source0:	%{name}-%{version}.tar.bz2

BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(systemsettings) >= 0.2.25
BuildRequires:  pkgconfig(keepalive)
BuildRequires:  pkgconfig(systemd)
Requires:       systemd
Requires:       btrfs-balancer-configs
Requires:       util-linux

%description
%{summary}

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5
%make_build

%install
rm -rf %{buildroot}
%qmake5_install

%files
%defattr(-,root,root,-)
/etc/dbus-1/system.d/*
/usr/share/dbus-1/system-services/*
%{_unitdir}/*
%{_sbindir}/*

%package config-example
Summary: Sample device configuration data
Group: System/Base
Provides: btrfs-balancer-configs

%description config-example
%{summary}. A device maker needs to install their device-specific configuration file.

%files config-example
%defattr(-,root,root,-)
/usr/share/btrfs-balancer/btrfs-balancer.conf
