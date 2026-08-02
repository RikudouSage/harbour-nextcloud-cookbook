Name:       harbour-nextcloud-cookbook

Summary:    Cookbook
Version:    1.0.4
Release:    1
License:    MIT
URL:        https://github.com/RikudouSage/harbour-nextcloud-cookbook
Source0:    %{name}-%{version}.tar.bz2

%global __provides_exclude_from ^%{_datadir}/%{name}/lib/.*$
%global __requires_exclude_from ^%{_datadir}/%{name}/lib/.*$
%global __requires_exclude ^libcookbook\\.so$|^libcookbook\\.so\\(\\)\\(64bit\\)$

Requires:   sailfishsilica-qt5 >= 0.10.9
Requires:   sailfishsecretsdaemon-secretsplugins-default
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(sailfishsecrets)
BuildRequires:  pkgconfig(sailfishcrypto)
BuildRequires:  desktop-file-utils

%description
A client app for Nextcloud Cookbook.


%prep
%setup -q -n %{name}-%{version}

%build

%qmake5 

%make_build


%install
%qmake5_install

strip --strip-unneeded %{buildroot}%{_datadir}/%{name}/lib/libcookbook.so


desktop-file-install --delete-original         --dir %{buildroot}%{_datadir}/applications                %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
