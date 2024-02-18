#include "bin_heap.h"
#include <stddef.h>

static uint8_t bh_parent(uint8_t i) { return (i - 1) / 2; }
static uint8_t bh_left(uint8_t i) { return (2 * i + 1); }
static uint8_t bh_right(uint8_t i) { return (2 * i + 2); }
static void bh_swap(bh_node_t **l, bh_node_t **r) {
  bh_node_t *t = *l;
  *l = *r;
  *r = t;
}
static void bh_heapify(bheap_t *bh, uint8_t idx);
//////////////////////////////////////////////////////////////

const bh_node_t *bh_min(const bheap_t *bh) { return *bh->arr; }
//////////////////////////////////////////////////////////////

void bh_insert(bheap_t *bh, bh_node_t *node) {
  if (bh->size >= bh->capacity)
    return;

  uint8_t i = bh->size++;
  bh->arr[i] = node;
  while (i && bh->pf_cmp(bh->arr[bh_parent(i)], bh->arr[i]) > 0) {
    bh_swap(&bh->arr[i], &bh->arr[bh_parent(i)]);
    i = bh_parent(i);
  }
}
//////////////////////////////////////////////////////////////

void bh_heapify(bheap_t *bh, uint8_t idx) {
  uint8_t ri = bh_right(idx);
  uint8_t li = bh_left(idx);
  uint8_t smallest = idx;
  if (li < bh->size && bh->pf_cmp(bh->arr[li], bh->arr[smallest]) < 0)
    smallest = li;
  if (ri < bh->size && bh->pf_cmp(bh->arr[ri], bh->arr[smallest]) < 0)
    smallest = ri;

  if (smallest != idx) {
    bh_swap(&bh->arr[idx], &bh->arr[smallest]);
    bh_heapify(bh, smallest);
  }
}
//////////////////////////////////////////////////////////////

bh_node_t *bh_pop(bheap_t *bh) {
  if (bh->size == 0) {
    return NULL;
  }

  if (bh->size == 1) {
    --bh->size;
    return bh->arr[0];
  }

  bh_node_t *node = bh->arr[0];
  bh->arr[0] = bh->arr[--bh->size];
  bh_heapify(bh, 0);
  return node;
}
//////////////////////////////////////////////////////////////

