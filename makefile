# =============================================================================
# General Settings
# =============================================================================
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -Iinclude -D_GNU_SOURCE

# Use pkg-config to get compiler/linker flags for libraries
PANGO_CAIRO_CFLAGS = $(shell pkg-config --cflags pangocairo)
PANGO_CAIRO_LIBS = $(shell pkg-config --libs pangocairo)
LDFLAGS = -lm

# =============================================================================
# Targets
# =============================================================================

# The default target, executed when you just run `make`
all: ascii-view

# Main program: image to ascii art for terminal
ASCII_VIEW_SRCS = src/main.c src/argparse.c src/image.c src/print_image.c src/export.c src/process.c
ASCII_VIEW_OBJS = $(ASCII_VIEW_SRCS:.c=.o)

ascii-view: $(ASCII_VIEW_OBJS)
	$(CC) $(CFLAGS) $(PANGO_CAIRO_CFLAGS) $(ASCII_VIEW_OBJS) -o $@ $(LDFLAGS) $(PANGO_CAIRO_LIBS)

# Generic rule to compile .c files into .o object files
%.o: %.c
	$(CC) $(CFLAGS) $(PANGO_CAIRO_CFLAGS) -c $< -o $@

# Release build for the main ascii-view program
release: CFLAGS += -O3 -flto -march=native
release: LDFLAGS += -flto
release: clean all

# Clean up object files and executables
clean:
	rm -f src/*.o ascii-view ascii-to-image ascii-exporter

.PHONY: all clean release
