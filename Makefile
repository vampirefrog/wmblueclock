# Makefile

# Installation directory
PREFIX=/usr/local

# USE Xft for the menu
USE_XFT=yes

VERSION=0.5

BINDIR=$(PREFIX)/bin
MANPREFIX=$(PREFIX)/share/man
MANDIR=$(MANPREFIX)/man1
PROG=wmblueclock
MANUAL=$(PROG).1
OBJS=main.o menu.o
LIBS=-L/usr/X11R6/lib -lX11 -lXext -lXpm
CFLAGS=-O2 -Wall -DVERSION=\"$(VERSION)\"

CC=gcc
RM=rm -rf
INST=install
MKDIR_P=mkdir -p

ifeq ($(USE_XFT),yes)
CFLAGS += -DUSE_XFT $(shell pkg-config xft --cflags)
LIBS += $(shell pkg-config xft --libs)
endif

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(OBJS) $(LIBS)
	strip $(PROG)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	$(RM) *.o $(PROG) *~ *.bak *.BAK .xvpics
install: $(PROG) $(MANUAL)
	$(INST) -m 755 $(PROG) $(BINDIR)
	$(MKDIR_P) $(MANDIR)
	$(INST) -m 644 $(MANUAL) $(MANDIR)
uninstall:
	$(RM) $(BINDIR)/$(PROG)
	$(RM) $(MANDIR)/$(MANUAL)

main.o: main.c mwm.h menu.h pixmap.xpm icon.xpm common.c
menu.o: menu.c menu.h
