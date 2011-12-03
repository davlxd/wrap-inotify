/*
 * Open hashing 
 *
 * Copyright (c) 2010, 2011 lxd <i@lxd.me>
 * 
 * This file is part of File Synchronization System(fss).
 *
 * fss is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, 
 * (at your option) any later version.
 *
 * fss is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fss.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hashtable.h"
#include <stdlib.h>
#include <assert.h>


uint64_t power_of_2_ceiling(uint64_t x)
{
  uint64_t legal_size = 1;

  if (x <= MIN_HASHTABLE_SIZE)
    return MIN_HASHTABLE_SIZE;
  if (x >= MAX_HASHTABLE_SIZE)
    return MAX_HASHTABLE_SIZE;

  while (legal_size < x)
    legal_size <<= 1;

  return legal_size;
}

hashtable* init_hashtable(uint64_t size, size_t offset)
{
  void *ptr = calloc(1, sizeof(hashtable) + sizeof(void*)*size);
  hashtable *htb = (hashtable*)ptr;
  htb->bucket = ptr + sizeof(hashtable);

  htb->size = size;
  htb->lce = 0;
  htb->chain_off = offset;

  return htb;
}


// cast void** to char** so it can be dereferenced
#define get_chain(htb, node) (*((char**)(node+htb->chain_off)))

void hashtable_insert(hashtable *htb, uint64_t key, void *node)
{
  assert(htb && node);
  void **head = htb->bucket + key;
  
  get_chain(htb, node) = *head;
  *(htb->bucket + key) = node;
}

void *hashtable_remove(hashtable *htb, uint64_t key, void *target)
{
  assert(htb && target);
  void **head = htb->bucket + key;
  
  void *prev = NULL;
  void *cur = *head;
  void *next = get_chain(htb, cur);

  while (next && cur != target) {
    prev = cur;
    cur = next;
    next = get_chain(htb, next);
  }

  assert(cur == target);

  if (prev)
    get_chain(htb, prev) = next;
  else
    *head = next;
  
  return target;
}

void* hashtable_search(hashtable *htb, uint64_t key, void *preq, int (*hit)(void *, void *))
{
  assert(htb && preq && hit);
  int i;
  void *node = *(htb->bucket + key);

  for (i = 0; node && !hit(preq, node); i++)
    node = get_chain(htb, node);

  if (i > htb->lce)
    htb->lce = i;

  return node;
}

void traverse_hashtable(hashtable *htb, void (*fn)(void *))
{
  assert(htb && fn);
  uint64_t i;
  void *cur, *next;
  cur = next = NULL;

  for (i = 0; i < htb->size; i++)
    if (*(htb->bucket + i)) {
      cur = (htb->bucket)[i];
      while (cur) {
	next = get_chain(htb, cur);
	fn(cur);
	cur = next;
      }
    }
  
}

void free_hashtable(hashtable *htb)
{
  assert(htb);
  free(htb);
}
