/*
 * wrap functions of sha1.c
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

#define _FILE_OFFSET_BITS 64

#include "utils.h"
#include "sha1.h"
#include "digest.h"
#include <assert.h>
#include <string.h>
#include <inttypes.h>
#include <endian.h>
#include <errno.h>

extern int errno;


// cast the bytes at the end `digest' to uint64_t as hash key
uint64_t digest2hashkey(const char *digest, uint64_t mask)
{
  assert(digest);
  uint64_t keyspan = 0;
  
  size_t offset = DIGEST_BYTES > 8 ? (DIGEST_BYTES - 8) : 0;
  memcpy(&keyspan, digest+offset, min(8, DIGEST_BYTES));
  

  return be64toh(keyspan) & mask; // `be64toh()' defined in endian.h
}

unsigned char* str_digest(const char *text)
{
  assert(text);
  static unsigned char digest[DIGEST_BYTES];
  blk_SHA_CTX c;
  
  blk_SHA1_Init(&c);
  blk_SHA1_Update(&c, text, strlen(text));
  blk_SHA1_Final(digest, &c);

  return digest;
}


