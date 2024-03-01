# SPDX-FileCopyrightText: 2024 Tobias Böhm <code@aibor.de>
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# syntax=docker/dockerfile:1

FROM debian:bookworm

RUN --mount=type=cache,target=/var/cache/apt \
  --mount=type=cache,target=/var/lib/apt \
  apt-get update && \
  DEBIAN_FRONTEND=noninteractive apt-get install --yes --no-install-recommends \
    clang \
	llvm \
	libbpf-dev \
	gcc-multilib \
	bpftool \
	make

WORKDIR /build
VOLUME /build

CMD make
