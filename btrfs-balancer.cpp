/****************************************************************************************
**
** Copyright (C) 2015 Jolla Ltd.
** Contact: Thomas Perl <thomas.perl@jolla.com>
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <btrfs/ioctl.h>

#include <initializer_list>

#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <keepalive-glib/keepalive-cpukeepalive.h>

#include "config.h"


static inline void FAIL(const char *msg)
{
    fprintf(stderr, "FATAL: %s\n", msg);
    exit(1);
}

struct AutoCloseFd {
    AutoCloseFd(int fd) : fd(fd) { }
    ~AutoCloseFd() { if (fd != -1) { ::close(fd); } }
    int fd;
};

struct AutoFreeStr {
    AutoFreeStr() : s(0) {}
    ~AutoFreeStr() { if (s) { ::free(s); } }
    char *s;
};

struct ScopedKeepAlive {
    ScopedKeepAlive() : pid(fork()) { if (pid == 0) { exit(run()); } }
    ~ScopedKeepAlive() { if (pid != 0) { kill(pid, SIGKILL); } }
    int run();
    pid_t pid;
};


int ScopedKeepAlive::run()
{
    /**
     * Based on:
     * https://github.com/nemomobile/nemo-keepalive/blob/master/examples-glib/block-suspend.c
     **/

    GMainLoop *mainloop_handle = g_main_loop_new(0, 0);

    DBusConnection *system_bus = dbus_bus_get(DBUS_BUS_SYSTEM, NULL);
    if (!system_bus) {
        FAIL("Could not connect to system bus");
    }
    dbus_connection_setup_with_g_main(system_bus, 0);

    cpukeepalive_t *cpukeepalive = cpukeepalive_new();
    cpukeepalive_start(cpukeepalive);
    g_main_loop_run(mainloop_handle);
    cpukeepalive_stop(cpukeepalive);
    cpukeepalive_unref(cpukeepalive);

    dbus_connection_unref(system_bus);
    g_main_loop_unref(mainloop_handle);

    return 0;
}


float get_btrfs_usage(const char *mountpoint, const char *device)
{
    AutoCloseFd mount(open(mountpoint, O_RDONLY));

    if (mount.fd == -1) {
        FAIL("Could not open fd");
    }

    struct btrfs_ioctl_fs_info_args fs_info = {0};

    if (ioctl(mount.fd, BTRFS_IOC_FS_INFO, &fs_info) != 0) {
        FAIL("ioctl(BTRFS_IOC_FS_INFO) failed");
    }

    for (__u64 i=1; i<=fs_info.num_devices; i++) {
        struct btrfs_ioctl_dev_info_args dev_info = {0};

        dev_info.devid = i;
        if (ioctl(mount.fd, BTRFS_IOC_DEV_INFO, &dev_info) != 0) {
            FAIL("ioctl(BTRFS_IOC_DEV_INFO) failed");
        }

        if (::strcmp((const char *)dev_info.path, device) == 0) {
            return (float)dev_info.bytes_used / (float)dev_info.total_bytes;
        }
    }

    FAIL("Could not find device");
    return 0.f;
}


bool do_balance(const char *mountpoint, int usage)
{
    AutoFreeStr cmd;
    if (asprintf(&cmd.s, "btrfs balance start -dusage=%d %s", usage, mountpoint) == -1) {
        FAIL("Could not assemble btrfs command");
    }
    return (system(cmd.s) == 0);
}


int main(int argc, char *argv[])
{
    ScopedKeepAlive keepalive;

    float usage = get_btrfs_usage(MOUNTPOINT, ROOTDEV);
    printf("Usage: %.0f%%\n", usage * 100.f);

    if (usage > BALANCE_THRESHOLD) {
        // We start from empty block groups and advance to more full ones
        for (auto usage: { 0, 10, 20, 40, 60, 80, 96 }) {
            printf("Balancing with usage >= %d%%\n", usage);
            if (!do_balance(MOUNTPOINT, usage)) {
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}
