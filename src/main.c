#include <stdlib.h>

#include "engine.h"

int main(int argc, char **argv) {
  (void) argc;
  (void) argv;

  engine_init();

  engine_get_all_base_files();
  engine_parse_base_files();

  engine_exit();
  return EXIT_SUCCESS;
}
