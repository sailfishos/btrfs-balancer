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


#include "btrfsbalancer.h"

#include <qmath.h>
#include <QDebug>

namespace
{

// the max usages in percents for which to run the balancing operation, in the
// order of their application
const QList<int> MAX_USAGE_PERCENTS =
        QList<int>() << 0 << 10 << 20 << 35 << 50 << 75 << 96;

}

BtrfsBalancer::BtrfsBalancer(QObject *parent)
    : QObject(parent)
    , m_currentStatus(READY)
{

}

void BtrfsBalancer::setStatus(Status newStatus)
{
    if (newStatus != m_currentStatus) {
        m_currentStatus = newStatus;
        emit status(newStatus);
    }
}

void BtrfsBalancer::checkStatus()
{
    emit pendingChanged(true);
    emit status(m_currentStatus);
    emit pendingChanged(false);
}

void BtrfsBalancer::checkAllocation()
{
    emit pendingChanged(true);
    if (m_currentStatus == READY) {
        Btrfs* btrfs = new Btrfs;
        connect(btrfs, SIGNAL(allocationReceived(qint64,qint64)),
                this, SLOT(slotReceivedAllocation(qint64,qint64)));
        btrfs->requestAllocation();
    } else {
        emit allocation(-1, -1);
        emit pendingChanged(false);
    }
}

void BtrfsBalancer::startBalance()
{
    emit pendingChanged(true);
    m_usageLevels = MAX_USAGE_PERCENTS;
    setStatus(BALANCING);
    process();
}

void BtrfsBalancer::process()
{
    if (m_usageLevels.size()) {
        int usage = m_usageLevels.first();

        qDebug() << "Balancing..." << usage << "%";
        Btrfs* btrfs = new Btrfs();
        connect(btrfs, SIGNAL(balanceProgress(int)),
                this, SLOT(slotBalanceProgress(int)));
        connect(btrfs, SIGNAL(balanceFinished(bool)),
                this, SLOT(slotBalanceFinished(bool)));
        btrfs->startBalance(usage);
        emit progress(usage);
    } else {
        qDebug() << "Balancing finished";
        emit progress(100);
        emit finished(true);
        setStatus(READY);
        emit pendingChanged(false);
    }
}

void BtrfsBalancer::slotReceivedAllocation(qint64 size, qint64 used)
{
    sender()->deleteLater();

    if (size > 0) {
        qDebug() << used << "of" << size << "bytes used";
        emit allocation(size, used);
    } else {
        qDebug() << "Failed to determine filesystem allocation";
        emit allocation(-1, -1);
    }
    emit pendingChanged(false);
}

void BtrfsBalancer::slotBalanceProgress(int percents)
{
    // this progress is a subprogress between the current usage level and the
    // next, or 100, if there is no next

    int currentUsage = m_usageLevels.size() > 0 ? m_usageLevels.first()
                                                : 100;
    int nextUsage = m_usageLevels.size() > 1 ? m_usageLevels.at(1)
                                             : 100;

    double subProgress = percents * (nextUsage - currentUsage) / 100.0;
    emit progress(qFloor(currentUsage + subProgress));
}

void BtrfsBalancer::slotBalanceFinished(bool success)
{
    sender()->deleteLater();

    if (success) {
        m_usageLevels.removeFirst();
        process();
    } else {
        qWarning() << "Balancing failed. Please free up more space and try again";
        emit finished(false);
        setStatus(READY);
        emit pendingChanged(false);
    }
}
