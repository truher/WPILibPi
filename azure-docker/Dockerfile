FROM debian:bullseye

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get -y update && \
    apt-get -y install --no-install-recommends \
        git vim parted pkg-config nodejs \
        quilt coreutils qemu-user-static debootstrap zerofree zip dosfstools \
        libarchive-tools libcap2-bin rsync grep udev xz-utils curl xxd file kmod bc\
        binfmt-support ca-certificates qemu-utils kpartx fdisk gpg pigz\
        crossbuild-essential-arm64 \
        build-essential cmake python3 python3-distutils python3-jinja2 ant sudo openjdk-17-jdk \
    && rm -rf /var/lib/apt/lists/*

LABEL "com.azure.dev.pipelines.agent.handler.node.path"="/usr/bin/node"
