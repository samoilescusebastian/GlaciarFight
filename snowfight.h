// Copyright 2018 Samoilescu Sebastian
#ifndef _SNOWFIGHT_H
#define _SNOWFIGHT_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MASK 255
#define C_SIZE 4
#define BYTE 8
#define ELF_NAME 30
#define F_NAME 30
#define C_length 100
#define M_length 80
#define COMMANDS 4
typedef struct {
  int x, y;
} point_t;

typedef struct {
  char name[ELF_NAME];
  point_t point;
  int hp, stamina, dmg, dry, victories;
} elf_t;

typedef struct {
  int id, gloves, attitude;
} cell_t;

cell_t **alloc_cells(cell_t **map, int dimension);
void free_cells(cell_t **map, int dimension);
void *alloc_list(void *list, int length);
int verify_alloc(void *pointer);
void get_winner(elf_t *elf_list, int length, FILE *file_out);
void build_file_in(FILE **file_in, char *file_name);
int is_on_glacier(point_t point, point_t center, int radius);
void translate_move(char c, int *x, int *y);
void update_info(elf_t *elf_1, elf_t *elf_2);
void take_gloves(int id, point_t point, elf_t *elf_list, cell_t **map);
void take_place(int id, point_t point, cell_t **map);
void result_of_fight(elf_t *elf_list, int elf_1, int elf_2, FILE *file_out);
int start_fight(elf_t *elf_list, int elf_1, int elf_2, FILE *file_out);
void unpack_number(int K, point_t *point, int *R, int *dmg);
void hit_cell(elf_t *elf_list, cell_t **map, point_t hitted_cell, int dmg);
void start_storm(point_t epicenter, point_t center, int R, int radius,
                 elf_t *elf_list, cell_t **map, int dmg);
int storm_result(elf_t *elf_list, int P, FILE *file_out);
int compare_elves(elf_t *elf_1, elf_t *elf_2);
void copy_id(int P, int *elves_id);
void swap_elves_id(int *id_1, int *id_2);
void sort_elves(elf_t *elf_list, int *elves_id, int P);
int get_command_id(char *command);
int move(char *command, point_t center, int R, int P, elf_t *elf_list,
         cell_t **map, FILE *file_out);
int SNOWSTORM(char *command, point_t center, int R, int P, elf_t *elf_list,
              cell_t **map, FILE *file_out);
void PRINT_SCOREBOARD(int P, elf_t *elf_list, int *elves_id, FILE *file_out);
int MELTDOWN(char *command, point_t center, int *R, int P, elf_t *elf_list,
             FILE *file_out);
int read_info(FILE *file_in, int *R, int *P, elf_t **elf_list, cell_t ***map,
              int *on_board_elves, FILE *file_out);
int read_commands(FILE *file_in, int R, int P, elf_t *elf_list, cell_t **map,
                  int on_board_elves, FILE *file_out);
void print_list(elf_t *elf_list, int length);
void print_map(cell_t **map, point_t center, int radius);
#endif
