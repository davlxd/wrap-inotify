#include "wrap-inotify.h"
#include <sys/select.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/inotify.h>

extern int errno;

int main(int argc, char **argv)
{
  int n, i;
  fd_set rset;
  char buffer[BUF_LEN];
  char *path = "/home/i/Dropbox";
  int mfd;
  int maxfd;

  if (argv[1])
    path = argv[1];
  
  if (monitors_init(path, IN_MODIFY | IN_CREATE | IN_DELETE |
		    IN_MOVED_FROM | IN_MOVED_TO, &mfd)) {
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

	exit(0);

      }
    }

    /* if come from file descriptor wrap-inotify set */
    if (FD_ISSET(mfd, &rset)) {
      if (0 > (n = read(mfd, buffer, BUF_LEN))) {
	perror("@main(): read() failed");
	exit(1);
      }
      
      i = 0;
      while(i < n) {
	struct inotify_event *event = (struct inotify_event*)&buffer[i];
	if ( event->len ) {
	  if ( event->mask & IN_CREATE ) {
	    if ( event->mask & IN_ISDIR ) {
	      printf( "The directory %s was created.\n", event->name );       
	    }
	    else {
	      printf( "The file %s was created.\n", event->name );
	    }
	  }
	  else if ( event->mask & IN_DELETE ) {
	    if ( event->mask & IN_ISDIR ) {
	      printf( "The directory %s was deleted.\n", event->name );       
	    }
	    else {
	      printf( "The file %s was deleted.\n", event->name );
	    }
	  }
	  else if ( event->mask & IN_MODIFY ) {
	    if ( event->mask & IN_ISDIR ) {
	      printf( "The directory %s was modified.\n", event->name );
	    }
	    else {
	      printf( "The file %s was modified.\n", event->name );
	    }
	  }
	  
	  else if ( event->mask & IN_MOVED_TO ) {
	    if ( event->mask & IN_ISDIR ) {
	      printf( "The directory %s was moved to.\n", event->name );
	    }
	    else {
	      printf( "The file %s was moved to.\n", event->name );
	    }
	  }else if ( event->mask & IN_MOVED_FROM) {
	    if ( event->mask & IN_ISDIR ) {
	      printf( "The directory %s was moved from.\n", event->name );
	    }
	    else {
	      printf( "The file %s was moved from.\n", event->name );
	    }
	  }
	  
	  
	}
	i += EVENT_SIZE + event->len;

      } // end while(1)
    }
  }

}
