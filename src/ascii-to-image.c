
// ascii_to_image.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <cairo.h>
#include <pango/pangocairo.h>

#define MAX_LINE_LEN 16384
#define INITIAL_COLS 256

typedef struct {
    int r,g,b;
    char ch;
} Cell;

static int parse_ansi_line(const char *line, Cell **out_cells) {
    // alloca iniziale (verrà riallocata se serve)
    int capacity = INITIAL_COLS;
    Cell *cells = malloc(sizeof(Cell) * capacity);
    if (!cells) return -1;
    int count = 0;

    const unsigned char *p = (const unsigned char*)line;
    while (*p) {
        if (*p == 0x1b) { // ESC
            const unsigned char *q = p + 1;
            if (*q == '[') q++;
            else { p++; continue; }

            // Expect "38;2;"
            if (strncmp((const char*)q, "38;2;", 5) == 0) {
                q += 5;
                // parse three ints separated by ';' and ending with 'm'
                int r=0,g=0,b=0;
                int n = 0;
                // use sscanf on the substring; but need to ensure format fits
                // we'll try to parse "<r>;<g>;<b>m"
                if (sscanf((const char*)q, "%d;%d;%d%n", &r, &g, &b, &n) == 3) {
                    // q + n should now be at 'm' or position before 'm'
                    const unsigned char *after_nums = q + n;
                    if (*after_nums == 'm') {
                        // character right after 'm'
                        const unsigned char *chptr = after_nums + 1;
                        if (*chptr == '\0' || *chptr == '\n' || *chptr == '\r') {
                            // nothing usable, skip
                            p = after_nums + 1;
                            continue;
                        }
                        // store
                        if (count >= capacity) {
                            capacity *= 2;
                            Cell *tmp = realloc(cells, sizeof(Cell) * capacity);
                            if (!tmp) { free(cells); return -1; }
                            cells = tmp;
                        }
                        cells[count].r = r;
                        cells[count].g = g;
                        cells[count].b = b;
                        cells[count].ch = (char)*chptr;
                        count++;
                        // advance p past the character we consumed
                        p = chptr + 1;
                        continue;
                    }
                }
            }
        }
        // if not an ANSI color seq we care about, skip single byte
        p++;
    }

    // shrink to fit
    if (count == 0) {
        free(cells);
        *out_cells = NULL;
        return 0;
    }
    Cell *sh = realloc(cells, sizeof(Cell) * count);
    if (sh) cells = sh;
    *out_cells = cells;
    return count;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s input.txt output.png [-v]\n", argv[0]);
        return 1;
    }
    const char *input = argv[1];
    const char *output = argv[2];
    int verbose = 0;
    if (argc >= 4 && strcmp(argv[3], "-v") == 0) verbose = 1;

    FILE *f = fopen(input, "r");
    if (!f) {
        perror("Impossibile aprire file input");
        return 1;
    }

    // legge tutte le linee e le parsing in memoria
    char buffer[MAX_LINE_LEN];
    Cell **rows = NULL;
    int *cols = NULL;
    int row_count = 0;
    int max_cols = 0;
    while (fgets(buffer, sizeof(buffer), f)) {
        Cell *cells = NULL;
        int n = parse_ansi_line(buffer, &cells);
        if (n < 0) {
            fprintf(stderr, "Errore memoria durante parsing\n");
            fclose(f);
            return 1;
        }
        // memorizza riga (anche se n==0, memorizziamo NULL)
        rows = realloc(rows, sizeof(Cell*) * (row_count + 1));
        cols = realloc(cols, sizeof(int) * (row_count + 1));
        rows[row_count] = cells;
        cols[row_count] = n;
        if (n > max_cols) max_cols = n;
        row_count++;
    }
    fclose(f);

    if (row_count == 0) {
        fprintf(stderr, "File vuoto o non letto.\n");
        return 1;
    }
    if (max_cols == 0) {
        fprintf(stderr, "Nessuna sequenza ANSI trovata (max_cols == 0). Controlla che il file contenga sequenze \\x1b[38;2;R;G;BmCHAR\n");
        // libera righe
        for (int i=0;i<row_count;i++) free(rows[i]);
        free(rows); free(cols);
        return 1;
    }

    if (verbose) {
        fprintf(stderr, "Righe: %d, Max colonne: %d\n", row_count, max_cols);
    }

    // Init Cairo + Pango per misura
    int font_size = 12;
    double vertical_scale = 1.2; // il tuo fattore empirico
    const char *font_family = "DejaVu Sans Mono";

    // superficie temporanea per misure
    cairo_surface_t *tmp_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, 10, 10);
    cairo_t *cr = cairo_create(tmp_surface);
    PangoLayout *layout = pango_cairo_create_layout(cr);
    PangoFontDescription *desc = pango_font_description_new();
    pango_font_description_set_family(desc, font_family);
    pango_font_description_set_size(desc, font_size * PANGO_SCALE);
    pango_layout_set_font_description(layout, desc);

    // misura un singolo carattere
    pango_layout_set_text(layout, "A", -1);
    int char_w, char_h;
    pango_layout_get_pixel_size(layout, &char_w, &char_h);
    if (char_w <= 0 || char_h <= 0) {
        fprintf(stderr, "Impossibile misurare il font. char_w=%d char_h=%d\n", char_w, char_h);
        // cleanup
        g_object_unref(layout);
        pango_font_description_free(desc);
        cairo_destroy(cr);
        cairo_surface_destroy(tmp_surface);
        for (int i=0;i<row_count;i++) free(rows[i]);
        free(rows); free(cols);
        return 1;
    }
    // applica fattore verticale empirico
    char_h = (int)(char_h * vertical_scale);
    if (verbose) fprintf(stderr, "char_w=%d char_h=%d (scale=%.2f)\n", char_w, char_h, vertical_scale);

    // cleanup temporanea
    g_object_unref(layout);
    pango_font_description_free(desc);
    cairo_destroy(cr);
    cairo_surface_destroy(tmp_surface);

    // crea superficie definitiva
    int width = max_cols * char_w;
    int height = row_count * char_h;
    if (width <= 0 || height <= 0) {
        fprintf(stderr, "Dimensioni immagine non valide: %d x %d\n", width, height);
        for (int i=0;i<row_count;i++) free(rows[i]);
        free(rows); free(cols);
        return 1;
    }

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
    cairo_status_t s = cairo_surface_status(surface);
    if (s != CAIRO_STATUS_SUCCESS) {
        fprintf(stderr, "Errore creazione superficie Cairo: %s\n", cairo_status_to_string(s));
        for (int i=0;i<row_count;i++) free(rows[i]);
        free(rows); free(cols);
        return 1;
    }
    cr = cairo_create(surface);

    // setup pango/layout per la nuova surface
    layout = pango_cairo_create_layout(cr);
    desc = pango_font_description_new();
    pango_font_description_set_family(desc, font_family);
    pango_font_description_set_size(desc, font_size * PANGO_SCALE);
    pango_layout_set_font_description(layout, desc);

    // riempi sfondo di nero
    cairo_set_source_rgb(cr, 0,0,0);
    cairo_paint(cr);

    // disegna caratteri
    for (int y=0; y<row_count; y++) {
        for (int x=0; x<cols[y]; x++) {
            Cell c = rows[y][x];
            double rf = c.r / 255.0;
            double gf = c.g / 255.0;
            double bf = c.b / 255.0;
            cairo_set_source_rgb(cr, rf, gf, bf);

            char chstr[2] = { c.ch, '\0' };
            pango_layout_set_text(layout, chstr, -1);
            cairo_move_to(cr, x * char_w, y * char_h);
            pango_cairo_show_layout(cr, layout);
        }
    }

    // flush & save
    cairo_surface_flush(surface);
    cairo_status_t wr = cairo_surface_write_to_png(surface, output);
    if (wr != CAIRO_STATUS_SUCCESS) {
        fprintf(stderr, "Errore scrittura PNG: %s\n", cairo_status_to_string(wr));
        // cleanup
        g_object_unref(layout);
        pango_font_description_free(desc);
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        for (int i=0;i<row_count;i++) free(rows[i]);
        free(rows); free(cols);
        return 1;
    }

    if (verbose) fprintf(stderr, "✅ Immagine salvata in: %s\n", output);
    else printf("Immagine salvata in: %s\n", output);

    // cleanup finale
    g_object_unref(layout);
    pango_font_description_free(desc);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    for (int i=0;i<row_count;i++) free(rows[i]);
    free(rows); free(cols);

    return 0;
}
