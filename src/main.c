#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <X11/Xlib.h>

#ifndef va_copy
#    define va_copy(d,s)      __va_copy(d,s)
#endif

struct fifo_t
{
     int fd;
     char *path;
     char buf[BUFSIZ];
};

int vasprintf (char **ret, const char *fmt, va_list args)
{
     va_list ap;
     char c = 0;
     int size = 0;

     if (ret == NULL)
          return -1;

     va_copy (ap, args);
     size = vsnprintf (&c, 1, fmt, args);
     *ret = calloc (size + 1, 1);

     if (!(*ret))
          return -1;

     size = vsprintf (*ret, fmt, ap);
     return size;
}

int asprintf (char **ret, const char *fmt, ...)
{
     va_list args;
     int sz = 0;

     va_start (args, fmt);
     sz = vasprintf (ret, fmt, args);
     va_end (args);

     return sz;
}
static void exec_uicb_function (Display *dpy, Window root, char *func, char *cmd)
{
     Atom utf8s = XInternAtom (dpy, "UTF8_STRING", false);
     XClientMessageEvent e;

     e.type         = ClientMessage;
     e.message_type = XInternAtom (dpy, "_WMFS_FUNCTION", false);
     e.window       = root;
     e.format       = 32;
     e.data.l[4]    = true;

     XChangeProperty (dpy,root, XInternAtom (dpy, "_WMFS_FUNCTION", false), utf8s,
                      8, PropModeReplace, (unsigned char *) func, strlen (func));

     if (!cmd) cmd = "";

     XChangeProperty (dpy, root, XInternAtom (dpy, "_WMFS_CMD", false), utf8s,
                      8, PropModeReplace, (unsigned char *) cmd, strlen (cmd));

     XSendEvent (dpy, root, false, StructureNotifyMask, (XEvent *) &e);
     XSync (dpy, False);
}

int main (int argc, char **argv)
{
     struct fifo_t fifo;
     Display *dpy;
     Window root;
     bool quit = false;

     if (!(dpy = XOpenDisplay (NULL)))
     {
          fprintf (stderr, "%s: Can't open X server\n", argv[0]);
          exit (EXIT_FAILURE);
     }

     root = RootWindow (dpy, DefaultScreen (dpy));

     asprintf (&fifo.path, "/tmp/wmfs-%s.fifo", DisplayString (dpy));

     if (access (fifo.path, F_OK) != -1)
          unlink (fifo.path);

     if (mkfifo (fifo.path, 0644) < 0)
     {
          perror (fifo.path);
          exit (EXIT_FAILURE);
     }

     while (!quit)
     {
          int ret;

          /* open the FIFO for reading */
          if ((fifo.fd = open (fifo.path, O_RDONLY)) < 0)
          {
               perror (fifo.path);
               exit (EXIT_FAILURE);
          }

          while ((ret = read (fifo.fd, fifo.buf, BUFSIZ - 1)) > 0)
          {
               char *p = NULL;
               char *func = NULL;
               char *arg = NULL;

               func = &fifo.buf[0];

               /* remove trailing spaces */
               if ((p = strchr (func, '\n')))
                    *p = '\0';

               /* get argument */
               if ((p = strchr (func, ' ')))
               {
                    *p = '\0';
                    arg = p + 1;
               }

               /* send function to WMFS */
               exec_uicb_function (dpy, root, func, arg);

               if (!strcmp (func, "quit"))
                    quit = true;
          }

          /* after reading, close the FIFO */
          close (fifo.fd);
          fifo.fd = 0;
     }

     XCloseDisplay (dpy);

     return EXIT_SUCCESS;
}
