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
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

namespace
{

// the max usages in percents for which to run the balancing operation, in the
// order of their application
const QList<int> MAX_USAGE_PERCENTS =
        QList<int>() << 0 << 10 << 20 << 35 << 50 << 75 << 96;


// path of the file for remembering the timestamp of the last successful
// balancing operation
const QString CACHE_DIR("/var/cache");
const QString BALANCER_DIR("btrfs-balancer");
const QString TIMESTAMP_FILE("last-success");

void updateTimestamp()
{
    QDir cacheDir(CACHE_DIR);
    if (!cacheDir.exists(BALANCER_DIR) && !cacheDir.mkdir(BALANCER_DIR)) {
        qWarning() << "Could not create directory:"
                   << cacheDir.filePath(BALANCER_DIR);
        return;
    }

    if (!cacheDir.cd(BALANCER_DIR)
            || !QFile(cacheDir.filePath(TIMESTAMP_FILE)).open(QFile::WriteOnly)) {
        qWarning() << "Could not update timestamp file:" << TIMESTAMP_FILE;
    }
}

}

BtrfsBalancer::BtrfsBalancer(QObject *parent)
    : QObject(parent)
    , m_currentStatus(READY)
    , m_currentBtrfs(0)
    , m_allocationGoal(0)
{

}

BtrfsBalancer::~BtrfsBalancer()
{
    if (m_currentBtrfs) {
        delete m_currentBtrfs;
    }
}

void BtrfsBalancer::setStatus(Status newStatus)
{
    if (newStatus != m_currentStatus) {
        m_currentStatus = newStatus;
        emit status(newStatus);
    }

    emit pendingChanged(newStatus != READY);
}

void BtrfsBalancer::checkStatus()
{
    emit status(m_currentStatus);
}

void BtrfsBalancer::checkLastBalanced()
{
    QDir cacheDir(CACHE_DIR);
    if (cacheDir.cd(BALANCER_DIR)) {
        QFileInfo timeStampFile(cacheDir.absoluteFilePath(TIMESTAMP_FILE));
        if (timeStampFile.exists()) {
            emit lastBalanced(timeStampFile.lastModified().toMSecsSinceEpoch());
        } else {
            emit lastBalanced(0);
        }
    } else {
        emit lastBalanced(0);
    }
}

void BtrfsBalancer::checkAllocation()
{
    qDebug() << Q_FUNC_INFO;
    emit pendingChanged(true);
    if (m_currentStatus == READY) {
        m_currentBtrfs = new Btrfs;
        connect(m_currentBtrfs, SIGNAL(allocationReceived(qint64,qint64)),
                this, SLOT(slotReceivedAllocation(qint64,qint64)));
        m_currentBtrfs->requestAllocation();
    } else {
        emit allocation(-1, -1);
    }
}

void BtrfsBalancer::startBalance(int allocationGoal)
{
    emit pendingChanged(true);
    m_usageLevels = MAX_USAGE_PERCENTS;
    m_allocationGoal = allocationGoal;
    setStatus(BALANCING);
    process();
}

void BtrfsBalancer::process()
{
    if (m_usageLevels.size()) {
        int usage = m_usageLevels.first();

        qDebug() << "Balancing..." << usage << "%";
        m_currentBtrfs = new Btrfs();
        connect(m_currentBtrfs, SIGNAL(balanceProgress(int)),
                this, SLOT(slotBalanceProgress(int)));
        connect(m_currentBtrfs, SIGNAL(balanceFinished(bool,qint64,qint64)),
                this, SLOT(slotBalanceFinished(bool,qint64,qint64)));
        m_currentBtrfs->startBalance(usage);
        emit progress(usage);
    } else {
        qDebug() << "Balancing finished";
        emit progress(100);
        emit finished(true);
        updateTimestamp();
        setStatus(READY);
        emit pendingChanged(false);
    }
}

void BtrfsBalancer::slotReceivedAllocation(qint64 size, qint64 used)
{
    sender()->deleteLater();
    m_currentBtrfs = 0;

    if (size > 0) {
        qDebug() << used << "of" << size << "bytes used";
        emit allocation(size, used);
    } else {
        qDebug() << "Failed to determine filesystem allocation";
        emit allocation(-1, -1);
    }
    emit pendingChanged(m_currentStatus != READY);
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

void BtrfsBalancer::slotBalanceFinished(bool success, qint64 size, qint64 used)
{
    sender()->deleteLater();
    m_currentBtrfs = 0;

    if (success) {
        int percentage = static_cast<int>(used * 100 / size);
        if (percentage <= m_allocationGoal) {
            // goal reached; all done
            m_usageLevels.clear();
        } else {
            m_usageLevels.removeFirst();
        }
        process();
    } else {
        qWarning() << "Balancing failed.";
        emit finished(false);
        setStatus(READY);
        emit pendingChanged(false);
    }
}
