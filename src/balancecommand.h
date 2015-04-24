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


#ifndef BALANCECOMMAND_H
#define BALANCECOMMAND_H

#include "command.h"
#include "batterymonitor.h"

#include <QList>
#include <QVariant>

class BalanceCommand : public Command
{
    Q_OBJECT
public:
    BalanceCommand(int batteryThreshold,
                   int allocationThreshold,
                   QObject *parent = 0);
    virtual ~BalanceCommand();

public slots:
    virtual void start();

private:
    void callService(const QString &methodName,
                     const QList<QVariant> &arguments = QList<QVariant>());

private slots:
    void slotDBusCallSuccess() { /* not of interest */ }
    void slotDBusCallError();
    void slotGotStatus(int statusCode);
    void slotGotAllocation(qlonglong size, qlonglong used);
    void slotGotProgress(int percents);
    void slotFinished(bool successful);

    void slotBatteryStatusChanged(BatteryMonitor::ChargerStatus chargerStatus,
                                  int level);

private:
    int m_allocationThreshold;
    int m_batteryThreshold;
    bool m_isWaitingForBatteryCheck;
    bool m_isBalancing;
    BatteryMonitor *m_batteryMonitor;

};

#endif // BALANCECOMMAND_H
