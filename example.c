#include "wrap-inotify.h"
#include <sys/select.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

extern int errno;

int main()
{
  int n;
  fd_set rset;
  char buffer[1024];
  char *path = "/home/foo";
  int mfd;
  int maxfd;
  
  if (monitors_init(path,
		    IN_MODIFY|IN_CREATE|IN_DELETE|
		    IN_MOVED_FROM|IN_MOVED_TO, &mfd)) {
    fprintf(stderr,
	    "@main(): monitors_init() fails\n");
    exit(0);
  }

  while(1) {
    
    FD_SET(fileno(stdin), &rset);
    FD_SET(mfd, &rset);
    maxfd = mfd > fileno(stdin) ? mfd : fileno(stdin);

    /* I/O polling */
    if (0 > select(maxfd+1, &rset, NULL, NULL, NULL)) {
      perror("@main(): select fails");
      exit(1);
    }

    /* if come from stdin */
    if (FD_ISSET(fileno(stdin), &rset)) {
      if (!fgets(buffer, 1024, stdin)) {
	perror("@main(): fgets fails");
	exit(1);
      }
      if (0 == strncmp(buffer, "exit", 4)) {
	if(monitors_cleanup()) {
	  perror("@main(): monitors_cleanup() fails");
	  exit(1);
	}
	if(0 > close(mfd)) {
	  perror("@main(): close mfd fails");
	  exit(1);
	}
	return 0;
      }
    }

    /* if come from file descriptor wrap-inotify set */
    if (FD_ISSET(mfd, &rset)) {
      if (0 > (n = read(mfd, buffer, 1024))) {
	perror("@main(): read() fails");
	exit(1);
      }
      buffer[n] = 0;
      printf("%s", buffer);
    }
  }

  return 0;
}
