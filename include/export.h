#ifndef EXPORT_H
#define EXPORT_H

#include "image.h"

// Export the ASCII grid to an image file (PNG/JPG) based on options
void export_ascii_to_image(ascii_grid_t* grid, export_options_t* options);

#endif
