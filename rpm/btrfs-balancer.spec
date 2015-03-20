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
BuildRequires:  pkgconfig(contextkit-statefs)
BuildRequires:  pkgconfig(keepalive)
BuildRequires:  ssu-devel >= 0.37.9
Requires:       systemd

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

%package sbj-config
Summary: %{summary} (SbJ configuration)
Requires: %{name} = %{version}

%description sbj-config
%{summary}.

%files sbj-config
%attr(644,root,root) %{_datadir}/%{name}/btrfs-sbj.conf

