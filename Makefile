SOFTWARE = mtd

CCFLAGS = -std=c99 -Wall -Werror -Wextra -O99 -Os

OBJDIR = obj
SRCDIR = src
INCDIR = include

OBJFILES = \
$(OBJDIR)/daemonize.o \
$(OBJDIR)/path.o \
$(OBJDIR)/config.o \
$(OBJDIR)/watch.o \
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
