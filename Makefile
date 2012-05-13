SOFTWARE = procfiled

CCFLAGS = -std=gnu99 -Wall -pedantic -Werror -Wextra -O99 -Os
LDFLAGS = -lnotify # -lefence

OBJDIR = obj
SRCDIR = src
INCDIR = include

OBJFILES = \
$(OBJDIR)/logger.o \
$(OBJDIR)/daemonize.o \
$(OBJDIR)/attribute.o \
$(OBJDIR)/config.o \
$(OBJDIR)/watches.o \
$(OBJDIR)/main.o

all: init $(OBJFILES)
	gcc $(CCFLAGS) -o $(SOFTWARE) $(OBJFILES) -I$(INCDIR) $(LDFLAGS)
	strip --strip-all $(SOFTWARE)
	echo Done.

init:
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	echo Compiling ["$<"]
	gcc $(CCFLAGS) -c -o "$@" "$<" -I$(INCDIR)

$(OBJDIR)/watches.o: $(SRCDIR)/watches.c
	mkdir -p $(OBJDIR)
	echo Compiling ["$<"]
	gcc `pkg-config --cflags gtk+-2.0` $(CCFLAGS) -c -o "$@" "$<" -I$(INCDIR)

clean:
	rm -rf $(OBJDIR)

rebuild: clean all

.PHONY: all init clean rebuild
