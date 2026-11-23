%global debug_package %{nil}
%global tarball_version 51.1

Name:           gnome-control-center
Version:        51.1
Release:        2.yubios%{?dist}
Summary:        yubiOS GNOME Settings with System Trust verification panel
License:        GPL-2.0-or-later AND CC0-1.0
URL:            https://github.com/yubi-OS/gnome-control-center
Source0:        %{name}-%{tarball_version}.tar.gz

BuildRequires:  blueprint-compiler >= 0.19
BuildRequires:  chrpath
BuildRequires:  desktop-file-utils
BuildRequires:  docbook-style-xsl
BuildRequires:  gcc
BuildRequires:  gettext
BuildRequires:  libappstream-glib
BuildRequires:  libxslt
BuildRequires:  meson >= 0.60.0
BuildRequires:  ninja-build
BuildRequires:  pkgconfig(accountsservice) >= 26.27.3
BuildRequires:  pkgconfig(colord) >= 0.1.34
BuildRequires:  pkgconfig(colord-gtk4)
BuildRequires:  pkgconfig(cups) >= 1.4
BuildRequires:  pkgconfig(epoxy)
BuildRequires:  pkgconfig(fontconfig)
BuildRequires:  pkgconfig(gcr-4) >= 4.1.0
BuildRequires:  pkgconfig(gdk-pixbuf-2.0) >= 2.23.0
BuildRequires:  pkgconfig(gio-2.0)
BuildRequires:  pkgconfig(glib-2.0) >= 2.76.6
BuildRequires:  pkgconfig(gnome-bg-4)
BuildRequires:  pkgconfig(gnome-bluetooth-ui-3.0)
BuildRequires:  pkgconfig(gnome-desktop-4) >= 51.alpha
BuildRequires:  pkgconfig(gnome-qr-gtk-4)
BuildRequires:  pkgconfig(gnome-settings-daemon) >= 48.rc
BuildRequires:  pkgconfig(goa-1.0) >= 3.51.0
BuildRequires:  pkgconfig(goa-backend-1.0)
BuildRequires:  pkgconfig(gsettings-desktop-schemas) >= 51.beta
BuildRequires:  pkgconfig(gsound)
BuildRequires:  pkgconfig(gtk4) >= 4.23.0
BuildRequires:  pkgconfig(gudev-1.0) >= 232
BuildRequires:  pkgconfig(ibus-1.0) >= 1.5.2
BuildRequires:  pkgconfig(json-glib-1.0)
BuildRequires:  pkgconfig(libadwaita-1) >= 1.8.alpha
BuildRequires:  pkgconfig(libgtop-2.0)
BuildRequires:  pkgconfig(libnm) >= 1.52.0
BuildRequires:  pkgconfig(libnma-gtk4) >= 1.10.2
BuildRequires:  pkgconfig(libpulse) >= 2.0
BuildRequires:  pkgconfig(libpulse-mainloop-glib) >= 2.0
BuildRequires:  pkgconfig(libsecret-1)
BuildRequires:  pkgconfig(libsoup-3.0)
BuildRequires:  pkgconfig(libwacom) >= 1.4
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(malcontent-0) >= 0.14.alpha
BuildRequires:  pkgconfig(mm-glib) >= 0.7
BuildRequires:  pkgconfig(polkit-gobject-1) >= 0.103
BuildRequires:  pkgconfig(pwquality) >= 1.2.2
BuildRequires:  pkgconfig(smbclient)
BuildRequires:  pkgconfig(tecla) >= 47.0
BuildRequires:  pkgconfig(udisks2)
BuildRequires:  pkgconfig(upower-glib) >= 1.90.6
BuildRequires:  pkgconfig(wayland-client)
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xi)

Requires:       %{name}-filesystem = %{version}-%{release}

%description
GNOME Settings (gnome-control-center) built for yubiOS with the native
System Trust panel (Firmware, Boot image, and Owner key verification).

%package filesystem
Summary:        GNOME Control Center directories
BuildArch:      noarch

%description filesystem
This package provides directories for GNOME Control Center panels and sounds.

%prep
%autosetup -n %{name}-%{tarball_version}

%build
%meson \
  -Ddocumentation=true \
  -Dlocation-services=enabled \
  -Dibus=true \
  -Dsnap=false \
  -Dmalcontent=true \
  -Dtests=false \
  -Ddistributor_logo=%{_datadir}/pixmaps/yubios-logo.png \
  -Ddark_mode_distributor_logo=%{_datadir}/pixmaps/yubios-logo.png
%meson_build

%install
%meson_install
mkdir -p %{buildroot}%{_datadir}/gnome-control-center/keybindings
mkdir -p %{buildroot}%{_datadir}/sounds/gnome/default/alerts
%find_lang %{name}-2.0 --with-gnome --all-name

%files -f %{name}-2.0.lang
%license COPYING
%{_bindir}/gnome-control-center
%{_libexecdir}/gnome-control-center-global-shortcuts-provider
%{_libexecdir}/gnome-control-center-print-renderer
%{_libexecdir}/gnome-control-center-search-provider
%{_datadir}/applications/*.desktop
%{_datadir}/bash-completion/completions/gnome-control-center
%{_datadir}/dbus-1/interfaces/org.gnome.GlobalShortcutsRebind.xml
%{_datadir}/dbus-1/services/org.gnome.Settings.service
%{_datadir}/dbus-1/services/org.gnome.Settings.SearchProvider.service
%{_datadir}/dbus-1/services/org.gnome.Settings.GlobalShortcutsProvider.service
%{_datadir}/gettext/its/gnome-keybindings.its
%{_datadir}/gettext/its/gnome-keybindings.loc
%{_datadir}/gettext/its/sounds.its
%{_datadir}/gettext/its/sounds.loc
%{_datadir}/glib-2.0/schemas/*.xml
%{_datadir}/gnome-control-center/keybindings/*
%{_datadir}/gnome-control-center/pixmaps
%{_datadir}/gnome-shell/search-providers/org.gnome.Settings.search-provider.ini
%{_datadir}/icons/gnome-logo-text*.svg
%{_datadir}/icons/hicolor/*/*/*
%{_datadir}/man/man1/gnome-control-center.1*
%{_datadir}/metainfo/org.gnome.Settings.metainfo.xml
%{_datadir}/pixmaps/faces
%{_datadir}/pkgconfig/gnome-keybindings.pc
%{_datadir}/polkit-1/actions/org.gnome.controlcenter.*.policy
%{_datadir}/polkit-1/rules.d/gnome-control-center.rules
%{_datadir}/sounds/gnome/default/alerts/*

%files filesystem
%dir %{_datadir}/gnome-control-center
%dir %{_datadir}/gnome-control-center/keybindings
%dir %{_datadir}/sounds/gnome
%dir %{_datadir}/sounds/gnome/default
%dir %{_datadir}/sounds/gnome/default/alerts

%changelog
* Thu Nov 19 2026 yubi-OS Contributors <security@yubios.org> - 51.1-2.yubios
- Add yubiOS System Trust panel (Firmware, Boot image, Owner key) and sidebar branding
