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


#include "allocationcommand.h"
#include "balancecommand.h"
#include "dbusconnector.h"
#include "options.h"

#include <QCoreApplication>
#include <QSharedPointer>
#include <QTimer>
#include <QDebug>
#include <backgroundactivity.h>

#include <signal.h>

void terminate(int unused)
{
    Q_UNUSED(unused);
    qApp->exit(Command::ABORTION_ERROR);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QSharedPointer<Command> command;

    // keep CPU alive while this service is running
    BackgroundActivity activity;
    activity.setState(BackgroundActivity::Running);

    Options options(app.arguments());

    if (options.command() == Options::SERVER) {

        DBusConnector dbusConnector;
        if (!dbusConnector.isConnected()) {
            qCritical("Connection to D-Bus and/or service registration failed.");
            return 1;
        }
        return app.exec();

    } else if (options.command() == Options::ALLOCATION) {

        command = QSharedPointer<Command>(new AllocationCommand);

    } else if (options.command() == Options::BALANCE) {

        command = QSharedPointer<Command>(
                    new BalanceCommand(options.batteryThreshold(),
                                       options.allocationThreshold()));

    }

    if (!command.isNull()) {
        QTimer::singleShot(0, command.data(), SLOT(start()));
    }

    // install signal handler to terminate cleanly on Ctrl-C
    struct sigaction intAction;
    intAction.sa_handler = terminate;
    sigemptyset(&intAction.sa_mask);
    intAction.sa_flags |= SA_RESTART;
    sigaction(SIGINT, &intAction, 0);

    return app.exec();
}
