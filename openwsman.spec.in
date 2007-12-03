Name:           openwsman
Version:       @PACKAGE_VERSION@
Release:        1
License:        BSD
Url:            http://www.openwsman.org/
Source:         %{name}-%{version}.tar.bz2
BuildRoot:      %{_tmppath}/%{name}-%{version}-build
Group:          Applications/Management
BuildRequires:  sblim-sfcc-devel curl-devel pkgconfig libxml2-devel pam-devel gcc-c++
Requires:       sblim-sfcc curl libxml2 openssl pam
Summary:        Opensource Implementation of WS-Management


%description
Opensource Implementation of WS-Management


%package devel
Summary:        Openwsman Development files
Group:          Applications/Management
Requires:       openwsman
%description devel
Openwsman Development files



%prep
%setup

%build
%configure --with-shttpd --disable-more-warnings
make

%install
make DESTDIR=%{buildroot} install
install -c -m 644 etc/openwsman.conf $RPM_BUILD_ROOT/etc/openwsman
install -c -m 644 etc/ssleay.cnf $RPM_BUILD_ROOT/etc/openwsman

%clean
rm -rf "$RPM_BUILD_ROOT"

%files
%defattr(-,root,root)
%{_libdir}/*.so*
%{_sbindir}/openwsmand
%dir %{_libdir}/openwsman
%dir %{_libdir}/openwsman/plugins
%{_libdir}/openwsman/plugins/*so*
%dir %{_libdir}/openwsman/authenticators
%{_libdir}/openwsman/authenticators/*so*
%dir /etc/openwsman
%config /etc/openwsman/openwsman.conf
/etc/openwsman/owsmangencert.sh
/etc/openwsman/ssleay.cnf


%files devel
%defattr(-,root,root)
%{_includedir}/*
%{_libdir}/*.a
%{_libdir}/*.la
%{_libdir}/openwsman/authenticators/*.a
%{_libdir}/openwsman/authenticators/*.la
%{_libdir}/openwsman/plugins/*.a
%{_libdir}/openwsman/plugins/*.la
%{_libdir}/pkgconfig/openwsman.pc
%{_libdir}/pkgconfig/openwsman++.pc
%{_libdir}/pkgconfig/openwsman-server.pc

