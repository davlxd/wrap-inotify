/*
 * Stack for pointers
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

#include "ptrstack.h"
#include <stdlib.h>
#include <assert.h>

ptrstack* init_ptrstack(uint64_t sz)
{
  void *vptr = calloc(1, sizeof(ptrstack) + sizeof(void*) * sz);
  ptrstack *ps = (ptrstack*)vptr;

  ps->elem = vptr + sizeof(ptrstack);
  ps->size = sz;
  ps->stacktop = 0;

  return ps;
}

void* ptrstack_top(ptrstack *ps)
{
  return ps->stacktop == 0 ? NULL : *(ps->elem + ps->stacktop - 1);
}

void* ptrstack_pop(ptrstack *ps)
{
  return ps->stacktop == 0 ? NULL : *(ps->elem + --ps->stacktop);
}

void* ptrstack_push(ptrstack *ps, void *eptr)
{
  // TODO: The assert here should be replaced by more detailed
  // stackoverflow check statements
  assert(ps->stacktop < ps->size);
  *(ps->elem + ps->stacktop++) = eptr;

  return eptr;
}

void cleanup_ptrstack(ptrstack *ps)
{
  assert(ps);
  free(ps);
}
