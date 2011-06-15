/*
 * wrap inotify, mainly implement recursively monitoring.
 * symbolic links is followedd.
 *
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
#define _XOPEN_SOURCE 500

#include "wrap-inotify.h"
#include <ftw.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>

extern int errno;

static char rootpath[MAX_PATH_LEN];
static pthread_t tid;
static uint32_t mask;
static int inotify_fd;
static int pfd[2];

static monitor *monitor_tail;

static void *thread_new(void *arg);
static int monitors_polling();
static int show_all_monitors();
static int monitor_search(int wd, monitor **target);
static int combine_path(struct inotify_event *, char *);
static int monitor_connect(const char *, const struct stat *, 
			   int , struct FTW *);
static int monitor_connect_recursively(struct inotify_event *event);
static int monitor_disconnect(monitor *);
static int monitor_disconnect_recursively(struct inotify_event *event);


/*
 * arg:root_path -> root directory we monitor,
 * arg:mask -> inotify mask
 * arg:fd -> file descriptor created
 *
 * return:1 -> error
 * return:0 -> ok
 */
int monitors_init(const char *path, uint32_t m, int *fd)
{
  size_t path_len;
  path_len = strlen(path);
  
  strncpy(rootpath, path, path_len+1);
  if (path_len == '/')
    rootpath[path_len] = 0;

  mask = m;

  if (pipe(pfd) < 0) {
    perror("@monitors_init(): pipe() failed");
    return 1;
  }


  /* if (dup2(pfd[0], *fd) < 0) { */
  /*   perror("@monitors_init(): dup2() failed"); */
  /*   return 1; */
  /* } */

  *fd = pfd[0];
   
  inotify_fd = inotify_init();
  if (inotify_fd < 0) {
    perror("@monitors_init(): inotify_init() failed");
    return 1;
  }
    
  /* the first-time, iteration, initial doubly linked list */
  if (nftw(rootpath, monitor_connect, NFTW_DEPTH, FTW_DEPTH) != 0) {
    perror("@monitors_init(): nftw() failed");
    return 1;
  }
  monitor_tail->next = NULL;

  if (pthread_create(&tid, NULL, thread_new, NULL) < 0) {
    perror("@monitors_init(): pthread_create() failed");
    return 1;
  }

  return 0;
}


static void *thread_new(void *arg)
{
  if (monitors_polling()) {
    fprintf(stderr, "@thread_new(): monitors_polling() failed");
    return (void*)1;
  }

  pthread_exit((void*)0);
}


static int show_all_monitors()
{
  monitor *temp;

  for (temp = monitor_tail; temp != NULL; temp = temp->prev)
    printf("--%s--\n", temp->pathname);

  return 0;

}

static int monitors_polling()
{
  monitor *temp_monitor;
  int i = 0;
  int len;
  char buf[BUF_LEN];
  
  while (1) {
    if (0 > (len = read(inotify_fd, buf, BUF_LEN))) {
      perror("@thread_new(): read failed");
      return 1;
    }
    write(pfd[1], buf, len);
	
    i = 0;
    /* inotify handle routine */
    while (i < len) {
      struct inotify_event *event = (struct inotify_event *)&buf[i];
      if (!event->len)
	continue;

      // show_all_monitors();

      /* DIR detected*/
      if (event->mask & IN_ISDIR) {

	/* if CREATE detected, this new dir
	 * should be added to linked list */
	if (event->mask & IN_CREATE) {
	  if (monitor_connect_recursively(event)) {
	    fprintf(stderr,"@monitors_polling(): monitor_connect_recursively() failed");
	    return 1;
	  }

	  /* if DELETE detected, this departed dir
	   * should be removed from linked list */
	} else if (event->mask & IN_DELETE) {
	  if (monitor_disconnect_recursively(event)) {
	    fprintf(stderr,
		    "@minitors_poll(): monitor_disconnect_recursively() failed");
	    return 1;
	  }

	  /* MOVED_FROM detected, same as DELETE */
	} else if (event->mask & IN_MOVED_FROM) {
	  printf("moved from ");
	  if (monitor_disconnect_recursively(event)) {
	    fprintf(stderr,
		    "@minitors_poll(): monitor_disconnect_recursively() failed");
	    return 1;
	  }

	  /* MOVED_TO detected, same as CREATE */
	} else if (event->mask & IN_MOVED_TO) {
	  printf("moved to ");
	  if (monitor_connect_recursively(event)) {
	    fprintf(stderr,"@monitors_polling(): monitor_connect_recursively() failed");
	    return 1;
	  }

	}

      }
	
      i += sizeof(struct inotify_event) + event->len;

    }  //end while(i < len)
  }  //end while(1)
}  //end function


