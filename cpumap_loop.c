// SPDX-FileCopyrightText: 2024 Tobias Böhm <code@aibor.de>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cpumap_loop.skel.h"
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <signal.h>
#include <stdnoreturn.h>
#include <unistd.h>

static struct cpumap_loop *skel = NULL;
static __u32 cpumap_key = 0;
static int err = 0;

static noreturn void cleanup() {
  if (skel) {
    bpf_map__delete_elem(skel->maps.cpumap, &cpumap_key, sizeof(cpumap_key), 0);
    cpumap_loop__destroy(skel);
  }

  exit(err);
}

/*
 * This program loads bpf objects that result in an infinite loop and utilizes
 * CPU 0 to the full. Not recommended to be used on any kind of production
 * machine.
 */
int main() {
  struct bpf_test_run_opts run_opts = {0};
  struct bpf_cpumap_val cpumap_value = {0};
  __u32 counter_key = 0;
  __u64 counter_value = 0, last_counter_value = 0;
  int prog_fd = -1;

  skel = cpumap_loop__open_and_load();
  if (!skel)
    goto cleanup;

  /*
   * Setup the cpumap with the program of type "xdp/cpumap" that counts its
   * invocations and calls the entry program again, which results in another
   * cpumap redirect.
   */
  cpumap_value.qsize = 16;
  cpumap_value.bpf_prog.fd = bpf_program__fd(skel->progs.cpumap_prog);
  err = bpf_map__update_elem(skel->maps.cpumap, &cpumap_key, sizeof(cpumap_key),
                             &cpumap_value, sizeof(cpumap_value), 0);
  if (err)
    goto cleanup;

  /* Send a single "packet" to the simple XDP program, that just does a cpumap
   * redirect.
   */
  prog_fd = bpf_program__fd(skel->progs.entry);
  run_opts.sz = sizeof(run_opts);
  run_opts.flags = BPF_F_TEST_XDP_LIVE_FRAMES;
  run_opts.repeat = 1;
  err = bpf_prog_test_run_opts(prog_fd, &run_opts);
  if (err)
    goto cleanup;

  signal(SIGINT, cleanup);
  signal(SIGTERM, cleanup);

  printf("\nPrints number of loops done and sleeps for a second 60 times.\n"
         "Abort early with CTRL-C.\n\n");
  for (int i = 0; i < 60; i++) {
    /* Print cpumap program invocations every second. */
    err = bpf_map__lookup_elem(skel->maps.counters, &counter_key,
                               sizeof(counter_key), &counter_value,
                               sizeof(counter_value), 0);
    if (err)
      goto cleanup;

    printf("%d: cpumap loops: %llu total, %llu since last lookup\n", i,
           counter_value, counter_value - last_counter_value);
    last_counter_value = counter_value;
    sleep(1);
  }

cleanup:
  cleanup();
  return err;
}
