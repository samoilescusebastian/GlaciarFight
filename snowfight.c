// Copyright 2018 Samoilescu Sebastian
#include "./snowfight.h"

cell_t **alloc_cells(cell_t **map, int dimension) {
  map = malloc(dimension * sizeof(cell_t *));
  if (map == NULL) {
    return NULL;
  }
  for (int i = 0; i < dimension; i++) {
    map[i] = malloc(dimension * sizeof(cell_t));
    if (map[i] == NULL) {
      for (int j = 0; j < i; j++) {
        free(map[j]);
      }
      free(map);
      return NULL;
    }
  }
  return map;
}

void free_cells(cell_t **map, int dimension) {
  for (int i = 0; i < dimension; i++) {
    free(map[i]);
  }
  free(map);
}

void *alloc_list(void *list, int length) {
  list = malloc(length * sizeof(elf_t));
  return list;
}

int verify_alloc(void *pointer) {
  if (pointer == NULL) {
    return 0;
  } else {
    return 1;
  }
}

// caut castigatorul printre elfi si il afisez
void get_winner(elf_t *elf_list, int length, FILE *file_out) {
  for (int i = 0; i < length; i++) {
    if (elf_list[i].dry == 1) {
      fprintf(file_out, "%s has won.\n", elf_list[i].name);
      break;
    }
  }
}

// construiesc fisierul de intrare pe care il deschid ulterior
void build_file_in(FILE **file_in, char *file_name) {
  snprintf(file_name + strlen(file_name), F_NAME, "%s", ".in");
  *file_in = fopen(file_name, "r");
}

// verific daca un anumit punct apartine ghetarului
int is_on_glacier(point_t point, point_t center, int radius) {
  if (sqrt((point.x - center.x) * (point.x - center.x) +
           (point.y - center.y) * (point.y - center.y)) > radius) {
    return 0;
  }
  return 1;
}

// obtin schimbarea coordonatelor in plan
void translate_move(char c, int *x, int *y) {
  if (c == 'U') {
    *x = -1;
    *y = 0;
  } else if (c == 'D') {
    *x = 1;
    *y = 0;
  } else if (c == 'L') {
    *x = 0;
    *y = -1;
  } else {
    *x = 0;
    *y = 1;
  }
}


// updatez informatiile unui spiridusi dupa ce acesta a fost implicat in lupta
void update_info(elf_t *elf_1, elf_t *elf_2) {
  (*elf_1).stamina += (*elf_2).stamina;
  (*elf_2).stamina = 0;
  (*elf_2).dry = 0;
  (*elf_2).point.x = -1;
  (*elf_2).point.y = -1;
  (*elf_1).victories++;
}

// insusirea manusiilor de catre un spiridus
void take_gloves(int id, point_t point, elf_t *elf_list, cell_t **map) {
  if (elf_list[id].dmg < map[point.x][point.y].gloves) {
    int old_gloves = elf_list[id].dmg;
    elf_list[id].dmg = map[point.x][point.y].gloves;
    map[point.x][point.y].gloves = old_gloves;
  }
}


void take_place(int id, point_t point, cell_t **map) {
  map[point.x][point.y].id = id;
}

void result_of_fight(elf_t *elf_list, int elf_1, int elf_2, FILE *file_out) {
  if (elf_list[elf_1].hp <= 0) {
    fprintf(file_out, "%s sent %s back home.\n", elf_list[elf_2].name,
            elf_list[elf_1].name);
  } else {
    fprintf(file_out, "%s sent %s back home.\n", elf_list[elf_1].name,
            elf_list[elf_2].name);
  }
}

// inceperea luptei
int start_fight(elf_t *elf_list, int elf_1, int elf_2, FILE *file_out) {
  int turn_1 = 1, turn_2 = 0;
  if (elf_list[elf_1].stamina < elf_list[elf_2].stamina) {
    turn_1 = 0;
    turn_2 = 1;
  }
  int battle_is_over = 0;
  // lupta va tine cat timp ambii spiridusi sunt in viata
  while (!battle_is_over) {
    if (turn_1 == 1) {
      elf_list[elf_2].hp -= elf_list[elf_1].dmg;
      if (elf_list[elf_2].hp <= 0) {
        battle_is_over = 1;
      }
    }
    if (turn_2 == 1) {
      elf_list[elf_1].hp -= elf_list[elf_2].dmg;
      if (elf_list[elf_1].hp <= 0) {
        battle_is_over = 1;
      }
    }
    turn_1 = 1 - turn_1;
    turn_2 = 1 - turn_2;
  }
  // urmaresc efectul luptei
  result_of_fight(elf_list, elf_1, elf_2, file_out);
  return elf_list[elf_1].hp > 0;
}

