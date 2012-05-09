EXEC = wmfs-fifo

PREFIX = /usr

CFLAGS  = -Wall -ansi -pedantic -O2
LDFLAGS = -lX11

.PHONY: all
all: $(EXEC)

.PHONY: clean
clean:
	@rm -vrf $(EXEC)

.PHONY: install
install: all
	@install $(EXEC) $(DESTDIR)$(PREFIX)/bin/$(EXEC)

.PHONY: uninstall
uninstall:
	@rm $(DESTDIR)$(PREFIX)/bin/$(EXEC)

$(EXEC): src/main.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@
