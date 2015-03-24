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

#include <QByteArray>
#include <QFile>
#include <QRegExp>
#include <QDebug>

namespace
{
// path of the btrfs tool
const QString BTRFS_PATH("/usr/sbin/btrfs");

// path to the device-specific partition configuration
const QString PARTITION_CONF("/usr/share/btrfs-balancer/btrfs-balancer.conf");

// system configuration key of partition
const QString CONF_PARTITION("PARTITION");

// system configuration key of root mountpoint
const QString CONF_ROOT_MOUNTPOINT("ROOT");

// regexp to retrieve size usage information from btrfs output
const QRegExp RE_USAGE("size ([0-9.]+\\w+) used ([0-9.]+\\w+)");

// regexp to retrieve the balancing progress from btrfs output
// note that besides percent numbers, there can also be nan
const QRegExp RE_PROGRESS(", +([0-9na]+)% left");

// interval in ms between polling the balancing progress
const int PROGRESS_TIMER_INTERVAL = 1000;

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
    , m_currentProgress(0)
    , m_isBalancing(false)
{
    loadDeviceConfiguration();

    connect(&m_progressTimer, SIGNAL(timeout()),
            this, SLOT(slotBalanceProgress()));
}

Btrfs::~Btrfs()
{
    // if the process gets aborted unexpectedly, at least try to cancel the
    // balancing
    if (m_currentProcess
            && m_isBalancing
            && m_deviceConfiguration.contains(CONF_ROOT_MOUNTPOINT)) {
        qWarning() << "Cancelling balancing operation.";
        QProcess::execute(BTRFS_PATH,
                          QStringList()
                          << "balance"
                          << "cancel"
                          << m_deviceConfiguration.value(CONF_ROOT_MOUNTPOINT));
    }
}

void Btrfs::loadDeviceConfiguration()
{
    m_deviceConfiguration.clear();

    QFile configFile(PARTITION_CONF);
    if (configFile.exists() && configFile.open(QIODevice::ReadOnly)) {
        while (!configFile.atEnd()) {
            const QByteArray line = configFile.readLine();
            int pos = line.indexOf('=');
            if (pos == -1 || line.startsWith('#')) {
                continue;
            }

            const QString key = QString::fromUtf8(line.left(pos)).trimmed();
            const QString value = QString::fromUtf8(line.mid(pos + 1)).trimmed();
            m_deviceConfiguration[key] = value;
        }
    } else {
        qWarning() << "Unable to read partition configuration:"
                   << PARTITION_CONF;
    }
}

void Btrfs::requestAllocation()
{
    if (m_currentProcess) {
        emit allocationReceived(-1, -1);
        return;
    } else if (!m_deviceConfiguration.contains(CONF_PARTITION)) {
        qCritical() << "Cannot get allocation. No partition configured.";
        emit allocationReceived(-1, -1);
        return;
    }

    m_currentProcess = new QProcess;
    m_currentProcess->setProgram(BTRFS_PATH);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "C");
    m_currentProcess->setProcessEnvironment(env);
    m_currentProcess->setArguments(QStringList() << "filesystem" << "show");

    connect(m_currentProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(slotAllocationFinished(int,QProcess::ExitStatus)));
    m_currentProcess->start(QProcess::ReadOnly);
}

void Btrfs::startBalance(int maxUsagePercent)
{
    if (m_currentProcess) {
        emit balanceFinished(false);
        return;
    }

    if (!m_deviceConfiguration.contains(CONF_ROOT_MOUNTPOINT)) {
        qCritical() << "Cannot get root path. No mountpoint configured.";
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
                                   << QString("-dusage=%1").arg(maxUsagePercent)
                                   << m_deviceConfiguration.value(CONF_ROOT_MOUNTPOINT));

    connect(m_currentProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(slotBalanceFinished(int,QProcess::ExitStatus)));
    m_currentProcess->start(QProcess::ReadOnly);

    m_currentProgress = 0;
    m_isBalancing = true;
    m_progressTimer.start(PROGRESS_TIMER_INTERVAL);
}

int Btrfs::getBalanceProgress()
{
    QProcess process;
    process.setProgram(BTRFS_PATH);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LANG", "C");
    process.setProcessEnvironment(env);
    process.setArguments(QStringList()
                         << "balance"
                         << "status"
                         << m_deviceConfiguration.value(CONF_ROOT_MOUNTPOINT));
    process.start(QProcess::ReadOnly);
    process.waitForFinished(1000);

    // don't care about the exit code... it is not 0 for success
    while (!process.atEnd()) {
        const QString line = process.readLine().trimmed();
        if (RE_PROGRESS.indexIn(line) != -1) {
            const QString progressString = RE_PROGRESS.cap(1);
            int progress = (progressString == "nan")
                    ? 0 // I guess that's 0 with bug in btrfs
                    : progressString.toInt();
            // btrfs balancing progress is going backwards, so invert it
            return qMax(100 - progress, 0);
        }
    }
    // could not determine progress
    return -1;
}

void Btrfs::slotBalanceProgress()
{
    int progress = getBalanceProgress();
    if (progress != m_currentProgress && progress != -1) {
        m_currentProgress = progress;
        emit balanceProgress(progress);
    }
}

void Btrfs::slotAllocationFinished(int exitCode, QProcess::ExitStatus status)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(status)

    qint64 size = -1;
    qint64 used = -1;

    while (m_currentProcess->canReadLine()) {
        QString line = m_currentProcess->readLine().trimmed();

        if (!line.endsWith(" path " + m_deviceConfiguration.value(CONF_PARTITION))) {
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
    m_isBalancing = false;
    m_progressTimer.stop();

    if (exitCode == 0) {
        emit balanceFinished(true);
    } else {
        emit balanceFinished(false);
    }
}
