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


#ifndef OPTIONS_H
#define OPTIONS_H

#include <QStringList>

/* Class for parsing the command line arguments.
 */
class Options
{
public:
    enum Command
    {
        NO_COMMAND,
        SERVER,
        BALANCE,
        ALLOCATION,
    };

    Options(const QStringList &arguments);
    Command command() const { return m_command; }
    int batteryThreshold() const { return m_batteryThreshold; }
    int allocationThreshold() const { return m_allocationThreshold; }

private:
    Command m_command;
    int m_batteryThreshold;
    int m_allocationThreshold;
};

#endif // OPTIONS_H
