Name:		btrfs-balancer
Summary:	Automatic balancing service for btrfs filesystem
Version:	0.0.1
Release:	1
Group:		System/Filesystems
License:	BSD
URL:		https://github.com/sailfishos/btrfs-balancer
Source0:	%{name}-%{version}.tar.bz2

BuildRequires:  btrfs-progs-devel
BuildRequires:  pkgconfig(keepalive-glib)
BuildRequires:	pkgconfig(glib-2.0)
BuildRequires:	pkgconfig(dbus-glib-1)

Requires:	systemd

%description
%{summary}

%prep
%setup -q -n %{name}-%{version}

%build
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

%files
%defattr(-,root,root,-)
/lib/systemd/system/*
%{_sbindir}/%{name}