// decodific numarul K
void unpack_number(int K, point_t *point, int *R, int *dmg) {
  int container[C_SIZE];
  for (int i = 0; i < C_SIZE; i++) {
    container[i] = K & MASK;
    K >>= BYTE;
  }
  (*point).x = container[0];
  (*point).y = container[1];
  *R = container[2];
  *dmg = container[3];
}

// aplic efectul furtunii asupra unei singure celule
void hit_cell(elf_t *elf_list, cell_t **map, point_t hitted_cell, int dmg) {
  int elf_id = map[hitted_cell.x][hitted_cell.y].id;
  if (elf_id == -1) {
    return;
  }
  elf_list[elf_id].hp -= dmg;
  // updatez informatiile legate de un anumit spiridus in cazul in care acesta
  // a parasit ghetarul
  if (elf_list[elf_id].hp <= 0) {
    elf_list[elf_id].dry = -1;
    map[hitted_cell.x][hitted_cell.y].id = -1;
  }
}

void start_storm(point_t epicenter, point_t center, int R, int radius,
                 elf_t *elf_list, cell_t **map, int dmg) {
  // parcurgerea zonei afectate de furtuna
  for (int i = epicenter.x - radius; i <= epicenter.x; i++) {
    int diff = i - (epicenter.x - radius);
    int start_y = epicenter.y - diff;
    int end_y = epicenter.y + diff;
    for (int j = start_y; j <= end_y; j++) {
      point_t hitted_cell;
      hitted_cell.x = i;
      hitted_cell.y = j;
      // aplicarea efectului furtunii doar asupra ghetarului
      if (!is_on_glacier(hitted_cell, center, R)) {
        continue;
      }
      hit_cell(elf_list, map, hitted_cell, dmg);
    }
  }
  for (int i = epicenter.x + 1; i <= epicenter.x + radius; i++) {
    int diff = (epicenter.x + radius) - i;
    int start_y = epicenter.y - diff;
    int end_y = epicenter.y + diff;
    for (int j = start_y; j <= end_y; j++) {
      point_t hitted_cell;
      hitted_cell.x = i;
      hitted_cell.y = j;
      if (!is_on_glacier(hitted_cell, center, R)) {
        continue;
      }
      hit_cell(elf_list, map, hitted_cell, dmg);
    }
  }
}

// verific daca in urma furtunii exista spiridusi nevoiti sa paraseasca ghetarul
int storm_result(elf_t *elf_list, int P, FILE *file_out) {
  int eliminated_elves = 0;
  for (int i = 0; i < P; i++) {
    if (elf_list[i].dry == -1) {
      fprintf(file_out, "%s was hit by snowstorm.\n", elf_list[i].name);
      eliminated_elves++;
      elf_list[i].dry = 0;
      elf_list[i].point.x = -1;
      elf_list[i].point.y = -1;
    }
  }
  return eliminated_elves;
}

// compar elfii in functie de criteriile expuse in enunt
int compare_elves(elf_t *elf_1, elf_t *elf_2) {
  if ((*elf_1).dry < (*elf_2).dry) {
    return 1;
  } else if ((*elf_1).dry == (*elf_2).dry) {
    if ((*elf_1).victories < (*elf_2).victories) {
      return 1;
    } else if ((*elf_1).victories == (*elf_2).victories) {
      if (strcmp((*elf_1).name, (*elf_2).name) > 0) {
        return 1;
      }
    }
  }
  return 0;
}

// consider initial lista elfiilor sortata
void copy_id(int P, int *elves_id) {
  for (int i = 0; i < P; i++) {
    elves_id[i] = i;
  }
}

void swap_elves_id(int *id_1, int *id_2) {
  int aux = *id_1;
  *id_1 = *id_2;
  *id_2 = aux;
}

// sortez id -ul elfiilor
void sort_elves(elf_t *elf_list, int *elves_id, int P) {
  int sem;
  do {
    sem = 1;
    for (int i = 0; i < P - 1; i++) {
      if (compare_elves(&elf_list[elves_id[i]], &elf_list[elves_id[i + 1]])) {
        swap_elves_id(&elves_id[i], &elves_id[i + 1]);
        sem = 0;
      }
    }
  } while (sem == 0);
}

int get_command_id(char *command) {
  // obtinerea numelui comenzii citite din fisier, prima parte a comenzii
  char command_name[ELF_NAME];
  sscanf(command, "%s", command_name);
  // eliminarea numelui din cadrul comenzii
  char temp[ELF_NAME];
  snprintf(temp, ELF_NAME, "%s", command + strlen(command_name) + 1);
  snprintf(command, ELF_NAME, "%s", temp);
  char commands[COMMANDS][ELF_NAME];
  // codificare comezniilor dupa cum urmeaza
  snprintf(commands[0], ELF_NAME, "%s", "MOVE");
  snprintf(commands[1], ELF_NAME, "%s", "SNOWSTORM");
  snprintf(commands[2], ELF_NAME, "%s", "PRINT_SCOREBOARD");
  snprintf(commands[3], ELF_NAME, "%s", "MELTDOWN");
  for (int i = 0; i < 4; i++) {
    if (strcmp(commands[i], command_name) == 0) {
      return i + 1;
    }
  }
  return -1;
}

