#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  
  if (argc < 5) {
    // argv[0] -> nome dello script
    // argv[1] -> nome immagine
    // argv[2] -> output immagine
    // argv[3] -> width
    // argv[4] -> height

    printf("Uso: %s nome_immagine output_immagine width height", argv[0]);
    return 1;
  }


  return 0;
}
