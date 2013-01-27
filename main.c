/*
 *  WMBlueClock - a clock dockapp
 *
 *  Copyright (C) 2003 Draghicioiu Mihai Andrei <misuceldestept@go.ro>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "mwm.h"
#include "menu.h"

#include "pixmap.xpm"
#include "icon.xpm"

#define WINDOW_WIDTH 64
#define WINDOW_HEIGHT 64
#define WINDOW_NAME "WMBlueClock"
#define ICON_NAME "wmblueclock"
#define CLASS_NAME "wmblueclock"
#define CLASS_CLASS "WMBlueClock"

#define DIGIT_X 0
#define DIGIT_Y 72
#define DIGIT_WIDTH 8
#define DIGIT_HEIGHT 12
#define MERIDIAN_AM_X 80
#define MERIDIAN_AM_Y 64
#define MERIDIAN_PM_X 98
#define MERIDIAN_PM_Y 64
#define MERIDIAN_WIDTH 18
#define MERIDIAN_HEIGHT 8
#define NUMBER_X 0
#define NUMBER_Y 64
#define NUMBER_WIDTH 8
#define NUMBER_HEIGHT 8

#define OPT_DISPLAY NULL
#define OPT_MILISECS 1000
#define OPT_BGCOLOR  "black"
#define OPT_OFFCOLOR "rgb:00/95/E0"
#define OPT_ONCOLOR  "rgb:87/D7/FF"
#define OPT_WINDOW 0
#define OPT_SHAPE 1
#define OPT_AMPM 0

int            argc;
char         **argv;
Display       *display;
int            screen;
Colormap       colormap;
Visual        *visual;
int            depth;
GC             backgc, offgc, ongc;
Window         rootwindow, window, iconwindow, mapwindow;
XEvent         event;
int            exitloop;
MWMHints       mwmhints;
Atom           wm_delete_window, _motif_wm_hints;
Pixmap         buffer, pixmap, pixmask, iconpixmap;
unsigned long  bgcolor, offcolor, oncolor;
struct timeval tv = { 0, 0 };
int            oldx, oldy;
menu_t *       m;

char *opt_display  = OPT_DISPLAY;
int   opt_milisecs = OPT_MILISECS;
char *opt_bgcolor  = OPT_BGCOLOR;
char *opt_oncolor  = OPT_ONCOLOR;
char *opt_offcolor = OPT_OFFCOLOR;
int   opt_window   = OPT_WINDOW;
int   opt_shape    = OPT_SHAPE;
int   opt_ampm     = OPT_AMPM;

unsigned long get_color(char *colorname)
{
 XColor color, color2;
 color.pixel = 0;
 XAllocNamedColor(display, colormap, colorname, &color2, &color);
 return color2.pixel;
}

void bad_option(int arg)
{
 fprintf(stderr, "%s needs an argument.\n"
                 "Try `%s --help' for more information.\n",
                 argv[arg-1], argv[0]);
 exit(1);
}

void print_usage()
{
 printf("Usage: %s [option]\n"
	"Options:\n"
	"    -h,  --help           Print out this help and exit\n"
	"    -v,  --version        Print out version number and exit\n"
	"    -d,  --display  <dpy> The X11 display to connect to\n"
	"    -m,  --milisecs <ms>  The number of milisecs between updates\n"
	"    -b,  --bgcolor  <col> The background color\n"
	"    -f,  --offcolor <col> The color for Off leds\n"
	"    -o,  --oncolor  <col> The color for text and On leds\n"
	"    -w,  --window         Run in a window\n"
	"    -nw, --no-window      Run as a dockapp\n"
	"    -s,  --shape          Use XShape extension\n"
	"    -ns, --no-shape       Don't use XShape extension\n"
	"    -12, --ampm           Use 12 hour mode\n",
	argv[0]);
 exit(0);
}

void print_version()
{
 printf(WINDOW_NAME " version " VERSION "\n");
 exit(0);
}

void parse_args()
{
 int n;

 for(n = 1; n < argc; n++)
 {
  if(!strcmp(argv[n], "-h") ||
     !strcmp(argv[n], "--help"))
  {
   print_usage();
  }
  else if(!strcmp(argv[n], "-v") ||
          !strcmp(argv[n], "--version"))
  {
   print_version();
  }
  else if(!strcmp(argv[n], "-d") ||
     !strcmp(argv[n], "--display"))
  {
   if(argc <= ++n)
   {
    bad_option(n);
   }
   opt_display = argv[n];
  }
  else if(!strcmp(argv[n], "-m") ||
          !strcmp(argv[n], "--milisecs"))
  {
   if(argc <= ++n)
   {
    bad_option(n);
   }
   opt_milisecs = strtol(argv[n], NULL, 0);
  }
  else if(!strcmp(argv[n], "-b") ||
          !strcmp(argv[n], "--bgcolor"))
  {
   if(argc <= ++n)
   {
    bad_option(n);
   }
   opt_bgcolor = argv[n];
  }
  else if(!strcmp(argv[n], "-f") ||
          !strcmp(argv[n], "--offcolor"))
  {
   if(argc <= ++n)
   {
    bad_option(n);
   }
   opt_offcolor = argv[n];
  }
  else if(!strcmp(argv[n], "-o") ||
          !strcmp(argv[n], "--oncolor"))
  {
   if(argc <= ++n)
   {
    bad_option(n);
   }
   opt_oncolor = argv[n];
  }
  else if(!strcmp(argv[n], "-w") ||
          !strcmp(argv[n], "--window"))
  {
   opt_window = 1;
  }
  else if(!strcmp(argv[n], "-nw") ||
          !strcmp(argv[n], "--no-window"))
  {
   opt_window = 0;
  }
  else if(!strcmp(argv[n], "-s") ||
          !strcmp(argv[n], "--shape"))
  {
   opt_shape = 1;
  }
  else if(!strcmp(argv[n], "-ns") ||
          !strcmp(argv[n], "--no-shape"))
  {
   opt_shape = 0;
  }
  else if(!strcmp(argv[n], "--ampm") ||
		  !strcmp(argv[n], "-12"))
  {
   opt_ampm = 1;
  }
  else
  {
   fprintf(stderr, "Bad option `%s'.\n"
                   "Try `%s --help' for more information.\n",
		   argv[n], argv[0]);
   exit(1);
  }
 }
}

#include "common.c"

void draw_digit(int x, int y, int n)
{
 if(n>=0)
  XCopyArea(display, pixmap, buffer, backgc,
            DIGIT_X + n * DIGIT_WIDTH, DIGIT_Y,
	    DIGIT_WIDTH, DIGIT_HEIGHT, x, y);
 else
  XFillRectangle(display, buffer, backgc, x, y, DIGIT_WIDTH, DIGIT_HEIGHT);
}

void draw_number(int x, int y, int n)
{
 if(n>=0)
  XCopyArea(display, pixmap, buffer, backgc,
            NUMBER_X + n * NUMBER_WIDTH, NUMBER_Y,
	    NUMBER_WIDTH, NUMBER_HEIGHT, x, y);
 else
  XFillRectangle(display, buffer, backgc, x, y, NUMBER_WIDTH, NUMBER_HEIGHT);
}

void draw_meridian(int x, int y, int pm)
{
 if(pm == -1)
  XFillRectangle(display, buffer, backgc, x, y, MERIDIAN_WIDTH, MERIDIAN_HEIGHT);
 else if(pm == 0)
  XCopyArea(display, pixmap, buffer, backgc, MERIDIAN_AM_X, MERIDIAN_AM_Y, MERIDIAN_WIDTH, MERIDIAN_HEIGHT, x, y);
 else if(pm == 1)
  XCopyArea(display, pixmap, buffer, backgc, MERIDIAN_PM_X, MERIDIAN_PM_Y, MERIDIAN_WIDTH, MERIDIAN_HEIGHT, x, y);
}

void draw_window()
{
 struct tm *atm;
 time_t     atime;

 atime = time(NULL);
 atm = localtime(&atime);

 if(opt_ampm)
 {
  if(atm->tm_hour > 12)
  {
   atm->tm_hour -= 12;
   draw_meridian(19, 30, 1);
  }
  else
   draw_meridian(19, 30, 0);
 }
 else
  draw_meridian(19, 30, -1);

 draw_digit(19, 10, (atm->tm_hour > 9) ? atm->tm_hour / 10 : -1);
 draw_digit(29, 10, atm->tm_hour % 10);
 draw_digit(41, 10, atm->tm_min / 10);
 draw_digit(51, 10, atm->tm_min % 10);

 draw_number(41, 30, atm->tm_sec / 10);
 draw_number(51, 30, atm->tm_sec % 10);

 draw_number(19, 51, (atm->tm_mday >= 10) ? atm->tm_mday / 10 : atm->tm_mday % 10);
 draw_number(29, 51, (atm->tm_mday >= 10) ? atm->tm_mday % 10 : -1);
 draw_number(41, 51, ((atm->tm_mon + 1) / 10) % 10);
 draw_number(51, 51, (atm->tm_mon + 1) % 10);
}

void free_stuff()
{
 XUnmapWindow(display, mapwindow);
 XDestroyWindow(display, iconwindow);
 XDestroyWindow(display, window);
 XFreePixmap(display, buffer);
 XFreePixmap(display, iconpixmap);
 XFreePixmap(display, pixmask);
 XFreePixmap(display, pixmap);
 XFreeGC(display, ongc);
 XFreeGC(display, offgc);
 XFreeGC(display, backgc);
 XCloseDisplay(display);
}

void proc()
{
 int i;
 fd_set fs;
 int fd = ConnectionNumber(display);

 process_events();
 FD_ZERO(&fs); FD_SET(fd, &fs);
 i = select(fd + 1, &fs, 0, 0, &tv);
 if(i == -1)
 {
  fprintf(stderr, "Error with select(): %s", strerror(errno));
  exit(1);
 }
 if(!i)
 {
  draw_window();
  update_window();
  tv.tv_sec = opt_milisecs / 1000;
  tv.tv_usec = (opt_milisecs % 1000) * 1000;
 }
}

void switch_ampm(menuitem_t *i)
{
 opt_ampm = i->checked;
 draw_window();
 update_window();
}

int main(int carg, char **varg)
{
 menuitem_t *i;

 argc = carg;
 argv = varg;
 parse_args();
 make_window();

 menu_init(display);
 m = menu_new();

 i = menu_append(m, "12 Hour mode");
 i->i = -1;
 i->checked = opt_ampm;
 i->callback = switch_ampm;

 i = menu_append(m, "Exit");
 i->i = 0;

 while(!exitloop) proc();

 free_stuff();
 return 0;
}
