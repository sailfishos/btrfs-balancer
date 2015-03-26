TEMPLATE = aux

OTHER_FILES += btrfs-balancer.conf

# configuration file (example)
conf.files = btrfs-balancer.conf
conf.path = /usr/share/btrfs-balancer/

INSTALLS += conf
