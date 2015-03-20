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
#include "dbusservice.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QFileInfo>
#include <QString>
#include <QDebug>

namespace
{
// timeout in ms after which we service stops after being idle
const int IDLE_TIMEOUT = 30000;
}


Service::Service(QDBusContext *context, QObject *parent)
    : QDBusAbstractAdaptor(parent)
    , m_context(context)
{
    m_balancer = new BtrfsBalancer(this);
    connect(m_balancer, SIGNAL(status(BtrfsBalancer::Status)),
            this, SLOT(slotStatusReceived(BtrfsBalancer::Status)));
    connect(m_balancer, SIGNAL(lastBalanced(qlonglong)),
            this, SIGNAL(lastBalanced(qlonglong)));
    connect(m_balancer, SIGNAL(allocation(qlonglong,qlonglong)),
            this, SIGNAL(allocation(qlonglong,qlonglong)));
    connect(m_balancer, SIGNAL(progress(int)),
            this, SIGNAL(progress(int)));
    connect(m_balancer, SIGNAL(finished(bool)),
            this, SIGNAL(finished(bool)));

    connect(m_balancer, SIGNAL(pendingChanged(bool)),
            this, SLOT(slotPendingChanged(bool)));

    connect(&m_idleTimer, SIGNAL(timeout()),
            this, SLOT(slotIdleTimerTriggered()));

    m_idleTimer.start(IDLE_TIMEOUT);
}

void Service::checkStatus()
{
    if (!isPrivileged()) return;
    m_balancer->checkStatus();
}

void Service::checkLastBalanced()
{
    if (!isPrivileged()) return;
    m_balancer->checkLastBalanced();
}

void Service::checkAllocation()
{
    qDebug() << Q_FUNC_INFO;
    if (!isPrivileged()) return;
    m_balancer->checkAllocation();
}

void Service::startBalance()
{
    if (!isPrivileged()) return;
    m_balancer->startBalance();
}

bool Service::isPrivileged()
{
    // courtesy of nemomobile/password-manager

    // this object has m_context as parent, so m_context will always be valid
    // during its lifetime

    if (!m_context->calledFromDBus()) {
        // Local function calls are always privileged
        return true;
    }

    // Get the PID of the calling process
    pid_t pid = m_context->connection().interface()->servicePid(
                m_context->message().service());

    // The /proc/<pid> directory is owned by EUID:EGID of the process
    QFileInfo info(QString("/proc/%1").arg(pid));
    if (info.group() != "privileged" && info.owner() != "root") {
        m_context->sendErrorReply(
                    QDBusError::AccessDenied,
                    QString("PID %1 is not in privileged group").arg(pid));
        return false;
    }
    return true;
}

void Service::slotPendingChanged(bool pending)
{
    if (pending) {
        m_idleTimer.stop();
    } else {
        m_idleTimer.start(IDLE_TIMEOUT);
    }
}

void Service::slotStatusReceived(BtrfsBalancer::Status s)
{
    emit status(static_cast<int>(s));
}

void Service::slotIdleTimerTriggered()
{
    qDebug() << "Shutting down service" << DBUS_SERVICE;
    qApp->quit();
}


DBusConnector::DBusConnector(QObject *parent)
    : QObject(parent)
    , QDBusContext()
    , m_service(0)
    , m_isConnected(false)
{
    acquireService();
}

DBusConnector::~DBusConnector()
{
    if (m_service) {
        QDBusConnection::systemBus().unregisterService(DBUS_SERVICE);
    }
}

void DBusConnector::acquireService()
{
    qDebug() << "Acquiring D-Bus service";
    QDBusConnection bus = QDBusConnection::systemBus();

    if (!bus.isConnected()) {
        qCritical() << "Could not connect to system bus.";
        return;
    }

    if (bus.registerService(DBUS_SERVICE)) {
        m_service = new Service(this, this);

        if (!bus.registerObject(DBUS_PATH, this)) {
            qCritical() << "Failed to register service object:" << DBUS_PATH;
        } else {
            m_isConnected = true;
        }
    } else {
        qCritical() << "Service name is already in use or not authorized:" << DBUS_SERVICE;
    }
}
