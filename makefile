# vim: tabstop=8:shiftwidth=8:noexpandtab:tw=80

TARGET = mutil

# FIXME: Reorder these.
C_SOURCE_FILES = \
	mutil_common.c \
	mutil_album.c \
	mutil_audio_file.c \
	mutil_main.c \
	mutil_makefile.c \
	mutil_tag.c \
	mutil_track.c \
	mutil_xml.c

.PHONY: all
.PHONY: clean

CFLAGS=
CFLAGS+=-Wall -Werror -Wno-unused-function -D_GNU_SOURCE
LDFLAGS=

CFLAGS=
CFLAGS+=-Wall -Werror -Wno-unused-function -D_GNU_SOURCE
CFLAGS+=`pkg-config --cflags glib-2.0`
CFLAGS+=`pkg-config --cflags libxml-2.0`

LDFLAGS+=`pkg-config --libs glib-2.0`
LDFLAGS+=`pkg-config --libs libxml-2.0`
LDFLAGS+=-lFLAC -lm

ifdef MUTIL_DEBUG
CFLAGS+=-g3 -O0
else
CFLAGS+=-O3 -DNDEBUG -DG_DISABLE_ASSERT
endif

ifdef MUTIL_PROFILE
LDFLAGS+=-pg
endif

TAG_FILE=tags

O_SUFFIX = .o
DEP_SUFFIX = .dep

DEP_FILES = $(addsuffix $(DEP_SUFFIX), $(C_SOURCE_FILES))
O_FILES = $(addsuffix $(O_SUFFIX), $(C_SOURCE_FILES))

all: $(TARGET)

-include $(DEP_FILES)

clean:
	@echo Cleaning intermediate and target files
	@rm -f $(DEP_FILES)
	@rm -f $(O_FILES)
	@rm -f $(TARGET)

$(TAG_FILE): $(C_SOURCE_FILES)
	@echo Generating tags
	@ctags -R	

$(TARGET): $(O_FILES)
	@echo Linking $@
	@gcc -o$(TARGET) $(O_FILES) $(LDFLAGS)

$(O_FILES): %$(O_SUFFIX) : %$(DEP_SUFFIX)
	@echo Compiling $@
	@gcc -o$@ -c $(basename $@) $(CFLAGS)

$(DEP_FILES):
	@echo Generating dependencies for $@ 
	@gcc -MM -MT $(addsuffix $(O_SUFFIX), $(basename $@)) -MF $@ $(basename $@) $(CFLAGS)

