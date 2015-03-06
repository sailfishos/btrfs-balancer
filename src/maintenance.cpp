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


#include "maintenance.h"

#include <QFile>
#include <QString>
#include <QDebug>

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


Maintenance::Maintenance(QSharedPointer<BtrfsBalancer> balancer,
                         int allocationThreshold,
                         int batteryThreshold,
                         QObject *parent)
    : QObject(parent)
    , m_balancer(balancer)
    , m_allocationRequired(true)
    , m_allocationThreshold(allocationThreshold)
    , m_batteryThreshold(batteryThreshold)
{
    connect(m_balancer.data(), SIGNAL(status(BtrfsBalancer::Status)),
            this, SLOT(slotStatusReceived(BtrfsBalancer::Status)));
    connect(m_balancer.data(), SIGNAL(allocation(qlonglong,qlonglong)),
            this, SLOT(slotAllocationReceived(qint64,qint64)));
    connect(m_balancer.data(), SIGNAL(finished(bool)),
            this, SIGNAL(finished()));
}

Maintenance::~Maintenance()
{
    qDebug() << "Maintenance btrfs balancing finished";
}

void Maintenance::start()
{
    qDebug() << "Starting maintenance btrfs balancing";
    if (! isBatteryCharging() && batteryCharge() < m_batteryThreshold) {
        qWarning() << "Cannot balance. Battery charge is too low."
                   << "Please plug in charger";
        emit finished();
    } else {
        m_balancer->checkStatus();
    }
}

void Maintenance::slotStatusReceived(BtrfsBalancer::Status status)
{
    if (status == BtrfsBalancer::READY) {
        if (m_allocationRequired) {
            m_balancer->checkAllocation();
        }
    } else {
        // balancing is already in progress; nothing to do
        qDebug() << "Balancing is already in progress";
        emit finished();
    }
    m_allocationRequired = false;
}

void Maintenance::slotAllocationReceived(qlonglong size, qlonglong used)
{
    if (size > 0) {
        int percentage = static_cast<int>(used * 100 / size);
        if (percentage >= m_allocationThreshold) {
            qDebug() << percentage << "% of space allocated, threshold is"
                     << m_allocationThreshold << "%, balancing needed";
            m_balancer->balance();
        } else {
            qDebug() << "No need for balance:" << percentage << "% allocated,"
                     << "balance limit is" << m_allocationThreshold << "%";
            emit finished();
        }
    }
}
