/****************************************************************************************
**
** Copyright (C) 2015 Jolla Ltd.
** Contact: Martin Grimme <martin.grimme@gmail.com>
** All rights reserved.
**
** This file is part of the btrfs-balancer package.
**
** You may use this file under the terms of the GNU Lesser General
** Public License version 2.1 as published by the Free Software Foundation
** and appearing in the file license.lgpl included in the packaging
** of this file.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file license.lgpl included in the packaging
** of this file.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** Lesser General Public License for more details.
**
****************************************************************************************/


#include "allocationcommand.h"
#include "btrfsbalancer.h"
#include "dbusservice.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>

#include <iostream>

AllocationCommand::AllocationCommand(QObject *parent)
    : Command(parent)
{
    QDBusConnection bus = QDBusConnection::systemBus();

    bus.connect(DBUS_SERVICE,
                DBUS_PATH,
                DBUS_INTERFACE,
                "status",
                this,
                SLOT(slotGotStatus(int)));

    bus.connect(DBUS_SERVICE,
                DBUS_PATH,
                DBUS_INTERFACE,
                "allocation",
                this,
                SLOT(slotGotAllocation(qlonglong,qlonglong)));

}

void AllocationCommand::start()
{
    callService("checkStatus");
}

void AllocationCommand::callService(const QString &methodName)
{
    QDBusMessage methodCall = QDBusMessage::createMethodCall(DBUS_SERVICE,
                                                             DBUS_PATH,
                                                             DBUS_INTERFACE,
                                                             methodName);
    QDBusConnection::systemBus().callWithCallback(methodCall,
                                                  this,
                                                  SLOT(slotDBusCallSuccess()),
                                                  SLOT(slotDBusCallError()),
                                                  1000);
}

void AllocationCommand::slotDBusCallError()
{
    std::cerr << "D-Bus Error: "
              << QDBusConnection::systemBus()
                 .lastError()
                 .message()
                 .toUtf8()
                 .constData()
              << std::endl;
    failure(Command::DBUS_ERROR);
}

void AllocationCommand::slotGotStatus(int statusCode)
{
    BtrfsBalancer::Status status =
            static_cast<BtrfsBalancer::Status>(statusCode);

    if (status == BtrfsBalancer::READY) {
        callService("checkAllocation");
    } else {
        std::cout << "Failed to get allocation. Balancing in progress."
                  << std::endl;
        failure(Command::BUSY_ERROR);
    }
}

void AllocationCommand::slotGotAllocation(qlonglong size, qlonglong used)
{
    if (size > 0) {
        std::cout << "Total:     " << size << " bytes" << std::endl;
        std::cout << "Used:      " << used << " bytes" << std::endl;
        std::cout << "Allocated: " << static_cast<int>(used * 100 / size)
                  << " %" << std::endl;
        success();
    } else {
        std::cerr << "Failed to get allocation." << std::endl;
        failure(Command::RESULT_ERROR);
    }
}
