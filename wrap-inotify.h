/*
 * wrap inotify, mainly implement recursively monitoring, header file
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

#ifndef _WRAP_INOTIFY_H_
#define _WRAP_INOTIFY_H_

#include <sys/inotify.h>

#ifndef MAX_PATH_LEN
#define MAX_PATH_LEN 1024
#endif

#ifndef INCLUDE_HIDDEN
#define INCLUDE_HIDDEN 0
#endif

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 *(EVENT_SIZE + 16))
#define NFTW_DEPTH 20

// monitoring STRUCT per sub-directory
typedef struct monitor
{
  char *pathname;
  int wd;

  struct monitor *prev;
  struct monitor *next;
} monitor;


/* API */
int monitors_init(const char *rootpath, uint32_t mask, int *fd);
int monitors_cleanup();



#endif
