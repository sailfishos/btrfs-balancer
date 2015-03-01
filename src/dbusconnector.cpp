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


#include "dbusconnector.h"
#include "maintenance.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QString>
#include <QDebug>

namespace
{
const QString DBUS_SERVICE("org.nemomobile.BtrfsBalancer");
const QString DBUS_PATH("/BtrfsBalancer");
const QString DBUS_INTERFACE("org.nemomobile.BtrfsBalancer");

// timeout in ms after which we service stops after being idle
const int IDLE_TIMEOUT = 30000;
}


Service::Service(QObject *parent)
    : QDBusAbstractAdaptor(parent)
    , m_balancer(new BtrfsBalancer)
{
    connect(m_balancer.data(), SIGNAL(status(BtrfsBalancer::Status)),
            this, SLOT(slotStatusReceived(BtrfsBalancer::Status)));
    connect(m_balancer.data(), SIGNAL(allocation(qlonglong,qlonglong)),
            this, SIGNAL(allocation(qlonglong,qlonglong)));
    connect(m_balancer.data(), SIGNAL(progress(int)),
            this, SIGNAL(progress(int)));
    connect(m_balancer.data(), SIGNAL(finished(bool)),
            this, SIGNAL(finished(bool)));

    connect(&m_idleTimer, SIGNAL(timeout()),
            this, SLOT(slotIdleTimerTriggered()));

    m_idleTimer.start(IDLE_TIMEOUT);
}

void Service::checkStatus()
{
    m_balancer->checkStatus();
}

void Service::checkAllocation()
{
    m_balancer->checkAllocation();
}

void Service::balance()
{
    m_balancer->balance();
}

void Service::maintenance(int allocationThreshold,
                          int batteryThreshold)
{
    Maintenance *maintenance = new Maintenance(m_balancer,
                                               allocationThreshold,
                                               batteryThreshold);
    connect(maintenance, SIGNAL(finished()),
            this, SLOT(slotMaintenanceFinished()));
    maintenance->start();
}

void Service::slotStatusReceived(BtrfsBalancer::Status s)
{
    emit status(static_cast<int>(s));

    if (s == BtrfsBalancer::BALANCING) {
        m_idleTimer.stop();
    } else {
        m_idleTimer.start(IDLE_TIMEOUT);
    }
}

void Service::slotIdleTimerTriggered()
{
    qDebug() << "Shutting down service" << DBUS_SERVICE;
    qApp->quit();
}

void Service::slotMaintenanceFinished()
{
    sender()->deleteLater();
}


DBusConnector::DBusConnector(QObject *parent)
    : QObject(parent)
    , QDBusContext()
    , m_service(0)
{
    qDebug() << Q_FUNC_INFO;
    acquireService();
}

DBusConnector::~DBusConnector()
{
    qDebug() << Q_FUNC_INFO;
    if (m_service) {
        QDBusConnection::systemBus().unregisterService(DBUS_SERVICE);
    }
}

void DBusConnector::acquireService()
{
    qDebug() << "Acquiring D-Bus service";
    QDBusConnection bus = QDBusConnection::systemBus();

    if (!bus.isConnected()) {
        qWarning() << "Could not connect to system bus.";
        return;
    }

    if (bus.registerService(DBUS_SERVICE)) {
        m_service = new Service(this);

        if (!bus.registerObject(DBUS_PATH, this)) {
            qWarning() << "Failed to register service object:" << DBUS_PATH;
        }
    } else {
        qWarning() << "Service name is already in use or not authorized:" << DBUS_SERVICE;
    }
}
