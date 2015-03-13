TEMPLATE = app
TARGET = btrfs-balancer
target.path = /usr/sbin

QT = core dbus
CONFIG += console link_pkgconfig

LIBS += -lssu

INSTALLS += target

PKGCONFIG += keepalive

SOURCES += \
    main.cpp \
    dbusconnector.cpp \
    btrfsbalancer.cpp \
    btrfs.cpp \
    options.cpp \
    balancecommand.cpp \
    allocationcommand.cpp

HEADERS += \
    dbusconnector.h \
    btrfsbalancer.h \
    btrfs.h \
    options.h \
    dbusservice.h \
    balancecommand.h \
    allocationcommand.h \
    command.h
