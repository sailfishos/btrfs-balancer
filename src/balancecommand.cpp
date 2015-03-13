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


#include "balancecommand.h"
#include "btrfsbalancer.h"
#include "dbusservice.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QFile>

#include <iostream>

namespace
{
const QString BATTERY_CHARGE("/run/state/namespaces/Battery/ChargePercentage");
const QString BATTERY_STATUS("/run/state/namespaces/Battery/IsCharging");

int batteryCharge()
{
    QFile f(BATTERY_CHARGE);
    if (f.open(QFile::ReadOnly)) {
        return f.readAll().toInt();
    } else {
        return 0;
    }
}

bool isBatteryCharging()
{
    QFile f(BATTERY_STATUS);
    if (f.open(QFile::ReadOnly)) {
        return f.readAll().toInt() != 0;
    } else {
        return false;
    }
}

}

BalanceCommand::BalanceCommand(int batteryThreshold,
                               int allocationThreshold,
                               QObject *parent)
    : Command(parent)
    , m_allocationThreshold(allocationThreshold)
    , m_batteryThreshold(batteryThreshold)
    , m_isBalancing(false)
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

    bus.connect(DBUS_SERVICE,
                DBUS_PATH,
                DBUS_INTERFACE,
                "progress",
                this,
                SLOT(slotGotProgress(int)));

    bus.connect(DBUS_SERVICE,
                DBUS_PATH,
                DBUS_INTERFACE,
                "finished",
                this,
                SLOT(slotFinished()));
}

void BalanceCommand::start()
{
    if (!isBatteryCharging() && batteryCharge() < m_batteryThreshold) {
        std::cerr << "Cannot balance. Battery charge is too low. "
                  << "Please plug in charger and try again." << std::endl;
        failure(Command::BATTERY_ERROR);
    } else {
        callService("checkStatus");
    }
}

void BalanceCommand::callService(const QString &methodName)
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

void BalanceCommand::slotDBusCallError()
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

void BalanceCommand::slotGotStatus(int statusCode)
{
    if (!m_isBalancing) {
        BtrfsBalancer::Status status =
                static_cast<BtrfsBalancer::Status>(statusCode);

        if (status == BtrfsBalancer::READY) {
            callService("checkAllocation");
        } else {
            std::cout << "Balancing already in progress." << std::endl;
            failure(Command::BUSY_ERROR);
        }
    }
}

void BalanceCommand::slotGotAllocation(qlonglong size, qlonglong used)
{
    if (size > 0) {
        int percentage = static_cast<int>(used * 100 / size);
        if (percentage >= m_allocationThreshold) {
            m_isBalancing = true;
            callService("startBalance");
        } else {
            std::cout << "No balancing required. "
                      << percentage << " % allocated." << std::endl;
            success();
        }
    } else {
        std::cerr << "Failed to get allocation." << std::endl;
        failure(Command::RESULT_ERROR);
    }
}

void BalanceCommand::slotGotProgress(int percents)
{
    std::cout << "Balancing... " << percents << " %" << std::endl;
}

void BalanceCommand::slotFinished(bool successful)
{
    if (successful) {
        std::cout << "Finished." << std::endl;
        success();
    } else {
        std::cerr << "Failed. Please free up more space and try again." << std::endl;
        failure(Command::SPACE_ERROR);
    }
}