// functie ce exectuta mutarea spiridusilor
int move(char *command, point_t center, int R, int P, elf_t *elf_list,
         cell_t **map, FILE *file_out) {
  int elf_id = 0, eliminated_elves = 0;
  char moves[M_length];
  sscanf(command, "%d %s", &elf_id, moves);
  if (elf_id >= P || elf_list[elf_id].dry == 0) {
    return 0;
  }
  // parcurg intreg sirul de mutari
  for (unsigned int i = 0; i < strlen(moves); i++) {
    int move_x, move_y;
    point_t point;
    point = elf_list[elf_id].point;
    // obtin noile coordonate ale elfului
    translate_move(moves[i], &move_x, &move_y);
    point.x += move_x;
    point.y += move_y;
    int needed_stamina = abs(map[point.x][point.y].attitude -
                             map[point.x - move_x][point.y - move_y].attitude);
    if (needed_stamina > elf_list[elf_id].stamina) {
      continue;
    }
    if (is_on_glacier(point, center, R)) {
      // marcam plecarea spiridusului
      map[point.x - move_x][point.y - move_y].id = -1;
      elf_list[elf_id].stamina -= needed_stamina;
      int elf_2_id = map[point.x][point.y].id;

      take_gloves(elf_id, point, elf_list, map);
      // in cazul in care celula in care a aterizat spiridusul este ocupata
      // va avea loc lupta, altfel spiridsul va ocupa celula
      if (elf_2_id == -1) {
        take_place(elf_id, point, map);
      } else {
        eliminated_elves++;
        if (start_fight(elf_list, elf_id, elf_2_id, file_out)) {
          take_place(elf_id, point, map);
          update_info(&elf_list[elf_id], &elf_list[elf_2_id]);

        } else {
          update_info(&elf_list[elf_2_id], &elf_list[elf_id]);
          return eliminated_elves;
        }
      }
      elf_list[elf_id].point = point;
    // daca celula in care aterizat spiridusul nu face parte din ghetar,
    // marcam plecarea spiridusului si oprim sirul de mutari
    } else {
      fprintf(file_out, "%s fell off the glacier.\n", elf_list[elf_id].name);
      eliminated_elves++;
      elf_list[elf_id].dry = 0;
      map[elf_list[elf_id].point.x][elf_list[elf_id].point.y].id = -1;
      elf_list[elf_id].point.x = -1;
      elf_list[elf_id].point.y = -1;
      break;
    }
  }
  return eliminated_elves;
}

// functia de declansare a furtunii
int SNOWSTORM(char *command, point_t center, int R, int P, elf_t *elf_list,
              cell_t **map, FILE *file_out) {
  int K;
  sscanf(command, "%d", &K);
  // printf("%d\n", K);
  point_t epicenter;
  int radius;
  int dmg;
  unpack_number(K, &epicenter, &radius, &dmg);
  start_storm(epicenter, center, R, radius, elf_list, map, dmg);
  return storm_result(elf_list, P, file_out);
}

// functia de printare a clasamentului
void PRINT_SCOREBOARD(int P, elf_t *elf_list, int *elves_id, FILE *file_out) {
  sort_elves(elf_list, elves_id, P);
  fprintf(file_out, "SCOREBOARD:\n");
  // decodificam starea elfiilor
  // 0 - WET
  // 1 - DRY
  for (int i = 0; i < P; i++) {
    char status[C_SIZE] = "DRY";
    if (elf_list[elves_id[i]].dry == 0) {
      snprintf(status, C_SIZE, "%s", "WET");
    }
    fprintf(file_out, "%s\t%s\t%d\n", elf_list[elves_id[i]].name, status,
            elf_list[elves_id[i]].victories);
  }
}

// functia ce redimensioneaza ghetarul
int MELTDOWN(char *command, point_t center, int *R, int P, elf_t *elf_list,
             FILE *file_out) {
  *R -= 1;
  int received_stamina, eliminated_elves = 0;
  sscanf(command, "%d", &received_stamina);
  // parcurgem lista de elfi verificand care dintre acestia se afla pe noul
  // ghetar. Cei care se afla primesc stamina, iar ceilalti sunt marcati ca
  // eliminati de pe ghetar
  for (int i = 0; i < P; i++) {
    if (elf_list[i].dry == 1) {
      if (is_on_glacier(elf_list[i].point, center, *R)) {
        elf_list[i].stamina += received_stamina;
      } else {
        fprintf(file_out, "%s got wet because of global warming.\n",
                elf_list[i].name);
        elf_list[i].dry = 0;
        elf_list[i].point.x = -1;
        elf_list[i].point.y = -1;
        eliminated_elves++;
      }
    }
  }
  return eliminated_elves;
}

