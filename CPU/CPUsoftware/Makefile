# define application
APP      = mecontrol

# definitions
SRCEXT   = cpp
SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin

# paths
DIR     := ${CURDIR}
SRCS    := $(shell find $(SRCDIR) -name '*.$(SRCEXT)')
SRCDIRS := $(shell find . -name '*.$(SRCEXT)' -exec dirname {} \; | uniq)
OBJS    := $(patsubst %.$(SRCEXT),$(OBJDIR)/%.o,$(SRCS))

# flags
DEBUG    = -g
INCLUDES = -I./inc -I./src/instrument -I./src/subsystems -I./src/operations -I./src/tools -I./src/data_format
CFLAGS   = -std=c++11 -Wall -pedantic -c -O3 $(INCLUDES) $(DEBUG)

platform = $(shell uname -s)

ifeq ($(platform), Darwin)
LDFLAGS  = -L./lib -lm -lpthread -lusb-1.0
else
OS = $(shell lsb_release -si)
LDFLAGS  = -L./lib -lrtd-dm75xx -lrtd-aDIO -lm -lpthread -lusb-1.0
endif

# compiler
CXX = g++

ifeq ($(SRCEXT), cpp)
CC       = $(CXX)
endif

.PHONY: all clean distclean


all: $(BINDIR)/$(APP)

# target
$(BINDIR)/$(APP): buildrepo $(OBJS)
	@mkdir -p `dirname $@`
	@echo "Linking $@..."
	@$(CC) $(OBJS) $(LDFLAGS) -o $@
	./symlink.sh

# objects
$(OBJDIR)/%.o: %.$(SRCEXT) %.h
	@echo "Generating dependencies for $<..."
	@$(call make-depend,$<,$@,$(subst .o,.d,$@))
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) -r $(OBJDIR)

distclean: clean
	$(RM) -r $(BINDIR)

buildrepo:
	@$(call make-repo)

define make-repo
   for dir in $(SRCDIRS); \
   do \
	mkdir -p $(OBJDIR)/$$dir; \
   done
endef


# usage: $(call make-depend, source-file, object-file, depend-file)
define make-depend
  $(CC) -MM       \
        -MF $3    \
        -MP       \
        -MT $2    \
        $(CFLAGS) \
        $1
endef

