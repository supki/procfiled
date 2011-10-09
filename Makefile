SOFTWARE = mtd

CCFLAGS = -std=gnu99 -Wall -Werror -Wextra -O99 -Os

OBJDIR = obj
SRCDIR = src
INCDIR = include

OBJFILES = \
$(OBJDIR)/path.o \
$(OBJDIR)/logger.o \
$(OBJDIR)/daemonize.o \
$(OBJDIR)/attribute.o \
$(OBJDIR)/command.o \
$(OBJDIR)/config.o \
$(OBJDIR)/watches.o \
$(OBJDIR)/main.o \

all: $(OBJFILES)
	gcc $(CCFLAGS) -o $(SOFTWARE) $(OBJFILES) -I$(INCDIR) # -lefence
	strip --strip-all $(SOFTWARE)
	echo Done.

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(OBJDIR)
	echo Compiling ["$<"]
	gcc $(CCFLAGS) -c -o "$@" "$<" -I$(INCDIR)

clean:
	rm -rf $(OBJDIR)

rebuild: clean all

.PHONY: all clean rebuild
