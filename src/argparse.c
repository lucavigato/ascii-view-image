#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include "../include/argparse.h"

// Defaults
#define DEFAULT_MAX_WIDTH 80
#define DEFAULT_FONT "DejaVu Sans Mono"

void print_help(char* exec_alias) {
    printf("USAGE:\n");
    printf("\t%s <path/to/image> [OPTIONS]\n\n", exec_alias);

    printf("GENERAL OPTIONS:\n");
    printf("\t--width, -w <n>\t\tSet width in characters (overrides terminal width)\n");
    printf("\t--scale, -s <n>\t\tScale factor (1 char = n pixels). Good for keeping resolution.\n");
    printf("\t--dims <WxH>\t\tTarget output resolution in pixels (e.g. 1920x1080). Forces square cells.\n");
    
    printf("\nEXPORT OPTIONS:\n");
    printf("\t--export, -e\t\tSave output to image file instead of printing to terminal\n");
    printf("\t--output, -o <file>\tSpecify output filename. Default: input_name.png\n");
    printf("\t--jpg, -j\t\tUse JPG extension/format for default filename\n");
    printf("\t--font <name>\t\tFont family for export (default: %s)\n", DEFAULT_FONT);
    printf("\t--bg-white\t\tUse white background (default: black)\n");
    printf("\t--retro-colors\t\tUse 3-bit retro color palette (8 colors)\n");
}

// Helper: Get terminal size
int try_get_terminal_size(int* width, int* height) {
#ifdef _WIN32
    if (!_isatty(0)) return 0;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) return 0;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return 0;
    *width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    *height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
    if (!isatty(0)) return 0;
    struct winsize ws;
    if (ioctl(0, TIOCGWINSZ, &ws) == 0) {
        *width = ws.ws_col;
        *height = ws.ws_row;
        return 1;
    }
#endif
    return 0;
}

struct arguments parse_args(int argc, char *argv[]) {
    struct arguments args;
    // Init defaults
    args.filename = NULL;
    args.width = 0; // 0 means auto/terminal
    
    // Init export options defaults
    args.options.export_image = 0;
    args.options.output_path = NULL;
    args.options.force_jpg = 0;
    args.options.font_family = NULL; // Will default later if needed
    args.options.bg_is_white = 0;
    args.options.width_chars = 0;
    args.options.target_pixel_w = 0;
    args.options.target_pixel_h = 0;
    args.options.scale_factor = 0;
    args.options.use_retro_colors = 0;

    if (argc < 2) {
        print_help(argv[0]);
        return args;
    }

    args.filename = argv[1];
    if (strcmp(args.filename, "-h") == 0 || strcmp(args.filename, "--help") == 0) {
        print_help(argv[0]);
        args.filename = NULL; // Signal invalid
        return args;
    }

    for (int i = 2; i < argc; i++) {
        // Width
        if ((strcmp(argv[i], "--width") == 0 || strcmp(argv[i], "-w") == 0) && i + 1 < argc) {
            args.width = atoi(argv[++i]);
            args.options.width_chars = args.width;
        }
        // Export
        else if (strcmp(argv[i], "--export") == 0 || strcmp(argv[i], "-e") == 0) {
            args.options.export_image = 1;
        }
        // Output file
        else if ((strcmp(argv[i], "--output") == 0 || strcmp(argv[i], "-o") == 0) && i + 1 < argc) {
            args.options.output_path = strdup(argv[++i]);
            args.options.export_image = 1; // Implied
        }
        // JPG
        else if (strcmp(argv[i], "--jpg") == 0 || strcmp(argv[i], "-j") == 0) {
            args.options.force_jpg = 1;
        }
        // Font
        else if (strcmp(argv[i], "--font") == 0 && i + 1 < argc) {
            args.options.font_family = strdup(argv[++i]);
        }
        // Background
        else if (strcmp(argv[i], "--bg-white") == 0) {
            args.options.bg_is_white = 1;
        }
        // Retro Colors
        else if (strcmp(argv[i], "--retro-colors") == 0) {
            args.options.use_retro_colors = 1;
        }
        // Scale
        else if ((strcmp(argv[i], "--scale") == 0 || strcmp(argv[i], "-s") == 0) && i + 1 < argc) {
            args.options.scale_factor = atoi(argv[++i]);
        }
        // Dims (WxH)
        else if (strcmp(argv[i], "--dims") == 0 && i + 1 < argc) {
            char* val = argv[++i];
            char* x = strchr(val, 'x');
            if (x) {
                *x = '\0';
                args.options.target_pixel_w = atoi(val);
                args.options.target_pixel_h = atoi(x + 1);
            }
        }
    }

    // Post-process logic
    // 1. If no specific width set, try terminal size
    if (args.width == 0 && args.options.target_pixel_w == 0 && args.options.scale_factor == 0) {
        int t_w, t_h;
        if (try_get_terminal_size(&t_w, &t_h)) {
            args.width = t_w;
        } else {
            args.width = DEFAULT_MAX_WIDTH;
        }
        args.options.width_chars = args.width;
    }

    // 2. Generate output filename if exporting but no name given
    if (args.options.export_image && args.options.output_path == NULL) {
        // Extract base name
        char* base = strdup(args.filename);
        char* dot = strrchr(base, '.');
        if (dot) *dot = '\0'; // Remove extension

        char buf[1024];
        const char* ext = args.options.force_jpg ? "jpg" : "png";
        snprintf(buf, sizeof(buf), "%s_ascii.%s", base, ext);
        args.options.output_path = strdup(buf);
        free(base);
    }

    return args;
}