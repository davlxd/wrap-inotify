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


#ifndef _FSS_HASHTABLE_H
#define _FSS_HASHTABLE_H

#include <inttypes.h>
#include <stdlib.h>

#define MIN_HASHTABLE_SIZE ((uint32_t)1 << 8)
#define MAX_HASHTABLE_SIZE ((uint32_t)1 << 16)

typedef struct {
  uint64_t size;
  uint32_t lce; // Longest Chain Ever

  // `chain_off' is the offset of struct member `chain' of the struct
  // `*bucket' point to
  // I can use this to acquire the next node address of `*bucket' without
  // knowning specific struct type `*bucket' point to
  size_t chain_off;
  void **bucket;
  
} hashtable;

uint64_t power_of_2_ceiling(uint64_t x);
hashtable* init_hashtable(uint64_t size, size_t offset);
void hashtable_insert(hashtable *htb, uint64_t key, void *node);
void *hashtable_remove(hashtable *htb, uint64_t key, void *target);

// encapsulate all prerequisite of hit to node type
void *hashtable_search(hashtable *htb, uint64_t key, void *preq,
		       int (*hit)(void *, void *));
void traverse_hashtable(hashtable *htb, void (*fn)(void *));
void free_hashtable(hashtable *htb);




#endif
