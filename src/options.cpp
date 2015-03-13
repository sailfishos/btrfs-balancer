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


#include "options.h"

#include <QCommandLineOption>
#include <QCommandLineParser>

#include <iostream>

namespace
{
const int DEFAULT_BATTERY_THRESHOLD = 50;
const int DEFAULT_ALLOCATION_TRESHOLD = 75;
}

Options::Options(const QStringList &arguments)
    : m_command(NO_COMMAND)
    , m_batteryThreshold(DEFAULT_BATTERY_THRESHOLD)
    , m_allocationThreshold(DEFAULT_ALLOCATION_TRESHOLD)
{
    QCommandLineParser parser;
    parser.addPositionalArgument("command",
                                 "One of the commands: server, balance, allocation");
    QCommandLineOption batteryThreshold("b",
                                        "Required battery threshold for balancing (10 - 100).",
                                        "battery threshold",
                                        QString::number(DEFAULT_BATTERY_THRESHOLD));
    parser.addOption(batteryThreshold);
    QCommandLineOption allocationThreshold("a",
                                           "Required filesystem allocation threshold for balancing (0 - 100).",
                                           "allocation threshold",
                                           QString::number(DEFAULT_ALLOCATION_TRESHOLD));
    parser.addOption(allocationThreshold);
    parser.addHelpOption();

    parser.process(arguments);

    const QStringList positionalArgs = parser.positionalArguments();
    if (positionalArgs.size() > 0) {
        if (positionalArgs.at(0) == "server") {
            m_command = SERVER;
        } else if (positionalArgs.at(0) == "balance") {
            m_command = BALANCE;
        } else if (positionalArgs.at(0) == "allocation") {
            m_command = ALLOCATION;
        } else {
            parser.showHelp(1);
        }
    } else {
        parser.showHelp(1);
    }

    if (parser.isSet(batteryThreshold)) {
        bool ok = true;
        m_batteryThreshold = parser.value(batteryThreshold).toInt(&ok);
        if (!ok || m_batteryThreshold < 10 || m_batteryThreshold > 100) {
            std::cerr << "Battery threshold must be between 10 and 100."
                      << std::endl;
        }
    }

    if (parser.isSet(allocationThreshold)) {
        bool ok = true;
        m_allocationThreshold = parser.value(allocationThreshold).toInt(&ok);
        if (!ok || m_allocationThreshold < 0 || m_allocationThreshold > 100) {
            std::cerr << "Allocation threshold must be between 0 and 100."
                      << std::endl;
        }
    }
}
