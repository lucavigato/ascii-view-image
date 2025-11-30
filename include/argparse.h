#ifndef ARGPARSE_H
#define ARGPARSE_H

#include "image.h" // Per export_options_t

struct arguments {
    char *filename;
    // Opzioni legacy/core
    int width; // Larghezza in caratteri (se 0, calcola in base a terminale o opzioni)
    
    // Tutte le opzioni di export e configurazione avanzata
    export_options_t options;
};

struct arguments parse_args(int argc, char *argv[]);

#endif
