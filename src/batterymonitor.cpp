/****************************************************************************************
**
** Copyright (c) 2015 - 2020 Jolla Ltd.
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

#include "batterymonitor.h"

BatteryMonitor::BatteryMonitor(QObject *parent)
    : QObject(parent)
    , m_batteryStatusTracker(new BatteryStatus(this))
    , m_chargerStatus(BatteryStatus::ChargerStatusUnknown)
    , m_batteryStatus(BatteryStatus::BatteryStatusUnknown)
    , m_currentCharge(-1) // charge is -1 while pending and to be ignored
    , m_currentStatus(PENDING)
{
    connect(m_batteryStatusTracker, &BatteryStatus::chargerStatusChanged,
            this, &BatteryMonitor::onChargerStatusChanged);
    connect(m_batteryStatusTracker, &BatteryStatus::statusChanged,
            this, &BatteryMonitor::onBatteryStatusChanged);
    connect(m_batteryStatusTracker, &BatteryStatus::chargePercentageChanged,
            this, &BatteryMonitor::onChargePercentageChanged);
}

void BatteryMonitor::onChargerStatusChanged(BatteryStatus::ChargerStatus chargerStatus)
{
    if (m_chargerStatus != chargerStatus) {
        m_chargerStatus = chargerStatus;
        updateStatus();
    }
}

void BatteryMonitor::onBatteryStatusChanged(BatteryStatus::Status batteryStatus)
{
    if (m_batteryStatus != batteryStatus) {
        m_batteryStatus = batteryStatus;
        updateStatus();
    }
}

void BatteryMonitor::onChargePercentageChanged(int chargePercentage)
{
    if (m_currentCharge != chargePercentage) {
        m_currentCharge = chargePercentage;
        updateStatus();
    }
}

void BatteryMonitor::updateStatus()
{
    ChargerStatus updatedStatus = m_currentStatus;

    switch (m_batteryStatus) {
    case BatteryStatus::Low:
    case BatteryStatus::Empty:
        updatedStatus = CRITICAL;
        break;
    default:
        switch (m_chargerStatus) {
        case BatteryStatus::Connected:
            updatedStatus = CHARGING;
            break;
        case BatteryStatus::Disconnected:
            updatedStatus = DISCHARGING;
            break;
        default:
            if (m_currentStatus != PENDING)
                updatedStatus = UNKNOWN;
            break;
        }
    }

    if (updatedStatus != PENDING) {
        m_currentStatus = updatedStatus;
        emit status(m_currentStatus, m_currentCharge);
    }
}
