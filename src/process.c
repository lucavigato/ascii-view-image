#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "../include/process.h"
#include "../include/image.h"

// --- Constants & Helpers ---
#define VALUE_CHARS " .-=+*x#$&X@"
#define N_VALUES (sizeof(VALUE_CHARS) - 1) 
#define DEFAULT_EDGE_THRESHOLD 4.0
#define DEFAULT_CHAR_RATIO 2.0

// HSV Helpers (omitted for brevity, same as before but I need to include them for compilation)
typedef struct { double hue; double saturation; double value; } hsv_t;

static double* get_max(double* a, double* b, double* c) { if ((*a >= *b) && (*a >= *c)) return a; else if (*b >= *c) return b; else return c; }
static double* get_min(double* a, double* b, double* c) { if ((*a <= *b) && (*a <= *c)) return a; else if (*b <= *c) return b; else return c; }

static hsv_t rgb_to_hsv(double red, double green, double blue) {
    hsv_t hsv;
    double* max = get_max(&red, &green, &blue);
    double* min = get_min(&red, &green, &blue);
    hsv.value = *max;
    double chroma = hsv.value - *min;
    if (fabs(hsv.value) < 1e-4) hsv.saturation = 0.0; else hsv.saturation = chroma / hsv.value;
    if (chroma < 1e-4) hsv.hue = 0.0;
    else if (max == &red) { hsv.hue = 60.0 * fmod((green - blue) / chroma, 6.0); if (hsv.hue < 0.0) hsv.hue += 360.0; }
    else if (max == &green) { hsv.hue = 60.0 * (2.0 + (blue - red) / chroma); }
    else { hsv.hue = 60.0 * (4.0 + (red - green) / chroma); }
    return hsv;
}

static void hsv_to_rgb(const hsv_t* hsv, double* r, double* g, double* b) {
    double c = hsv->value * hsv->saturation;
    double h_prime = hsv->hue / 60.0;
    double x = c * (1.0 - fabs(fmod(h_prime, 2.0) - 1.0));
    double r1, g1, b1;
    if (h_prime >= 0.0 && h_prime < 1.0) { r1 = c; g1 = x; b1 = 0.0; }
    else if (h_prime >= 1.0 && h_prime < 2.0) { r1 = x; g1 = c; b1 = 0.0; }
    else if (h_prime >= 2.0 && h_prime < 3.0) { r1 = 0.0; g1 = c; b1 = x; }
    else if (h_prime >= 3.0 && h_prime < 4.0) { r1 = 0.0; g1 = x; b1 = c; }
    else if (h_prime >= 4.0 && h_prime < 5.0) { r1 = x; g1 = 0.0; b1 = c; }
    else { r1 = c; g1 = 0.0; b1 = x; }
    double m = hsv->value - c; *r = r1 + m; *g = g1 + m; *b = b1 + m;
}

static void get_retro_rgb(const hsv_t* hsv, double* r, double* g, double* b) {
    hsv_t quantized_hsv = *hsv;
    quantized_hsv.value = 1.0;
    quantized_hsv.hue = round(quantized_hsv.hue / 60.0) * 60.0;
    if (quantized_hsv.hue >= 360.0) quantized_hsv.hue = 0.0;
    quantized_hsv.saturation = (quantized_hsv.saturation < 0.25) ? 0.0 : 1.0;
    hsv_to_rgb(&quantized_hsv, r, g, b);
}

static double calculate_grayscale_from_hsv(const hsv_t* hsv) { return hsv->value * hsv->value; }
static char get_ascii_char(double grayscale) { size_t index = (size_t) (grayscale * N_VALUES); if (index >= N_VALUES) index = N_VALUES - 1; return VALUE_CHARS[index]; }
static char get_sobel_angle_char(double sobel_angle) {
    if ((22.5 <= sobel_angle && sobel_angle <= 67.5) || (-157.5 <= sobel_angle && sobel_angle <= -112.5)) return '\\';
    else if ((67.5 <= sobel_angle && sobel_angle <= 112.5) || (-112.5 <= sobel_angle && sobel_angle <= -67.5)) return '_';
    else if ((112.5 <= sobel_angle && sobel_angle <= 157.5) || (-67.5 <= sobel_angle && sobel_angle <= -22.5)) return '/';
    else return '|';
}

// --- Main Processing Function ---

