Name:		btrfs-balancer
Summary:	Automatic balancing service for btrfs filesystem
Version:	1.1.0
Release:	1
Group:		System/Filesystems
License:	BSD
URL:		https://bitbucket.org/jolla/tools-atruncate
Source0:	%{name}-%{version}.tar.bz2

Requires:	systemd
Requires:	libkeepalive-glib-tools >= 1.3.3

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
%{_sbindir}/*

