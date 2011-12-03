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

#ifndef _FSS_DIGEST_H_
#define _FSS_DIGEST_H_

#include <stdio.h>
#include <inttypes.h>


#define SHA1_BYTES   20
#define DIGEST_BYTES 20

// extract hashing key(uint64_t) from digest byte array
uint64_t digest2hashkey(const char *digest, uint64_t mask);
unsigned char* str_digest(const char *text);


#endif

    
