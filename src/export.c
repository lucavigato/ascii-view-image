#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cairo.h>
#include <pango/pangocairo.h>
#include "export.h"
#include "image.h"

void export_ascii_to_image(ascii_grid_t* grid, export_options_t* options) {
    if (!grid || !options || !options->output_path) return;

    printf("Preparazione export immagine: %s\n", options->output_path);

    // --- Configurazione Font & Dimensioni Cella ---
    cairo_surface_t* temp_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, 10, 10);
    cairo_t* temp_cr = cairo_create(temp_surface);

    PangoLayout* layout = pango_cairo_create_layout(temp_cr);
    PangoFontDescription* desc = pango_font_description_new();
    
    const char* font_family = options->font_family ? options->font_family : "DejaVu Sans Mono";
    pango_font_description_set_family(desc, font_family);
    
    // Use calculated cell dimensions from process.c
    double cell_w = (double)options->cell_pixel_width;
    double cell_h = (double)options->cell_pixel_height;
    
    // Safety check
    if (cell_w < 1.0) cell_w = 1.0;
    if (cell_h < 1.0) cell_h = 1.0;

    // Calculate font size to fit height
    // Pango size is in Pango units (1/1024 of a point)
    // We want pixel size. Roughly: PixelSize = PointSize * (DPI/72)
    // Pango assumes 96 DPI usually.
    // But let's try setting absolute size if possible, or estimate points.
    // Height in pixels = Points * (96/72) * (PangoScale/1024)? No.
    // Simplest: pango_font_description_set_absolute_size sets size in Pango units (scaled).
    // If we want 'cell_h' pixels, we set size to cell_h * PANGO_SCALE.
    pango_font_description_set_absolute_size(desc, cell_h * PANGO_SCALE);
    
    pango_layout_set_font_description(layout, desc);
    
    // Cleanup temp
    g_object_unref(layout);
    cairo_destroy(temp_cr);
    cairo_surface_destroy(temp_surface);


    // --- Creazione Superficie Reale ---
    int img_w = (int)(grid->width * cell_w);
    int img_h = (int)(grid->height * cell_h);
    
    // Force exact dimensions if target is set (dims mode)
    if (options->target_pixel_w > 0 && options->target_pixel_h > 0) {
        img_w = options->target_pixel_w;
        img_h = options->target_pixel_h;
    }

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, img_w, img_h);
    cairo_t* cr = cairo_create(surface);

    // Sfondo
    if (options->bg_is_white) {
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    } else {
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    }
    cairo_paint(cr);

    // --- Disegno Griglia ---
    layout = pango_cairo_create_layout(cr);
    // Re-apply font settings to this context's layout
    pango_layout_set_font_description(layout, desc);

    for (size_t y = 0; y < grid->height; y++) {
        for (size_t x = 0; x < grid->width; x++) {
            ascii_cell_t cell = grid->cells[y * grid->width + x];
            
            double r = cell.r / 255.0;
            double g = cell.g / 255.0;
            double b = cell.b / 255.0;
            cairo_set_source_rgb(cr, r, g, b);

            char str[2] = {cell.character, '\0'};
            pango_layout_set_text(layout, str, -1);

            // Posizionamento (Centrato nella cella se possibile)
            int char_pixel_w, char_pixel_h;
            pango_layout_get_pixel_size(layout, &char_pixel_w, &char_pixel_h);
            
            double pos_x = x * cell_w + (cell_w - char_pixel_w) / 2.0;
            double pos_y = y * cell_h + (cell_h - char_pixel_h) / 2.0;
            
            cairo_move_to(cr, pos_x, pos_y);
            pango_cairo_show_layout(cr, layout);
        }
    }

    cairo_surface_flush(surface);
    cairo_surface_write_to_png(surface, options->output_path);
    printf("Immagine salvata correttamente: %s\n", options->output_path);

    g_object_unref(layout);
    pango_font_description_free(desc);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}
