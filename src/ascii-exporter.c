#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <cairo.h>
#include <pango/pangocairo.h>

#define MAX_LINE_LENGTH 8192
#define MAX_LINES 2000

typedef struct {
  int r, g, b;
  char ch;
} Cell;


int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Uso: %s input.txt output.png\n", argv[0]); // argv[0] è il nome dell'eseguibile
    return 1; // errore
  }

  const char *input_file = argv[1];
  const char *output_file = argv[2];

  FILE *f = fopen(input_file, "r");
  if (!f) {
    perror("Errore apertura file");
    return 1;
  }

  regex_t regex;
  regcomp(&regex, "\x1b\\[38;2;([0-9]+);([0-9]+);([0-9]+)m(.)", REG_EXTENDED);

  Cell *matrix[MAX_LINES];
  int cols[MAX_LINES] = {0};
  int rows = 0;

  char line[MAX_LINE_LENGTH];
  while (fgets(line, sizeof(line), f) && rows < MAX_LINES) {
    matrix[rows] = malloc(sizeof(Cell) * MAX_LINE_LENGTH);
    if (!matrix[rows]) {
      fprintf(stderr, "Errore allocazione memoria.\n");
      return 1;
    }

    regmatch_t matches[5];
    int offset = 0;
    int col = 0;

    while (regexec(&regex, line + offset, 5, matches, 0) == 0) {
      char buf[16];

      for (int i = 1; i <= 3; i++) {
        int len = matches[i].rm_eo - matches[i].rm_so;
        strncpy(buf, line + offset + matches[i].rm_so, len);
        buf[len] = '\0';
        if (i == 1) matrix[rows][col].r = atoi(buf);
        if (i == 2) matrix[rows][col].g = atoi(buf);
        if (i == 3) matrix[rows][col].b = atoi(buf);
      }
      matrix[rows][col].ch = line[offset + matches[4].rm_so];
      col++;
      offset += matches[0].rm_eo;
    }

    cols[rows] = col;
    rows++;
  }

  fclose(f);
  regfree(&regex);

  const int font_size = 12;
  const char *font_family = "DejaVu Sans Mono";

  cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, 1000, 1000);
  cairo_t *cr = cairo_create(surface);

  PangoLayout *layout = pango_cairo_create_layout(cr);
  PangoFontDescription *desc = pango_font_description_new();
  pango_font_description_set_family(desc, font_family);
  pango_font_description_set_size(desc, font_size * PANGO_SCALE);
  pango_layout_set_font_description(layout, desc);

  pango_layout_set_text(layout, "A", -1);
  int char_w, char_h;
  pango_layout_get_pixel_size(layout, &char_w, &char_h);
  char_h = (int)(char_h * 1.8);

  int max_cols = 0;
  for (int i = 0; i < rows; i++) {
    if (cols[i] > max_cols)
      max_cols = cols[i];
  }

  cairo_surface_destroy(surface);
  surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, max_cols * char_w, rows * char_h);
  cr = cairo_create(surface);
  layout = pango_cairo_create_layout(cr);
  pango_layout_set_font_description(layout, desc);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_paint(cr);

  for (int y = 0; y < rows; y++) {
    for (int x = 0; x < cols[y]; x++) {
      Cell c = matrix[y][x];
      cairo_set_source_rgb(cr, c.r / 255.0, c.g / 255.0, c.b / 255.0);

      char ch[2] = {c.ch, '\0'};
      pango_layout_set_text(layout, ch, -1);
      cairo_move_to(cr, x * char_w, y * char_h);
      pango_cairo_show_layout(cr, layout);
    }
    free(matrix[y]);
  }

  cairo_surface_flush(surface);
  cairo_surface_write_to_png(surface, output_file);
  printf("Immagine salvata in %s\n", output_file);

  g_object_unref(layout);
  pango_font_description_free(desc);
  cairo_destroy(cr);
  cairo_surface_destroy(surface);

  return 0;
}
