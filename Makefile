# Makefile
#
# Copyright (c) 2011 Patrick "P. J." McDermott
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

SRCDIR = .
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

SHELL = /bin/sh
CC = gcc
INSTALL = /usr/bin/install -c

CFLAGS = -Wall $(CMACROS)
LDFLAGS = 
CDEBUG = -g -DDEBUG
OPTNONE = -O0
OPTFULL = -O2

.SUFFIXES:
.SUFFIXES: .c .o

SRCS = $(SRCDIR)/src/controllers.c $(SRCDIR)/src/text-input.c \
       $(SRCDIR)/src/views.c $(SRCDIR)/src/tabs.c $(SRCDIR)/src/windows.c \
       $(SRCDIR)/src/views/menu.c $(SRCDIR)/src/views/contacts.c \
       $(SRCDIR)/src/views/edit-contact.c $(SRCDIR)/src/views/chat.c \
       $(SRCDIR)/src/views/log.c \
       $(SRCDIR)/src/chat.c \
       $(SRCDIR)/src/chat_packet_message.c \
       $(SRCDIR)/src/contact.c $(SRCDIR)/src/chat_event.c \
       $(SRCDIR)/src/main.c
OBJS = $(SRCS:.c=.o)
LIBS = -lcurses
AUX = COPYING

# `all' target
# Compiles with optimizations.
.PHONY: all
all: CFLAGS += $(OPTFULL)
all: cs490

# `debug' target
# Compiles with debugging information and no optimizations.
.PHONY: debug
debug: CFLAGS += $(CDEBUG) $(OPTNONE)
debug: cs490

# `profile' target
# Compiles with debugging information and optimizations.
.PHONY: profile
profile: CFLAGS += $(CDEBUG) $(OPTFULL)
profile: cs490

cs490: $(OBJS)
	$(CC) $(LDFLAGS) -o"$@" $(LIBS) $(OBJS)

.PHONY: clean
clean:
	rm -f $(OBJS) cs490

.PHONY: install
install: all
	$(INSTALL) cs490 $(BINDIR)/cs490
