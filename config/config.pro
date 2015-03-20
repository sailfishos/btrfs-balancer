TEMPLATE = aux

OTHER_FILES += \
    btrfs-sbj.conf

# policy for accessing the D-Bus service
config.files = btrfs-sbj.conf
config.path = /usr/share/btrfs-balancer

INSTALLS += config
