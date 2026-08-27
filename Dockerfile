# ====================================================================
# File: Dockerfile
# TimeTracker — Persistent time tracking for Ubuntu
# Author: Maifee Ul Asad
# License: MIT
#
# Build:
#   docker build -t timetracker .
#
# Run (X11):
#   xhost +local:docker
#   docker run --rm -it \
#     -e DISPLAY=$DISPLAY \
#     -v /tmp/.X11-unix:/tmp/.X11-unix \
#     -v tt-data:/root/.local/share/MaifeeUlAsad/TimeTracker \
#     timetracker
#
# Run (Wayland — GNOME / Sway):
#   docker run --rm -it \
#     -e WAYLAND_DISPLAY=$WAYLAND_DISPLAY \
#     -e XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR \
#     -e QT_QPA_PLATFORM=wayland \
#     -v $XDG_RUNTIME_DIR/$WAYLAND_DISPLAY:/tmp/$WAYLAND_DISPLAY \
#     -v tt-data:/root/.local/share/MaifeeUlAsad/TimeTracker \
#     timetracker
# ====================================================================

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        qtbase5-dev \
        qtbase5-dev-tools \
        libqt5sql5-sqlite \
        pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY src/     ./src/
COPY Makefile ./

RUN make -j"$(nproc)"

RUN mkdir -p /root/.local/share/MaifeeUlAsad/TimeTracker

CMD ["./timetracker"]
