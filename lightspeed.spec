%define ver 1.2a
%define rel 1
%define prefix /usr/X11R6

Summary: Light Speed! - an interactive relativistic simulator
Name: lightspeed
Version: %ver
Release: %rel
Copyright: MozPL/GPL
Group: X11/Amusements
URL: http://lightspeed.sourceforge.net/
Requires: gtk+ >= 1.0.1
BuildRoot: /tmp/lightspeed-%ver-root

%description
Light Speed! is an OpenGL-based program which illustrates the effects of
special relativity on the appearance of moving objects. When an object
accelerates past a few million meters per second, these effects begin to
grow noticeable, becoming more and more pronounced as the speed of light
is approached. These relativistic effects are viewpoint-dependent, and
include shifts in length, object hue, brightness and shape.

The moving object is, by default, a geometric lattice. 3D Studio and
LightWave 3D objects may be imported as well. Best of all, the simulator
is completely interactive, rendering the distortions in real-time!

%prep

%setup

%build
./configure --prefix=%{prefix}
make CFLAGS="$RPM_OPT_FLAGS"

%install
rm -rf $RPM_BUILD_ROOT
make prefix=$RPM_BUILD_ROOT%{prefix} install-strip

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc AUTHORS CONTROLS COPYING ChangeLog MATH OVERVIEW README TODO
%{prefix}/bin/lightspeed
