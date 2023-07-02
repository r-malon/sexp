.POSIX:

include config.mk

PROG = sexp
SRCS = sexp-basic.c sexp-input.c sexp-main.c sexp-output.c
OBJS = $(SRCS:.c=.o)

all: $(PROG)

sexp-basic.o: sexp.h
sexp-input.o: sexp.h
sexp-main.o: sexp.h
sexp-output.o: sexp.h

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

$(PROG): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(MANPREFIX)/man$(MANSECTION)
	install -m 755 $(PROG) $(DESTDIR)$(PREFIX)/bin/$(PROG)
	install -m 644 $(PROG).$(MANSECTION) \
		$(DESTDIR)$(MANPREFIX)/man$(MANSECTION)/$(PROG).$(MANSECTION)

uninstall:
	rm $(DESTDIR)$(PREFIX)/bin/$(PROG)
	rm $(DESTDIR)$(MANPREFIX)/man$(MANSECTION)/$(PROG).$(MANSECTION)

clean:
	-rm -f $(OBJS) $(PROG)

.PHONY: all clean install uninstall