int read_info(FILE *file_in, int *R, int *P, elf_t **elf_list, cell_t ***map,
              int *on_board_elves, FILE *file_out) {
  fscanf(file_in, "%d %d", R, P);
  *on_board_elves = *P;
  *elf_list = alloc_list(*elf_list, *P);
  if (verify_alloc(elf_list) == 0) {
    return 0;
  }
  int dimension = (2 * *R) + 1;
  point_t center;
  center.x = *R;
  center.y = *R;
  *map = alloc_cells(*map, dimension);
  if (*map == NULL) {
    return 0;
  }
  // se atribuie informatiile legate de ghetar
  for (int i = 0; i < dimension; i++) {
    for (int j = 0; j < dimension; j++) {
      fscanf(file_in, "%d %d", &((*map)[i][j].attitude),
             &((*map)[i][j].gloves));
      (*map)[i][j].id = -1;
    }
  }
  char name[ELF_NAME];
  int hp, stamina;
  point_t point;
  // se atribuie informatiile elfiilor aflati pe ghetar
  for (int id = 0; id < *P; id++) {
    fscanf(file_in, "%s %d %d %d %d", name, &point.x, &point.y, &hp, &stamina);
    (*elf_list)[id].dry = 1;
    snprintf((*elf_list)[id].name, ELF_NAME, "%s", name);
    (*elf_list)[id].point.x = point.x;
    (*elf_list)[id].point.y = point.y;
    (*elf_list)[id].hp = hp;
    (*elf_list)[id].stamina = stamina;
    (*elf_list)[id].victories = 0;
    if (is_on_glacier(point, center, *R)) {
      (*map)[point.x][point.y].id = id;
      (*elf_list)[id].dmg = (*map)[point.x][point.y].gloves;
      (*map)[point.x][point.y].gloves = 0;
    // se marcheaza elfii ce nu au aterizat pe ghetar
    } else {
      (*on_board_elves)--;
      fprintf(file_out, "%s has missed the glacier.\n", name);
      (*elf_list)[id].hp = 0;
      (*elf_list)[id].dry = 0;
      (*elf_list)[id].point.x = -1;
      (*elf_list)[id].point.y = -1;
    }
  }
  return 1;
}

int read_commands(FILE *file_in, int R, int P, elf_t *elf_list, cell_t **map,
                  int on_board_elves, FILE *file_out) {
  char command[C_length];
  point_t center;
  center.x = R;
  center.y = R;
  int command_id;
  int eliminated_elves = 0;
  int *elves_id = malloc(P * sizeof(int));
  if (elves_id == NULL) {
    return -1;
  }
  copy_id(P, elves_id);
  // citirea comenziilor pana la sfarsitul fisierului
  // citesc linie cu line comenziile
  while (fgets(command, C_length, file_in)) {
    if (command[strlen(command) - 1] == '\n') {
      command[strlen(command) - 1] = '\0';
    }
    if (strlen(command) == 0) {
      continue;
    }
    // updatez numarul elfiilor aflati pe ghetar
    on_board_elves -= eliminated_elves;
    eliminated_elves = 0;
    // verific daca exista un castigator
    if (on_board_elves <= 1) {
      get_winner(elf_list, P, file_out);
      free(elves_id);
      return 1;
    }
    command_id = get_command_id(command);
    // se executa comanda asociata id -ului
    switch (command_id) {
      case 1: {
        eliminated_elves = move(command, center, R, P, elf_list, map, file_out);
        break;
      }
      case 2: {
        eliminated_elves =
            SNOWSTORM(command, center, R, P, elf_list, map, file_out);

        break;
      }
      case 3: {
        PRINT_SCOREBOARD(P, elf_list, elves_id, file_out);
        break;
      }
      case 4: {
        eliminated_elves = MELTDOWN(command, center, &R, P, elf_list, file_out);
        break;
      }
    }
  }
  free(elves_id);
  return 1;
}
void print_list(elf_t *elf_list, int length) {
  for (int i = 0; i < length; i++) {
    printf("%s %d %d %d %d %d %d %d\n", elf_list[i].name, elf_list[i].dry,
           elf_list[i].point.x, elf_list[i].point.y, elf_list[i].dmg,
           elf_list[i].hp, elf_list[i].stamina, elf_list[i].victories);
  }
}

