#ifndef PROCESS_H
#define PROCESS_H

#include "image.h"

// Converts an image into an ASCII Grid based on given options.
// Handles resizing, character selection, and color calculation.
// The input image 'original' is not modified, but a resized version is created internally.
ascii_grid_t process_image_to_grid(image_t* original, export_options_t* options);

#endif
