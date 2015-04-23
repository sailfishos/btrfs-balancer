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


#ifndef BTRFS_H
#define BTRFS_H

#include <QMap>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QTimer>

/* Asynchronous interface to some functionality of the btrfs system tool.
 */
class Btrfs : public QObject
{
    Q_OBJECT
public:
    explicit Btrfs(QObject *parent = 0);
    virtual ~Btrfs();

    void requestAllocation();
    void startBalance(int maxUsagePercent);


signals:
    void allocationReceived(qint64 size, qint64 used);
    void balanceProgress(int percents);
    void balanceFinished(bool success, qint64 size, qint64 used);

private:
    void loadDeviceConfiguration();
    int getBalanceProgress();

private slots:
    void slotBalanceProgress();
    void slotAllocationFinished(int exitCode, QProcess::ExitStatus status);
    void slotBalanceFinished(int exitCode, QProcess::ExitStatus status);

private:
    QProcess *m_currentProcess;
    QMap<QString, QString> m_deviceConfiguration;
    QTimer m_progressTimer;
    int m_currentProgress;
    bool m_isBalancing;
    bool m_isBalanced;
};

#endif // BTRFS_H
