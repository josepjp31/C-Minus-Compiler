FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive LANG=C.UTF-8 LC_ALL=C.UTF-8

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential gcc g++ make cmake ninja-build \
    vim nano sudo \
    flex bison libfl-dev \
    git ca-certificates pkg-config \
    python3 python3-pip \
    curl wget \
    gdb valgrind \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /work

# Default: interactive shell inside the container
CMD ["bash"]
