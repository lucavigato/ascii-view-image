#include <stdio.h>
#include <stdlib.h>

#include "../include/image.h"
#include "../include/print_image.h"
#include "../include/argparse.h"
#include "../include/process.h"
#include "../include/export.h"

int main(int argc, char* argv[]) {
    // 1. Parse Arguments
    struct arguments args = parse_args(argc, argv);
    if (args.filename == NULL) {
        return 0; // Help was printed or invalid args
    }

    // 2. Load Image
    image_t original = load_image(args.filename);
    if (!original.data) {
        return 1; // Error printed inside load_image
    }

    // 3. Process Image (Create ASCII Grid)
    // We pass the export options because they contain width/height/scale info
    ascii_grid_t grid = process_image_to_grid(&original, &args.options);
    
    if (!grid.cells) {
        fprintf(stderr, "Error: Failed to process image.\n");
        free_image(&original);
        return 1;
    }

    // 4. Output: Export OR Print
    if (args.options.export_image) {
        export_ascii_to_image(&grid, &args.options);
    } else {
        print_image(&grid);
    }

    // 5. Cleanup
    free_ascii_grid(&grid);
    free_image(&original);
    
    // Free allocated strings in options
    if (args.options.output_path) free(args.options.output_path);
    if (args.options.font_family) free(args.options.font_family);

    return 0;
}