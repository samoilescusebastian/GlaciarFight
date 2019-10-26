// Copyright 2018 Samoilescu Sebastian
#include "./snowfight.h"
int main(int argc, char **argv) {
  int R, P;
  FILE *file_in;
  // deschid fisierul din care vom citi datele de intrare
  build_file_in(&file_in, argv[argc - 1]);
  if (file_in == NULL) {
    return -1;
  }
  FILE *file_out = fopen("snowfight.out", "w");
  if (file_out == NULL) {
    return -1;
  }
  elf_t *elf_list = NULL;
  cell_t **map = NULL;
  int on_board_elves;
  // citesc informatiile legate de ghetar si de spiridusi
  int result =
      read_info(file_in, &R, &P, &elf_list, &map, &on_board_elves, file_out);
  if (result == -1) {
    return -1;
  }
  int dimension = 2 * R + 1;
  // citesc si execut comenzile existente in fisier
  if (read_commands(file_in, R, P, elf_list, map, on_board_elves, file_out) ==
      -1) {
    return -1;
  }
  free(elf_list);
  free_cells(map, dimension);
  fclose(file_in);
  fclose(file_out);
  return 0;
}