ascii_grid_t process_image_to_grid(image_t* original, export_options_t* options) {
    ascii_grid_t grid = {0};
    if (!original || !original->data) return grid;

    // Init calculated cell pixel dimensions
    options->cell_pixel_width = 0;
    options->cell_pixel_height = 0;

    size_t target_cols = 0;
    size_t target_rows = 0;
    double char_ratio = DEFAULT_CHAR_RATIO; 

    // 1. Determine Grid Dimensions & Render Cell Size
    if (options->scale_factor > 0) {
        // Scale Mode: "Square Pixel Replacement"
        // One character replaces an N x N block of pixels.
        // The character itself is rendered into an N x N cell.
        
        // Dimensions of the Grid (Downsampled)
        target_cols = original->width / options->scale_factor;
        target_rows = original->height / options->scale_factor;
        
        // Force char_ratio to 1.0 because we are sampling SQUARE blocks (scale x scale)
        char_ratio = 1.0; 
        
        // Render Dimensions (Size of 1 char in final image)
        options->cell_pixel_width = options->scale_factor;
        options->cell_pixel_height = options->scale_factor;
        
        if (target_cols < 1) target_cols = 1;
        if (target_rows < 1) target_rows = 1;
    } 
    else if (options->target_pixel_w > 0 && options->target_pixel_h > 0) {
        // Dims Mode: Pixel Perfect Target
        // We assume square cells to fill the target resolution.
        size_t w = (options->width_chars > 0) ? options->width_chars : 160;
        target_cols = w;
        double img_ar = (double)options->target_pixel_w / (double)options->target_pixel_h;
        target_rows = (size_t)(target_cols / img_ar);
        char_ratio = 1.0;

        options->cell_pixel_width = options->target_pixel_w / target_cols;
        options->cell_pixel_height = options->target_pixel_h / target_rows;
        
        if(options->cell_pixel_width < 1) options->cell_pixel_width = 1;
        if(options->cell_pixel_height < 1) options->cell_pixel_height = 1;
    } 
    else {
        // Standard Terminal Mode (or Export with Default Font)
        target_cols = (options->width_chars > 0) ? options->width_chars : 80;
        char_ratio = DEFAULT_CHAR_RATIO;
        target_rows = (original->height * target_cols) / (char_ratio * original->width);
        
        // Use estimate 8x16 for default render if not specified
        options->cell_pixel_width = 8;
        options->cell_pixel_height = 16;
    }
    
    if (target_rows < 1) target_rows = 1;

    // 2. Resize Image
    image_t resized = make_resized(original, target_cols, target_rows, char_ratio);
    
    grid.width = resized.width;
    grid.height = resized.height;
    grid.cells = malloc(sizeof(ascii_cell_t) * grid.width * grid.height);

    // 3. Edge Detection
    image_t grayscale = make_grayscale(&resized);
    double* sobel_x = calloc(grayscale.width * grayscale.height, sizeof(*sobel_x));
    double* sobel_y = calloc(grayscale.width * grayscale.height, sizeof(*sobel_y));
    double edge_threshold = DEFAULT_EDGE_THRESHOLD; 
    get_sobel(&grayscale, sobel_x, sobel_y);

    // 4. Fill Grid
    for (size_t y = 0; y < grid.height; y++) {
        for (size_t x = 0; x < grid.width; x++) {
            size_t idx = y * grid.width + x;
            ascii_cell_t* cell = &grid.cells[idx];
            double* pixel = get_pixel(&resized, x, y);
            
            double r_d, g_d, b_d;
            double val_grayscale;
            
            if (resized.channels <= 2) {
                 val_grayscale = pixel[0];
                 r_d = g_d = b_d = pixel[0];
            } else {
                hsv_t hsv = rgb_to_hsv(pixel[0], pixel[1], pixel[2]);
                val_grayscale = calculate_grayscale_from_hsv(&hsv);
                if (options->use_retro_colors) get_retro_rgb(&hsv, &r_d, &g_d, &b_d);
                else { hsv.value = 1.0; hsv_to_rgb(&hsv, &r_d, &g_d, &b_d); }
            }
            
            cell->r = (uint8_t)(r_d * 255); cell->g = (uint8_t)(g_d * 255); cell->b = (uint8_t)(b_d * 255);
            cell->character = get_ascii_char(val_grayscale);

            size_t sobel_idx = y * grayscale.width + x; 
            if ((sobel_x[sobel_idx]*sobel_x[sobel_idx] + sobel_y[sobel_idx]*sobel_y[sobel_idx]) >= edge_threshold * edge_threshold) {
                cell->character = get_sobel_angle_char(atan2(sobel_y[sobel_idx], sobel_x[sobel_idx]) * 180. / M_PI);
            }
        }
    }

    free(sobel_x); free(sobel_y); free_image(&grayscale); free_image(&resized);
    return grid;
}