int monitors_cleanup()
{
  monitor *temp_monitor;

  if (pthread_cancel(tid) != 0) {
    perror("@monitors_cleanup(): pthread_cancel() failed");
    return 1;
  }
  
  for (temp_monitor = monitor_tail; temp_monitor != NULL;
       temp_monitor = temp_monitor->prev)
    monitor_disconnect(temp_monitor);

  return 0;
}


static int monitor_search(int wd, monitor **target)
{
  monitor *temp;
  for (temp = monitor_tail; temp != NULL; temp = temp->prev)
    if (temp->wd == wd) {
      *target = temp;
      break;
    }
  if (temp == NULL) {
    fprintf(stderr, "@moinitor_search(): failed, wd-%d- invalid\n", wd);
    return 1;
  }

  return 0;
    
}
/* connect direcotry's full pathname with file(dir)'s name */
static int combine_path(struct inotify_event *event, char *fullpath)
{
  char *ptr;
  monitor *temp;
  
  if (monitor_search(event->wd, &temp)) {
    fprintf(stderr, "@combine_path(): monitor_search() failed\n");
    return 1;
  }
  
  if (!strncpy(fullpath, temp->pathname, strlen(temp->pathname)+1)) {
    perror("@combine_path: strncpy() failed\n");
    return 1;
  }

  ptr = fullpath + strlen(temp->pathname);
  
  if (*(ptr-1) != '/') {
    *ptr++ = '/';
    *ptr = 0;
  }

  if (!strncpy(ptr, event->name, strlen(event->name)+1)) {
    perror("@combine_path: strncpy() failed\n");
    return 1;
  }

  return 0;
}

/* called when a directory is created or moved in at current
 * monitoring directory,
 * iteration nftw() should be invoked here because this
 * new directory may still contain sub-directory*/
static int monitor_connect_recursively(struct inotify_event *event)
{
  char fullpath[MAX_PATH_LEN];
  
  if (combine_path(event, fullpath)) {
    fprintf(stderr,
	    "@monitor_connect_recursively(): combine_path() failed\n");
    return 1;
  }
  
  if (nftw(fullpath, monitor_connect, NFTW_DEPTH, FTW_DEPTH) != 0) {
    perror("@monitor_con_via_fpath(): nftw() failed");
    return 1;
  }
  monitor_tail->next = NULL;
  
  return 0;
}
  

/* create a linked list node, then add it to linked list */
static int monitor_connect(const char *path, const struct stat *sb,
			   int typeflag, struct FTW *fb)
{
  if (!S_ISDIR(sb->st_mode))
    return 0;
  
  /* create part */
  monitor *temp_monitor;
  temp_monitor = (monitor*)calloc(1, sizeof(monitor));
  temp_monitor->pathname = (char*)calloc(strlen(path)+1, sizeof(char));
  if (!strncpy(temp_monitor->pathname, path, strlen(path)+1)) {
    perror("@monitor_connect(): strncpy() failed");
    return 1;
  }

  temp_monitor->wd = inotify_add_watch(inotify_fd, path, mask);

  /* connect part */
  temp_monitor->prev = monitor_tail;

  /* cur->next = temp is omitted if tail_monitor is NULL
   * which means temp_monitor is head of this linked list
   */
  if (monitor_tail) 
    monitor_tail->next = temp_monitor;

  monitor_tail = temp_monitor;

  return 0;
}


/* called when a directory is removed, we also remove all sub-dir
   under it via a trick on strncmp */
static int monitor_disconnect_recursively(struct inotify_event *event)
{
  monitor *temp;
  char fullpath[MAX_PATH_LEN];

  if (combine_path(event, fullpath)) {
    fprintf(stderr,
	    "@monitor_disconnect_recursively(): combine_path() failed\n");
    return 1;
  }
  
  
  for (temp = monitor_tail; temp != NULL; temp = temp->prev)
    /* compare with length of fullpath, so sub-dir will be removed
     * from linked list, too */
    if (0 == strncmp(temp->pathname, fullpath, strlen(fullpath)))
      monitor_disconnect(temp);

  return 0;
}

/* first disconnect this node from linked list,
 * then free it */
static int monitor_disconnect(monitor *this_monitor)
{
  /* if this_monitor is only node, do nothing */
  if (!this_monitor->next && !this_monitor->prev)
    ;
  else if (!this_monitor->next) {  //tail of linked list
    this_monitor->prev->next = NULL;
    monitor_tail = this_monitor->prev;
  } else if (!this_monitor->prev) { //head of linked list
    this_monitor->next->prev = NULL;
  } else {
    this_monitor->prev->next = this_monitor->next;
    this_monitor->next->prev = this_monitor->prev;
  }

  inotify_rm_watch(inotify_fd, this_monitor->wd);
  
  free(this_monitor->pathname);
  free(this_monitor);

  return 0;
}

