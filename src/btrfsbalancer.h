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


#ifndef BTRFSBALANCER_H
#define BTRFSBALANCER_H

#include "btrfs.h"

#include <QObject>
#include <QString>

class BtrfsBalancer : public QObject
{
    Q_OBJECT
public:
    enum Status
    {
        READY = 0,
        BALANCING,
    };

    explicit BtrfsBalancer(QObject *parent = 0);

    /* Checks the status of the balancer.
     * Emits the status signal.
     */
    void checkStatus();

    /* Checks the filesystem allocation.
     * Emits the allocation signal.
     */
    void checkAllocation();

    /* Starts the balancing operation.
     * Emits finished signal after success or failure.
     * Emits progress signals inbetween.
     */
    void startBalance();

signals:
    void pendingChanged(bool pending);
    void status(BtrfsBalancer::Status status);
    void allocation(qlonglong size, qlonglong used);
    void progress(int percentage);
    void finished(bool success);

private:
    void setStatus(Status newStatus);
    void process();

private slots:
    void slotReceivedAllocation(qint64 size, qint64 used);
    void slotBalanceProgress(int percents);
    void slotBalanceFinished(bool success);

private:
    Status m_currentStatus;
    QList<int> m_usageLevels;
};

#endif // BTRFSBALANCER_H
