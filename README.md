<!--
SPDX-FileCopyrightText: 2024 Tobias Böhm <code@aibor.de>

SPDX-License-Identifier: GPL-3.0-or-later
-->

# cpumap_loop

This is a very simple example eBPF/XDP program, that utilizes a CPU infintiely
with a single packet.

![cpumap_loop](cpumap_loop.gif)

## Requirements

* clang (tested with clang v16, at least v11 or later should work, though)
* libbpf

## Build

Either run make on your system:

```
$ make
```

Or use the container to build:

```
docker build --rm -t clbuild .
docker run --rm --volume $(pwd):/build clbuild
```

## Run

__Warning: Running the program results in CPU 0 being busy for about a minute.__

```
$ ./cpumap_loop
```
