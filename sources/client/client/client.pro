TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += ../client-entities
SUBDIRS += ../client-modules
SUBDIRS += ../client-launcher
SUBDIRS += ../client-app

include (../common/common.pri)

unix {

    INSTALLS += binaries \
	pixmaps \
	desktop

    pixmaps.path = $$DATADIR/pixmaps
    pixmaps.files += ../client-app/images/client.png

    desktop.path = $$DATADIR/applications
    desktop.files += ../client-app/gnome.desktop

}


OTHER_FILES += debian/control
