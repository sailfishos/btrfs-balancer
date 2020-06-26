TEMPLATE = app
TARGET = btrfs-balancer
target.path = /usr/sbin

QT = core dbus
CONFIG += console link_pkgconfig

INSTALLS += target

PKGCONFIG += systemsettings keepalive

SOURCES += \
    main.cpp \
    dbusconnector.cpp \
    btrfsbalancer.cpp \
    btrfs.cpp \
    options.cpp \
    balancecommand.cpp \
    allocationcommand.cpp \
    batterymonitor.cpp

HEADERS += \
    dbusconnector.h \
    btrfsbalancer.h \
    btrfs.h \
    options.h \
    dbusservice.h \
    balancecommand.h \
    allocationcommand.h \
    command.h \
    batterymonitor.h
