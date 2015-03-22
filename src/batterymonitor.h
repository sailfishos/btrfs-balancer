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


#ifndef BATTERYMONITOR_H
#define BATTERYMONITOR_H

#include <QObject>

class BatteryMonitor : public QObject
{
    Q_OBJECT
public:
    enum ChargerStatus
    {
        PENDING, // ignore any charge changes in pending state
        UNKNOWN,
        CHARGING,
        DISCHARGING,
        CRITICAL,
    };

    explicit BatteryMonitor(QObject *parent = 0);

signals:
    void status(BatteryMonitor::ChargerStatus chargerStatus, int level);

private slots:
    void slotChargeChanged();
    void slotStateChanged();

private:
    int m_currentCharge;
    ChargerStatus m_currentStatus;
};

#endif // BATTERYMONITOR_H
