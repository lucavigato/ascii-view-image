#include <stdio.h>
#include "../include/print_image.h"
#include "../include/image.h"

#define RESET "\x1b[0m"

void print_image(ascii_grid_t* grid) {
    if (!grid || !grid->cells) return;

    for (size_t y = 0; y < grid->height; y++) {
        for (size_t x = 0; x < grid->width; x++) {
            ascii_cell_t* cell = &grid->cells[y * grid->width + x];
            
            // Print using TrueColor ANSI
            printf("\x1b[38;2;%d;%d;%dm%c", cell->r, cell->g, cell->b, cell->character);
        }
        printf("%s\n", RESET);
    }
}