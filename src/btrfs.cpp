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


#include "btrfs.h"

#include <QRegExp>
#include <QDebug>

namespace
{
// path of the btrfs tool
const QString BTRFS_PATH("/usr/sbin/btrfs");

// default root device to use
const QString ROOT_DEVICE("/dev/mmcblk0p28");

// regexp to retrieve size usage information from btrfs output
const QRegExp RE_USAGE("size ([0-9.]+\\w+) used ([0-9.]+\\w+)");

qint64 parseSize(const QString& size)
{
    double value = -1;

    if (size.endsWith("MiB")) {
        value = size.left(size.length() - 3).toDouble()
                * 1024 * 1024;
    } else if (size.endsWith("GiB")) {
        value = size.left(size.length() - 3).toDouble()
                * 1024 * 1024 * 1024;
    } else if (size.endsWith("TiB")) {
        // one day this may become useful...
        value = size.left(size.length() - 3).toDouble()
                * 1024 * 1024 * 1024 * 1024;
    }

    return static_cast<qint64>(value);
}

}

Btrfs::Btrfs(QObject *parent)
    : QObject(parent)
    , m_currentProcess(0)
{

}

void Btrfs::allocation()
{
    if (m_currentProcess) {
        emit allocationReceived(-1, -1);
        return;
    }

    m_currentProcess = new QProcess;
    m_currentProcess->setProgram(BTRFS_PATH);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "C");
    m_currentProcess->setProcessEnvironment(env);
    m_currentProcess->setArguments(QStringList()
                                   << "filesystem"
                                   << "show");

    connect(m_currentProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(slotAllocationFinished(int,QProcess::ExitStatus)));
    m_currentProcess->start(QProcess::ReadOnly);
}

void Btrfs::balance(int usage)
{
    if (m_currentProcess) {
        emit balanceFinished(false);
        return;
    }

    m_currentProcess = new QProcess;
    m_currentProcess->setProgram(BTRFS_PATH);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "C");
    m_currentProcess->setProcessEnvironment(env);
    m_currentProcess->setArguments(QStringList()
                                   << "balance"
                                   << "start"
                                   << QString("-dusage=%1").arg(usage)
                                   << "/");

    connect(m_currentProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(slotBalanceFinished(int,QProcess::ExitStatus)));
    m_currentProcess->start(QProcess::ReadOnly);
}

void Btrfs::slotAllocationFinished(int exitCode, QProcess::ExitStatus status)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(status)

    qint64 size = -1;
    qint64 used = -1;

    while (m_currentProcess->canReadLine()) {
        QString line = m_currentProcess->readLine().trimmed();

        if (!line.endsWith(" path " + ROOT_DEVICE)) {
            continue;
        }

        if (RE_USAGE.indexIn(line) != 1) {
            size = parseSize(RE_USAGE.cap(1));
            used = parseSize(RE_USAGE.cap(2));
        }
    }

    m_currentProcess->deleteLater();
    m_currentProcess = 0;

    emit allocationReceived(size, used);
}

void Btrfs::slotBalanceFinished(int exitCode, QProcess::ExitStatus status)
{
    Q_UNUSED(status)

    m_currentProcess->deleteLater();
    m_currentProcess = 0;

    if (exitCode == 0) {
        emit balanceFinished(true);
    } else {
        emit balanceFinished(false);
    }
}
