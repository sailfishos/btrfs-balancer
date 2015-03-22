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


#include "batterymonitor.h"

#include <contextproperty.h>

namespace
{
const QString BATTERY_CHARGE("Battery.ChargePercentage");
const QString BATTERY_STATE("Battery.State");
}

BatteryMonitor::BatteryMonitor(QObject *parent)
    : QObject(parent)
    , m_currentCharge(-1) // charge is -1 while pending and to be ignored
    , m_currentStatus(PENDING)
{
    ContextProperty *chargeProperty = new ContextProperty(BATTERY_CHARGE, this);
    ContextProperty *stateProperty = new ContextProperty(BATTERY_STATE, this);

    connect(chargeProperty, SIGNAL(valueChanged()),
            this, SLOT(slotChargeChanged()));
    connect(stateProperty, SIGNAL(valueChanged()),
            this, SLOT(slotStateChanged()));
}

void BatteryMonitor::slotChargeChanged()
{
    ContextProperty* prop = qobject_cast<ContextProperty*>(sender());
    if (prop) {
        m_currentCharge = prop->value(QVariant(0)).toInt();

        if (m_currentStatus != PENDING) {
            emit status(m_currentStatus, m_currentCharge);
        }
    }
}

void BatteryMonitor::slotStateChanged()
{
    ContextProperty* prop = qobject_cast<ContextProperty*>(sender());
    if (prop) {
        const QString value = prop->value(QVariant("unknown")).toString();

        if (value == "charging") {
            m_currentStatus = CHARGING;
        } else if (value == "discharging") {
            m_currentStatus = DISCHARGING;
        } else if (value == "full") {
            m_currentStatus = DISCHARGING; // no difference for this use case
        } else if (value == "low" || value == "empty") {
            m_currentStatus = CRITICAL;
        } else {  // unknown
            m_currentStatus = UNKNOWN;
        }

        if (m_currentCharge != -1) {
            emit status(m_currentStatus, m_currentCharge);
        }
    }
}
