SOURCE_DIR := src
INCLUDE_DIR := inc

SOURCE_FILES := $(wildcard $(SOURCE_DIR)/*.c)
INCLUDE_FILES := $(wildcard $(INCLUDE_DIR)/*.h)

DEPENDENCIES := x11 cairo poppler-glib glib-2.0

CC := gcc
CFLAGS := -g -O0 -Wno-deprecated-declarations -std=gnu99
CPPFLAGS := -I$(INCLUDE_DIR)
LIBS := -lm

define generate-dependencies
	$(eval CPPFLAGS += $(shell pkg-config --exists $1 && pkg-config --cflags $1))
	$(eval LIBS += $(shell pkg-config --exists $1 && pkg-config --libs $1))
endef

$(eval $(foreach dependency,$(DEPENDENCIES),$(call generate-dependencies,$(dependency))))

readerx: $(SOURCE_FILES) $(INCLUDE_FILES)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SOURCE_FILES) $(LIBS) -o $@

install:
	@cp readerx /usr/bin/

clean:
	@rm -f readerx
