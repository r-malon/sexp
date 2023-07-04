VERSION = 0.0.1

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man
MANSECTION = 1

CC = cc
CFLAGS = -std=c89 -Wall -Wextra -pedantic -O2 \
	$(shell pkg-config --cflags libbsd-overlay) # GNU extension
CPPFLAGS = 
LDFLAGS = -s $(shell pkg-config --libs libbsd-overlay)
