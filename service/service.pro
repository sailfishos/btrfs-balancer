TEMPLATE = aux

OTHER_FILES += \
    org.nemomobile.BtrfsBalancer.conf \
    org.nemomobile.BtrfsBalancer.service \
    btrfs-balance.service \
    btrfs-balancer.timer \
    dbus-org.nemomobile.BtrfsBalancer.service

# policy for accessing the D-Bus service
policy.files = org.nemomobile.BtrfsBalancer.conf
policy.path = /etc/dbus-1/system.d

# service manifest for D-Bus auto-activation
service.files = org.nemomobile.BtrfsBalancer.service
service.path = /usr/share/dbus-1/system-services

# systemd timer and service
systemd.files = btrfs-balance.service \
                btrfs-balancer.timer \
                dbus-org.nemomobile.BtrfsBalancer.service
systemd.path = /lib/systemd/system

INSTALLS += policy service systemd
