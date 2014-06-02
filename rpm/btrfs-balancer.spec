Name:		btrfs-balancer
Summary:	Automatic balancing service for btrfs filesystem
Version:	0.0.1
Release:	1
Group:		System/Filesystems
License:	BSD
URL:		https://bitbucket.org/jolla/tools-atruncate
Source0:	%{name}-%{version}.tar.bz2

Requires:	systemd
Requires:	keepalive

%description
%{summary}

%package -n keepalive
Summary:	Command line tool to keep CPU alive
BuildRequires:	pkgconfig
BuildRequires:	glib2-devel
BuildRequires:	dbus-glib-devel
BuildRequires:	mce-headers
Requires:	glib2
Requires:	dbus-glib
Requires:	mce

%description -n keepalive
%{summary}.

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

%files -n keepalive
%defattr(-,root,root,-)
%{_bindir}/*


