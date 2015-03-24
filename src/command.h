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


#ifndef COMMAND_H
#define COMMAND_H

#include <QCoreApplication>
#include <QDebug>

/* Base class for user commands.
 */
class Command : public QObject
{
    Q_OBJECT
public:
    enum FailureCode
    {
        DBUS_ERROR = 1,
        BUSY_ERROR,
        RESULT_ERROR,
        BATTERY_ERROR,
        SPACE_ERROR,
        ABORTION_ERROR,
    };

    Command(QObject *parent = 0)
        : QObject(parent)
    { }

    void success()
    {
        qApp->quit();
    }

    void failure(FailureCode code)
    {
        qApp->exit(static_cast<int>(code));
    }

public slots:
    virtual void start() = 0;
};

#endif // COMMAND_H
