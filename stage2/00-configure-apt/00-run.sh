#!/bin/bash -e

echo "adding repo for edgetpu"
echo "deb https://packages.cloud.google.com/apt coral-edgetpu-stable main" > "${ROOTFS_DIR}/etc/apt/sources.list.d/coral-edgetpu.list"

curl https://packages.cloud.google.com/apt/doc/apt-key.gpg | gpg --dearmor > "${ROOTFS_DIR}/etc/apt/trusted.gpg.d/google.gpg"
on_chroot << EOF
apt-get update
apt-get dist-upgrade -y
EOF
