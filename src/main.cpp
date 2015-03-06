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


#include "dbusconnector.h"

#include <QCoreApplication>
#include <QDebug>
#include <backgroundactivity.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    DBusConnector dbusConnector;

    // keep CPU alive while this service is running
    BackgroundActivity activity;
    activity.setState(BackgroundActivity::Running);

    if (dbusConnector.isConnected()) {
        return app.exec();
    } else {
        qCritical("Connection to D-Bus and/or service registration failed.");
        return 1;
    }
}
