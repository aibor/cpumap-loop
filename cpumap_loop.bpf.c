// SPDX-FileCopyrightText: 2024 Tobias Böhm <code@aibor.de>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <linux/bpf.h>
#include <linux/types.h>

#include <bpf/bpf_helpers.h>

/* Example program showing infinite cpumap looping. */

struct {
  __uint(type, BPF_MAP_TYPE_ARRAY);
  __uint(max_entries, 1);
  __type(key, __u32);
  __type(value, __u64);
} counters SEC(".maps");

struct {
  __uint(type, BPF_MAP_TYPE_CPUMAP);
  __uint(max_entries, 1);
  __type(key, __u32);
  __type(value, struct bpf_cpumap_val);
} cpumap SEC(".maps");

SEC("xdp")
int entry(struct xdp_md *ctx) {
  /* A usual task: redirect to some other CPU. */
  return bpf_redirect_map(&cpumap, 0, 0);
}

SEC("xdp/cpumap")
int cpumap_prog(struct xdp_md *ctx) {
  /* Count invocations of this program. */
  __u32 key = 0;
  __u64 *value = bpf_map_lookup_elem(&counters, &key);
  if (value)
    __sync_fetch_and_add(value, 1);

  /* By calling the entry program, we are going for another lap. */
  return entry(ctx);
}

char __license[] SEC("license") = "GPL";
