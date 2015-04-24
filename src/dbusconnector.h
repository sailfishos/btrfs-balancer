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


#ifndef DBUSCONNECTOR_H
#define DBUSCONNECTOR_H

#include "btrfsbalancer.h"

#include <QDBusAbstractAdaptor>
#include <QDBusContext>
#include <QTimer>

class Service : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.nemomobile.BtrfsBalancer")

public:
    Service(QDBusContext *context, QObject *parent = 0);

public slots:
    Q_NOREPLY void checkStatus();
    Q_NOREPLY void checkLastBalanced();
    Q_NOREPLY void checkAllocation();
    Q_NOREPLY void startBalance(int allocationGoal = 0);
    Q_NOREPLY void cancel();

signals:
    void status(int status);
    void lastBalanced(qlonglong msecsSinceEpoch);
    void allocation(qlonglong size, qlonglong used);
    void progress(int);
    void finished(bool success);

private:
    bool isPrivileged();

private slots:
    void slotPendingChanged(bool pending);
    void slotStatusReceived(BtrfsBalancer::Status s);
    void slotIdleTimerTriggered();

private:
    QDBusContext *m_context;
    BtrfsBalancer *m_balancer;
    QTimer m_idleTimer;
};


class DBusConnector : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.nemomobile.BtrfsBalancer")

public:
    DBusConnector(QObject *parent = 0);
    virtual ~DBusConnector();

    bool isConnected() const { return m_isConnected; }

private:
    void acquireService();

private:
    Service* m_service;
    bool m_isConnected;
};

#endif // DBUSCONNECTOR_H
