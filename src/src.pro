TEMPLATE = app
TARGET = btrfs-balancer
target.path = /usr/sbin

QT = core dbus
CONFIG += link_pkgconfig

INSTALLS += target

PKGCONFIG += keepalive

SOURCES += \
    main.cpp \
    dbusconnector.cpp \
    btrfsbalancer.cpp \
    btrfs.cpp \
    maintenance.cpp

HEADERS += \
    dbusconnector.h \
    btrfsbalancer.h \
    btrfs.h \
    maintenance.h
