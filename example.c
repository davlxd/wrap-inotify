#include "wrap-inotify.h"
#include <sys/select.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

extern int errno;

int main(int argc, char **argv)
{
  int n;
  fd_set rset;
  char buffer[1024];
  char *path = "/home/i/foo";
  int mfd;
  int maxfd;

  if (argv[1])
    path = argv[1];
  
  if (monitors_init(path, IN_MODIFY|IN_CREATE|IN_DELETE|
		    IN_MOVED_FROM|IN_MOVED_TO, &mfd)) {
    fprintf(stderr, "@main(): monitors_init() failed\n");

    if (monitors_cleanup())
      fprintf(stderr, "@main(): monitors_cleanup() failed\n");
    
  }

  while(1) {
    
    FD_SET(fileno(stdin), &rset);
    FD_SET(mfd, &rset);
    maxfd = mfd > fileno(stdin) ? mfd : fileno(stdin);

    /* I/O polling */
    if (0 > select(maxfd+1, &rset, NULL, NULL, NULL)) {
      perror("@main(): select failed");
      exit(1);
    }

    /* if come from stdin */
    if (FD_ISSET(fileno(stdin), &rset)) {
      if (!fgets(buffer, 1024, stdin)) {
	perror("@main(): fgets failed");
	exit(1);
      }
      /* stdin type 'exit' to termitate monitor process */
      if (0 == strncmp(buffer, "exit", 4)) {
      	if(monitors_cleanup()) {
      	  perror("@main(): monitors_cleanup() failed");
      	  exit(1);
	}
      }
    }

    /* if come from file descriptor wrap-inotify set */
    if (FD_ISSET(mfd, &rset)) {
      if (0 > (n = read(mfd, buffer, 1024))) {
	perror("@main(): read() failed");
	exit(1);
      }
      buffer[n] = 0;
      printf("%s", buffer);
    }
  }

}
