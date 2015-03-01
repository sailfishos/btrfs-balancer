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


#ifndef MAINTENANCE_H
#define MAINTENANCE_H

#include "btrfsbalancer.h"

#include <QObject>
#include <QSharedPointer>

class Maintenance : public QObject
{
    Q_OBJECT
public:
    Maintenance(QSharedPointer<BtrfsBalancer> balancer,
                int allocationThreshold,
                int batteryThreshold,
                QObject *parent = 0);

    virtual ~Maintenance();

    void start();

signals:
    void finished();

private slots:
    void slotStatusReceived(BtrfsBalancer::Status status);
    void slotAllocationReceived(qlonglong size, qlonglong used);

private:
    QSharedPointer<BtrfsBalancer> m_balancer;
    bool m_allocationRequired;
    int m_allocationThreshold;
    int m_batteryThreshold;
};

#endif // MAINTENANCE_H
