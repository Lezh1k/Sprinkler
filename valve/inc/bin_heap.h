#ifndef BIN_HEAP_H
#define BIN_HEAP_H

#include <stdbool.h>
#include <stdint.h>

typedef struct bh_node {
  int8_t unused;
} bh_node_t;

typedef int8_t (*pf_bh_node_cmp)(const bh_node_t *l, const bh_node_t *r);

typedef struct bheap {
  bh_node_t **arr;
  uint8_t capacity;
  uint8_t size;
  pf_bh_node_cmp pf_cmp;
} bheap_t;

void bh_insert(bheap_t *bh, bh_node_t *node);

const bh_node_t *bh_min(const bheap_t *bh);
bh_node_t *bh_pop(bheap_t *bh);

#endif
