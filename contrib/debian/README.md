
Debian
====================
This directory contains files used to package zixxd/zixx-qt
for Debian-based Linux systems. If you compile zixxd/zixx-qt yourself, there are some useful files here.

## zixx: URI support ##


zixx-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install zixx-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your zixx-qt binary to `/usr/bin`
and the `../../share/pixmaps/zixx128.png` to `/usr/share/pixmaps`

zixx-qt.protocol (KDE)

