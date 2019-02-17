# .SPEC-file to package RPMs for Fedora

# download and build like this
# (you need an rpmbuild dir, can be created with rpmdev-setuptree)
#
# cp smuview.spec $HOME/rpmbuild/SPECS
# cd $HOME/rpmbuild
# spectool -g -R SPECS/smuview.spec
# rpmbuild -ba SPECS/smuview.spec

Summary: SmuView is a Qt based source measure unit GUI for sigrok.
Name: smuview
Version: 0.0.1
Release: 1%{?dist}
License: GPLv3
URL: https://github.com/knarfS/smuview
Source: https://github.com/knarfS/%{name}/archive/%{name}-%{version}.tar.gz
BuildRequires: libsigrok-cxx-devel
BuildRequires: qwt-qt5-devel
BuildRequires: qt5-devel
BuildRequires: qt5-qtsvg-devel
BuildRequires: qt5-qtbase-devel
BuildRequires: glibmm24-devel
BuildRequires: boost-devel
BuildRequires: rubygem-asciidoctor

%description
The sigrok project aims at creating a portable, cross-platform,
Free/Libre/Open-Source signal analysis software suite that supports various
device types (such as logic analyzers, oscilloscopes, multimeters, and more).

SmuView is a Qt-based source measurement unit GUI for sigrok.

%prep
%autosetup -n %{name}-%{name}-%{version}

%build
%cmake .
%make_build
make manual-html

%install
%make_install

%files
%{!?_licensedir:%global license %%doc}
%license COPYING
%doc README INSTALL stuff
%{_docdir}/%{name}/images/sv_with_load.png
%{_docdir}/%{name}/manual.html
%{_bindir}/smuview
%{_mandir}/man1/%{name}.*
%{_datadir}/applications/org.sigrok.SmuView.desktop
%{_datadir}/metainfo/org.sigrok.SmuView.appdata.xml
%{_datadir}/icons/hicolor/48x48/apps/smuview.png
%{_datadir}/icons/hicolor/scalable/apps/smuview.svg
