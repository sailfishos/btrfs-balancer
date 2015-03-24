TEMPLATE = aux

OTHER_FILES += \
    btrfs-sbj.conf

# product-specific configuration
config.files = btrfs-sbj.conf
config.path = /usr/share/btrfs-balancer

INSTALLS += config
