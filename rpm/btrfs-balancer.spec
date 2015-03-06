Name:		btrfs-balancer
Summary:	Automatic balancing service for btrfs filesystem
Version:	1.2.0
Release:	1
Group:		System/Filesystems
License:	BSD
URL:		https://github.com/sailfishos/btrfs-balancer
Source0:	%{name}-%{version}.tar.bz2

BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(keepalive)
Requires:	systemd

%description
%{summary}

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%qmake5_install

%files
%defattr(-,root,root,-)
/etc/dbus-1/system.d/*
/usr/share/dbus-1/system-services/*
/lib/systemd/system/*
%{_sbindir}/*

