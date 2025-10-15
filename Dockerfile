FROM ubuntu:24.04

# Enable reproducible builds
ENV DEBIAN_FRONTEND=noninteractive
ENV SOURCE_DATE_EPOCH=1697328000
ENV TZ=UTC
ENV LANG=C.UTF-8

# Pin specific toolchain versions for hermetic builds
# These versions are selected for stability and security
ENV GCC_VERSION=13
ENV CLANG_VERSION=18
ENV NASM_VERSION=2.16.01

# Install build dependencies with pinned versions where possible
RUN apt-get update && apt-get install -y \
    # Core build tools
    build-essential=12.10ubuntu1 \
    gcc-${GCC_VERSION}=13.2.0-23ubuntu4 \
    g++-${GCC_VERSION}=13.2.0-23ubuntu4 \
    clang-${CLANG_VERSION} \
    lld-${CLANG_VERSION} \
    nasm \
    make=4.3-4.1build2 \
    git=1:2.43.0-1ubuntu7.1 \
    # QEMU for testing
    qemu-system-x86=1:8.2.2+ds-0ubuntu1.2 \
    # ISO building tools
    grub-pc-bin=2.12-1ubuntu7 \
    xorriso=1.5.6-1.1build1 \
    mtools=4.0.43-1build1 \
    # Additional security and build tools
    ca-certificates \
    wget \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Set default compiler to pinned version
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-${GCC_VERSION} 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-${GCC_VERSION} 100 && \
    update-alternatives --install /usr/bin/clang clang /usr/bin/clang-${CLANG_VERSION} 100 && \
    update-alternatives --install /usr/bin/lld lld /usr/bin/lld-${CLANG_VERSION} 100

# Create build workspace
WORKDIR /workspace

# Verify toolchain versions
RUN gcc --version && \
    clang --version && \
    nasm --version && \
    ld --version

# Usage:
# Build the image:
#   docker build -t limitlessos-build:v2.0 .
#
# Run a build (Linux/macOS/WSL):
#   docker run --rm -v $(pwd):/workspace -w /workspace limitlessos-build:v2.0 make all
#
# Run a build (Windows CMD):
#   docker run --rm -v %cd%:/workspace -w /workspace limitlessos-build:v2.0 make all
#
# For reproducible builds, ensure SOURCE_DATE_EPOCH is set consistently

