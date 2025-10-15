FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential clang lld nasm make git \
    qemu-system-x86 grub-pc-bin xorriso mtools \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace


# Usage:
# Linux/macOS/WSL:
#   docker build -t limitlessos-build .
#   docker run --rm -v $(pwd):/workspace -w /workspace limitlessos-build make all
# Windows (CMD):
#   docker run --rm -v %cd%:/workspace -w /workspace limitlessos-build make all
