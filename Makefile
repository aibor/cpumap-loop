# SPDX-FileCopyrightText: 2024 Tobias Böhm <code@aibor.de>
#
# SPDX-License-Identifier: GPL-3.0-or-later

CFLAGS += -O2 -g -v

PROG := cpumap_loop

$(PROG): $(PROG).c $(PROG).skel.h
	$(CC) $(CFLAGS) -Wall $< -lbpf -o $@

%.bpf.o: %.bpf.c
	clang -target bpf $(CFLAGS) -c -o $@ $<

%.skel.h: %.bpf.o
	bpftool gen skeleton $< name $* > $@

.PHONY: clean
clean:
	rm -rfv $(PROG){,bpf.o,.skel.h}
