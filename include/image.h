#ifndef MY_IMAGE_LIB
#define MY_IMAGE_LIB
#include <stdlib.h>
#include <stdint.h>

// --- Image Data Structure ---
typedef struct {
    size_t width;
    size_t height;
    size_t channels;
    double* data;
} image_t;

// --- ASCII Grid Structures ---
typedef struct {
    char character;
    uint8_t r;
    uint8_t g;
    uint8_t b;
} ascii_cell_t;

typedef struct {
    size_t width;   // Number of columns (characters)
    size_t height;  // Number of rows (lines)
    ascii_cell_t* cells; // 1D array of size width * height
} ascii_grid_t;

// --- Export Options ---
typedef struct {
    int export_image;       // 1 = Yes, 0 = No
    char* output_path;
    int force_jpg;          // 1 = JPG, 0 = PNG (default)
    char* font_family;      // e.g. "DejaVu Sans Mono"
    int bg_is_white;        // 1 = White, 0 = Black
    
    // Dims control
    int width_chars;        // If > 0, fixed width in chars
    int target_pixel_w;     // If > 0, target image width in pixels (pixel-perfect mode)
    int target_pixel_h;     // If > 0, target image height in pixels
    int scale_factor;       // If > 0, scale factor (1 char per N pixels)
    
    // Processing options
    int use_retro_colors;   // 1 = Retro 3-bit colors, 0 = Truecolor
    
    // Calculated render dimensions (used by export.c)
    int cell_pixel_width;
    int cell_pixel_height;
} export_options_t;


// --- Function Prototypes ---

image_t load_image(const char* file_path);
void free_image(image_t* image);
void free_ascii_grid(ascii_grid_t* grid);

image_t make_resized(image_t* original, size_t max_width, size_t max_height, double character_ratio);

image_t make_grayscale(image_t* original);

double* get_pixel(image_t* image, size_t x, size_t y);
void set_pixel(image_t* image, size_t x, size_t y, const double* new_pixel);

void get_convolution(image_t* image, double* kernel, double* out);
void get_sobel(image_t* image, double* out_x, double* out_y);

#endif