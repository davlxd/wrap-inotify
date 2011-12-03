/*
 * fss - File Synchronization System , detect file changes under
 *       specific directory on one client then synchronize to other
 *       client via a centralized server
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

#include "wrap-inotify.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>

extern int errno;

static void test_inotify(const char *path)
{
  int nready, fcntl_flags;
  ssize_t rdnum;
  char buf[BUF_LEN];
  struct pollfd pfd[2]; set0(pfd);
  
  int fd = init_watchers(path, IN_MODIFY | IN_CREATE | IN_DELETE |
			 IN_MOVED_FROM | IN_MOVED_TO, 10);
  
  fcntl_flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, fcntl_flags | O_NONBLOCK);

  
  pfd[0].fd = STDOUT_FILENO;
  pfd[1].fd = fd;
  pfd[0].events = pfd[1].events = POLLRDNORM;

  while(1) {
    if ((nready = poll(pfd, 2, 10000000)) < 0) {
      perror("poll returned error");
      exit(EXIT_FAILURE);
    }
    
    if (pfd[0].revents & POLLRDNORM) {
      while ((rdnum = read(STDIN_FILENO, buf, 2)) > 0)
	switch (*buf) {
	case 'q':
	  cleanup_watchers();
	  printf("\nExit\n");
	  exit(EXIT_SUCCESS);
	default:
	  printf("\nUnkown\n");
	}
    }

    if (pfd[1].revents & POLLRDNORM) {
      printf("fd readable\n");
      while ((rdnum = read(fd, buf, BUF_LEN)) > 0) ;
      
      if (rdnum < 0 && errno != EWOULDBLOCK) {
	perror("read from fd failed");
	exit(EXIT_FAILURE);
      }
    }
  }
}

int main(int argc, char *argv[])
{
  test_inotify("/home/i/fuck");
  exit(EXIT_SUCCESS);

}
