-//xvesela00
//29.11.2023
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>


typedef struct{
    int rows;
    int cols;
    unsigned char *cells;
} Map;
// BFS structures
typedef struct Node{
  int curr_index; // location of node in map
  struct Node *next[3]; // links nodes together
} Node;

typedef struct{
  int index_amount; // number of indexes
  Node **adjacent_node_list; // adjacency list containing neighbouring nodes//an array of pointers to Node structs
  int *visited; // visited indexes
} Graph;

typedef struct{
  int front;
  int rear;
  int *items;
} Queue;

typedef struct{
    int node_amount;
    Node **node_list;
} Final_list;
// ***


void help(void);
void test(Map *, FILE *);
void rpath(Map *, int, int, FILE *);
void lpath(Map *, int, int, FILE *);
void shortest(Map *, int, int, FILE *);

bool isborder(Map *, int, int, int);
int argcheck(Map *, int, int);

void map_init(Map *);
int struct_ctor(Map *, FILE *);
void struct_dtor(Map **);

int compare_borders(Map *, int, int);
int start_border(Map *, int, int, int, int *);

void move_lpath(Map *, int *, int *, int *, int *, int *);
void move_rpath(Map *, int *, int *, int *, int *, int *);

void go_left(int *, int *, int*);
void go_right(int *, int *, int*);
void go_up(Map *, int *, int *, int *);
void go_down(Map *, int *, int *, int *);
// BFS functions
int add_exit(unsigned char **, int, int);
int exit_check(unsigned char *, int, int);

void create_graph(Graph *, int);
void create_node(Graph *, Node *, int);
void add_link(Graph *, Node *, int, int);

void bfs(Graph *, Queue *, Node *, int, int);

void create_queue(Queue **);
void enqueue(Queue **, int, int, int *);
int dequeue(Queue *);
void display(Queue *);
int is_empty(Queue *);
void print_queue(Queue *);
// ***


int main(int argc, char *argv[]){

    Map *map = NULL;
    map = malloc(sizeof(Map));
    if(map == NULL){
        fprintf(stderr, "ERROR: MEMORY ALLOCATION FAILED\n");
        return 1;
    }
    map_init(map);
    FILE *file = NULL;
    int R, C;
    R = -1;
    C = -1;

    if(argc != 2 && argc != 3 && argc != 5){
        fprintf(stderr, "ERROR: INCORRECT AMOUNT OF ARGUMENTS. Use ./maze --help for more info\n");
        struct_dtor(&map);
        return 1;
    }

    if(argc == 3 || argc == 5){
        file = fopen(argv[argc - 1], "r");

        if(file == NULL){
            fprintf(stderr, "ERROR: UNABLE TO OPEN FILE\n");
            struct_dtor(&map);
            return 1;
        }
    }

    if(argc == 5){
        char *endPtr;
        R = strtol(argv[2], &endPtr, 10);
        C = strtol(argv[3], &endPtr, 10);

        if (*endPtr != '\0'){
            fprintf(stderr, "ERROR: INVALID NUMBER/S\n");
            struct_dtor(&map);
            return 1;
        }
        R -= 1; // -1 because I count from 0, bite me
        C -= 1;
    }

    if(strcmp(argv[1], "--help") == 0 && argc == 2){
        help();
    }
    else if(strcmp(argv[1], "--test") == 0 && argc == 3){
        test(map, file);
    }
    else if(strcmp(argv[1], "--rpath") == 0 && argc == 5){
        rpath(map, R, C, file);
    }
    else if(strcmp(argv[1], "--lpath") == 0 && argc == 5){
        lpath(map, R, C, file);
    }
    else if(strcmp(argv[1], "--shortest") == 0 && argc == 5){
        shortest(map, R, C, file);
    }
    else{
        fprintf(stderr, "ERROR: WRONG ARGUMENT, USE \"./maze --help\" FOR MORE INFO\n");
        struct_dtor(&map);
        return 1;
    }
    if(file != NULL){
        fclose(file);
    }
    if(map != NULL){
        struct_dtor(&map);
    }
    return 0;
}

bool isborder(Map *map, int r, int c, int border){ // border: left = 1, right = 2, top||bottom = 3 -> position of the bit we are evaluating so left = 1st bit(LSB)
    unsigned char temp_bit;
    temp_bit = 1 << (border - 1); // to help with comparing binary // or pass in 1, 2 or 4 if we want to avoid bit shifting
    if(((map -> cells[(r * map -> cols) + c]) & temp_bit) != 0){ // deal with this fuckery Adam!!!
        return true;
    }
    return false;
}

void help(){
    printf("---------------------------------------------------------------HOW TO RUN THE MAZE ABSOLUTE SOLVER PROGRAM---------------------------------------------------------------\n"
        "\t\"./maze --help\" --> info on how to run program\n"
        "\t\"./maze --test filename.txt\" --> checks validity of map in textfile\n"
        "\t\"./maze --lpath row column filename.txt\" --> solves the maze by printing coordinates of visited cells using the left-hand rule\n"
        "\t\"./maze --rpath row column filename.txt\" --> solves the maze by printing coordinates of visited cells using the right-hand rule\n"
        "\t\"./maze --shortest row column filename.txt\" --> solves the maze by finding the shortest route outside of the maze by printing the coordinates of visited cells\n"
        "\t*** \"row\" and \"column\" are integers between 0 and the first two numbers in the textfile respectively - 1st number for row, 2nd number for column ***\n"
        "-------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
}

void test(Map *map, FILE *textfile){
    int triangle_type = -1;
    int count = -1;
    count = struct_ctor(map, textfile);

    if((count != ((map -> rows) * (map -> cols))) || count < 1){
        printf("Invalid\n");
        return;
    }
    for(int i = 0; i < (map -> rows) * (map -> cols); i++){   // -1, +1 -> left, right; -c, +c -> up, down
        triangle_type = (((i / (map -> cols)) % 2) + (i % map -> cols)) % 2;  // ((i / map->cols) % 2) -> inverts the order of triangles (up or down) every other row
        if(compare_borders(map, triangle_type, i)){
            return;
        }
    }
    printf("Valid\n");
}

int argcheck(Map *map, int R, int C){
    if(((map -> rows) > R) && ((map -> cols) > C) && (R >= 0) && (C >= 0)){
        return 1;
    }
    return 0;
}

int struct_ctor(Map *map, FILE *textfile){
    int r, c, count, character;
    fscanf(textfile, "%d %d", &r, &c);
    if(r < 0 || c < 0){
        fprintf(stderr, "ERROR: NEGATIVE VALUES NOT ALLOWED\n");
        return -1;
    }
    map -> cells = malloc(sizeof(unsigned char) * r * c);
    if(map -> cells == NULL){
        fprintf(stderr, "ERROR: MEMORY ALLOCATION FAILED\n");
        return -1;
    }

    map -> rows = r;
    map -> cols = c;
    count = 0;
    
    while(fscanf(textfile, "%d", &character) == 1 && count < ((map -> rows) * (map -> cols))){ // checks if function found any numbers in file
        map -> cells[count++] = (unsigned char)character;
    }
    return count;
}

void struct_dtor(Map **map){
    if(((*map) -> cells) != NULL){
        free((*map) -> cells);
        (*map) -> cells = NULL;
    }
    free(*map);
    *map = NULL;
}

int compare_borders(Map *map, int t_type, int i){  // t_type -> up or down triangle (0 - up, 1 - down)
    int r, c;
    r = i / (map -> cols);  // evaluates which row we are on
    c = i % (map -> cols);  // evaluates which column we are on

    bool l_border, r_border, u_border, d_border; // the borders of the map, not the triangle
    l_border = (c == 0); // the left-most border of the map
    r_border = (c == ((map -> cols) - 1)); // the right-most border of the map
    u_border = (r == 0); // the upper-most border of the map
    d_border = (r == ((map -> rows) - 1)); // the lower-most border of the map

    if(!l_border){
        if(isborder(map, r, c, 1) != isborder(map, r, (c - 1), 2)){ // 1 -> left border (1st bit[LSB])
            printf("Invalid\n");
            return 1;
        }
    }
    if(!r_border){
        if(isborder(map, r, c, 2) != isborder(map, r, (c + 1), 1)){ // 2 -> right border (2nd bit)
            printf("Invalid\n");
            return 1;
        }
    }
    if(t_type == 0){  // -map -> cols (r - 1) to compare with triangle above it
        if(!u_border){
            if(isborder(map, r, c, 3) != isborder(map, (r - 1), c, 3)){ // 3 -> upper border (3rd bit)
                printf("Invalid\n");
                return 1;
            }
        }
    }
    else if(t_type == 1){ // +map -> cols (r + 1) to compare with triangle below it
        if(!d_border){
            if(isborder(map, r, c, 3) != isborder(map, (r + 1), c, 3)){ // 3 -> upper border (3rd bit)
                printf("Invalid\n");
                return 1;
            }
        }
    }
    else{
        fprintf(stderr, "ERROR: TRIANGLE TYPE INVALID\n");
        return 1;
    }
    return 0;
}

void map_init(Map *map){
    map -> rows = 0;
    map -> cols = 0;
    map -> cells = NULL;
}

void rpath(Map *map, int R, int C, FILE *textfile){
    if(struct_ctor(map, textfile) < 0){
        return;
    }
    if(!argcheck(map, R, C)){
        fprintf(stderr, "ERROR: INVALID ARGUMENTS\n");
        return;
    }
    int direction = 0; // left = 1; right = 2; up = 3; down = 4;
    int position = (R * (map -> cols)) + C;
    int triangle_type = -1;

    int curr_row, curr_col;
    curr_row = R;
    curr_col = C;

    if(!start_border(map, R, C, 1, &direction)){
        fprintf(stderr, "ERROR: NO AVAILABLE BORDER TO ENTER FROM\n");
        return;
    }

    while(curr_row >= 0 && curr_row < (map -> rows) && curr_col >= 0 && curr_col < (map -> cols)){
        printf("%d,%d\n", (curr_row + 1), (curr_col + 1));
        triangle_type = (curr_row + curr_col) % 2;
        if(triangle_type != 0 && triangle_type != 1){
            break;
        }
        move_rpath(map, &position, &curr_row, &curr_col, &direction, &triangle_type);
    }
}

void lpath(Map *map, int R, int C, FILE *textfile){
    if(struct_ctor(map, textfile) < 0){
        return;
    }
    if(!argcheck(map, R, C)){
        fprintf(stderr, "ERROR: INVALID ARGUMENTS\n");
        return;
    }
    int direction = 0; // west = 1; east = 2; north = 3; south = 4;
    int position = (R * (map -> cols)) + C;
    int triangle_type = -1;

    int curr_row, curr_col;
    curr_row = R;
    curr_col = C;

    if(!start_border(map, R, C, 0, &direction)){
        fprintf(stderr, "ERROR: NO AVAILABLE BORDER TO ENTER FROM\n");
        return;
    }

    while(curr_row >= 0 && curr_row < (map -> rows) && curr_col >= 0 && curr_col < (map -> cols)){
        printf("%d,%d\n", (curr_row + 1), (curr_col + 1));
        triangle_type = (curr_row + curr_col) % 2;
        if(triangle_type != 0 && triangle_type != 1){
            break;
        }
        move_lpath(map, &position, &curr_row, &curr_col, &direction, &triangle_type);
    }
}
// I could've easily optimized everything below, for the record, but no time (definitely not lazy)
// returns 1(left), 2(right), 3(top/bottom), or 0(no open borders)
int start_border(Map *map, int r, int c, int leftright, int *direction){ // leftright => left = 0; right = 1;
    if(r < 0 || r > map -> rows || c < 0 || c > map -> cols){
        fprintf(stderr, "ERROR: INVALID ARGUMENTS\n");
        return 0;
    }
    int triangle_type = (r + c) % 2; // 0 -> upper border, 1 -> lower border;

    bool l_border, r_border, u_border, d_border; // the borders of the map, not the triangle
    l_border = c == 0; // the left-most border of the map
    r_border = c == (map -> cols) - 1; // the right-most border of the map
    u_border = r == 0; // the upper-most border of the map
    d_border = r == (map -> rows) - 1; // the lower-most border of the map

    if(!l_border && !r_border && !u_border && !d_border){
        fprintf(stderr, "ERROR: INVALID STARTING POSITION\n");
        return 0;
    }

    if(leftright == 0){ // left hand rule
        if(l_border && (triangle_type == 0) && !isborder(map, r, c, 1)){
            *direction = 2;
            return 3; // up
        }
        else if(l_border && (triangle_type == 1) && !isborder(map, r, c, 1)){
            *direction = 2;
            return 2; // right
        }
        else if(r_border && (triangle_type == 0) && !isborder(map, r, c, 2)){
            *direction = 1;
            return 1; // left
        }
        else if(r_border && (triangle_type == 1) && !isborder(map, r, c, 2)){
            *direction = 1;
            return 3; // down
        }
        else if(u_border && (triangle_type == 0) && !isborder(map, r, c, 3)){
            *direction = 4;
            return 2; // right
        }
        else if(d_border && (triangle_type == 1) && !isborder(map, r, c, 3)){
            *direction = 3;
            return 1; // left
        }
        else{
            // fprintf(stderr, "ERROR: NO AVAILABLE BORDER TO ENTER FROM\n");
            return 0;
        }
    }
    else if(leftright == 1){ // right hand rule
        if(l_border && (triangle_type == 0) && !isborder(map, r, c, 1)){
            *direction = 2;
            return 2; // right
        }
        else if(l_border && (triangle_type == 1) && !isborder(map, r, c, 1)){
            *direction = 2;
            return 3; // down
        }
        else if(r_border && (triangle_type == 0) && !isborder(map, r, c, 2)){
            *direction = 1;
            return 3; // up
        }
        else if(r_border && (triangle_type == 1) && !isborder(map, r, c, 2)){
            *direction = 1;
            return 1; // left
        }
        else if(u_border && (triangle_type == 0) && !isborder(map, r, c, 3)){
            *direction = 4;
            return 1; // left
        }
        else if(d_border && (triangle_type == 1) && !isborder(map, r, c, 3)){
            *direction = 3;
            return 2; // right
        }
        else{
            // fprintf(stderr, "ERROR: NO AVAILABLE BORDER TO ENTER FROM\n");
            return 0;
        }
    }
    else{
        fprintf(stderr, "ERROR: UNKNOWN RULE\n");
        return 0;
    }
}
// future me: Direction variable -> +1 -> right, -1 -> left, +cols -> down, -cols -> up

void move_lpath(Map *map, int *position, int *curr_row, int *curr_col, int *direction, int *triangle_type){
    if(*direction == 1){ // facing west 
        if(*triangle_type == 0){
            if(!isborder(map, *curr_row, *curr_col, 1)){ // tries left border
                go_left(position, curr_col, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 3)){ // tries upper border
                go_up(map, position, curr_row, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 2)){ // tries right border
                go_right(position, curr_col, direction);
            }
            else{
                fprintf(stderr, "ERROR: NO AVAILABLE PATH\n");
            }
        }
        else if(*triangle_type == 1){
            if(!isborder(map, *curr_row, *curr_col, 3)){ // tries lower border
                go_down(map, position, curr_row, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 1)){ // tries left border
                go_left(position, curr_col, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 2)){ // tries right border
                go_right(position, curr_col, direction);
            }
            else{
                fprintf(stderr, "ERROR: NO AVAILABLE PATH\n");
            }
        }
        else{
            fprintf(stderr, "ERROR: INVALID TRIANGLE TYPE\n");
        }
    }
    else if(*direction == 2){ // facing east
        if(*triangle_type == 0){
            if(!isborder(map, *curr_row, *curr_col, 3)){ // tries upper border
                go_up(map, position, curr_row, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 2)){ // tries right border
                go_right(position, curr_col, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 1)){ // tries left border
                go_left(position, curr_col, direction);
            }
            else{
                fprintf(stderr, "ERROR: NO AVAILABLE PATH\n");
            }
        }
        else if(*triangle_type == 1){
            if(!isborder(map, *curr_row, *curr_col, 2)){ // tries right border
                go_right(position, curr_col, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 3)){ // tries lower border
                go_down(map, position, curr_row, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 1)){ // tries left border
                go_left(position, curr_col, direction);
            }
            else{
                fprintf(stderr, "ERROR: NO AVAILABLE PATH\n");
            }
        }
        else{
            fprintf(stderr, "ERROR: INVALID TRIANGLE TYPE\n");
        }
    }
    else if(*direction == 3){ // facing north
        if(*triangle_type == 1){
            if(!isborder(map, *curr_row, *curr_col, 1)){ // tries left border
                go_left(position, curr_col, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 2)){ // tries right border
                go_right(position, curr_col, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 3)){ // tries lower border
                go_down(map, position, curr_row, direction);
            }
            else{
                fprintf(stderr, "ERROR: NO AVAILABLE PATH\n");
            }
        }
        else{
            fprintf(stderr, "ERROR: INVALID TRIANGLE TYPE\n");
        }
    }
    else if(*direction == 4){ // facing south
        if(*triangle_type == 0){
            if(!isborder(map, *curr_row, *curr_col, 2)){ // tries right border
                go_right(position, curr_col, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 1)){ // tries left border
                go_left(position, curr_col, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 3)){ // tries upper border
                go_up(map, position, curr_row, direction);
            }
            else{
                fprintf(stderr, "ERROR: NO AVAILABLE PATH\n");
            }
        }
        else{
            fprintf(stderr, "ERROR: INVALID TRIANGLE TYPE\n");
        }
    }
    else{
        fprintf(stderr, "ERROR: INVALID DIRECTION\n");
    }
}

void move_rpath(Map *map, int *position, int *curr_row, int *curr_col, int *direction, int *triangle_type){
    if(*direction == 1){ // facing west 
        if(*triangle_type == 0){
            if(!isborder(map, *curr_row, *curr_col, 3)){ // tries upper border
                go_up(map, position, curr_row, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 1)){ // tries left border
                go_left(position, curr_col, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 2)){ // tries right border
                go_right(position, curr_col, direction);
            }
            else{
                fprintf(stderr, "ERROR: NO AVAILABLE PATH\n");
            }
        }
        else if(*triangle_type == 1){
            if(!isborder(map, *curr_row, *curr_col, 1)){ // tries left border
                go_left(position, curr_col, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 3)){ // tries lower border
                go_down(map, position, curr_row, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 2)){ // tries right border
                go_right(position, curr_col, direction);
            }
            else{
                fprintf(stderr, "ERROR: NO AVAILABLE PATH\n");
            }
        }
        else{
            fprintf(stderr, "ERROR: INVALID TRIANGLE TYPE\n");
        }
    }
    else if(*direction == 2){ // facing east
        if(*triangle_type == 0){
            if(!isborder(map, *curr_row, *curr_col, 2)){ // tries right border
                go_right(position, curr_col, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 3)){ // tries upper border
                go_up(map, position, curr_row, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 1)){ // tries left border
                go_left(position, curr_col, direction);
            }
            else{
                fprintf(stderr, "ERROR: NO AVAILABLE PATH\n");
            }
        }
        else if(*triangle_type == 1){
            if(!isborder(map, *curr_row, *curr_col, 3)){ // tries lower border
                go_down(map, position, curr_row, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 2)){ // tries right border
                go_right(position, curr_col, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 1)){ // tries left border
                go_left(position, curr_col, direction);
            }
            else{
                fprintf(stderr, "ERROR: NO AVAILABLE PATH\n");
            }
        }
        else{
            fprintf(stderr, "ERROR: INVALID TRIANGLE TYPE\n");
        }
    }
    else if(*direction == 3){ // facing north
        if(*triangle_type == 1){
            if(!isborder(map, *curr_row, *curr_col, 2)){ // tries right border
                go_right(position, curr_col, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 1)){ // tries left border
                go_left(position, curr_col, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 3)){ // tries lower border
                go_down(map, position, curr_row, direction);
            }
            else{
                fprintf(stderr, "ERROR: NO AVAILABLE PATH\n");
            }
        }
        else{
            fprintf(stderr, "ERROR: INVALID TRIANGLE TYPE\n");
        }
    }
    else if(*direction == 4){ // facing south
        if(*triangle_type == 0){
            if(!isborder(map, *curr_row, *curr_col, 1)){ // tries left border
                go_left(position, curr_col, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 2)){ // tries right border
                go_right (position, curr_col, direction);
            }
            else if(!isborder(map, *curr_row, *curr_col, 3)){ // tries upper border
                go_up(map, position, curr_row, direction);
            }
            else{
                fprintf(stderr, "ERROR: NO AVAILABLE PATH\n");
            }
        }
        else{
            fprintf(stderr, "ERROR: INVALID TRIANGLE TYPE\n");
        }
    }
    else{
        fprintf(stderr, "ERROR: INVALID DIRECTION\n");
    }
}

void go_left(int *position, int *curr_col, int *direction){
    *position -= 1;
    *curr_col -= 1;
    *direction = 1;
}

void go_right(int *position, int *curr_col, int *direction){
    *position += 1;
    *curr_col += 1;
    *direction = 2;
}

void go_up(Map *map, int *position, int *curr_row, int *direction){
    *position -= map -> cols;
    *curr_row -= 1;
    *direction = 3;
}

void go_down(Map *map, int *position, int *curr_row, int *direction){
    *position += map -> cols;
    *curr_row += 1;
    *direction = 4;
}

// BFS Things (confusing as fuck, but we ball)
void shortest(Map *map, int R, int C, FILE *textfile){
    if(struct_ctor(map, textfile) < 0){
        return;
    }
    if(!argcheck(map, R, C)){
        fprintf(stderr, "ERROR: INVALID ARGUMENTS\n");
        return;
    }
    int count = 1; // number of exits
    int direction = 0; // west/left = 1; east/right = 2; north/up = 3; south/down = 4;
    int position = (R * (map -> cols)) + C;
    int start_position = position;
    int triangle_type = -1;
    int curr_row, curr_col;
    curr_row = R;
    curr_col = C;
    unsigned char *exit_points = malloc(sizeof(unsigned char));
    if(exit_points == NULL){
        fprintf(stderr, "ERROR: MEMORY ALLOCATION UNSUCCESSFUL\n");
        return;
    }
    exit_points[0] = position;
    // finds all available exits
    for(int i = 0; i < map -> rows; i++){ // checks the left side of the maze for exits
        if(start_border(map, i, 0, 0, &direction) != 0){
            if(add_exit(&exit_points, count, i * (map -> cols))){
                continue;
            }
            count++;
        }
    }
    for(int i = 0; i < map -> rows; i++){ // checks the right side of the maze for exits
        if(start_border(map, i, (map -> cols) - 1, 0, &direction) != 0){
            if(add_exit(&exit_points, count, (i * map -> cols) + (map -> cols - 1))){
                continue;
            }
            count++;
        }
    }
    for(int i = 0; i < map -> cols; i++){ // checks the upper side of the maze for exits
        if(start_border(map, 0, i, 0, &direction) != 0){
            if(add_exit(&exit_points, count, i)){
                continue;
            }
            count++;
        }
    }
    for(int i = 0; i < map -> cols; i++){ // checks the lower side of the maze for exits
        if(start_border(map, (map -> rows) - 1, i, 0, &direction) != 0){
            if(add_exit(&exit_points, count, ((map -> rows - 1) * (map -> cols)) + i)){
                continue;
            }
            count++;
        }
    }
    // start of algorithm
    start_border(map, curr_row, curr_col, 0, &direction);
    int size = 0;
    size = (map -> rows) * (map -> cols); // map size
    Graph *graph = malloc(sizeof(Graph));
    Queue *q = malloc(sizeof(Queue));
    Node *node = malloc(sizeof(Node));
    // node -> next[0] = malloc(sizeof(Node));
    if(graph == NULL){
        fprintf(stderr, "ERROR: MEMORY ALLOCATION UNSUCCESSFUL\n");
        return;
    }
    if(q == NULL){
        fprintf(stderr, "ERROR: MEMORY ALLOCATION UNSUCCESSFUL\n");
        return;
    }
    if(node == NULL){
        fprintf(stderr, "ERROR: MEMORY ALLOCATION UNSUCCESSFUL\n");
        return;
    }
    if(node -> next == NULL){
        fprintf(stderr, "ERROR: MEMORY ALLOCATION UNSUCCESSFUL\n");
        return;
    }

    create_graph(graph, size);

    int links_to_exit = 0;
    triangle_type = (curr_row + curr_col) % 2;

    // printf("%d\n", graph->visited[0]);
// =========================== start fixing shit here ===========================
    while(links_to_exit < count){
        if(direction == 1){
            if(!isborder(map, curr_row, curr_col, 1) && curr_col > 0){ // checks if we're not on the edge of the map
                add_link(graph, node, position, position - 1); // -1 -> moves to the left
                go_left(&position, &curr_col, &direction);
                triangle_type = (curr_row + curr_col) % 2;
                if(exit_check(exit_points, position, count)){
                    links_to_exit++;
                }
                continue;
            }
            if(triangle_type == 0){
                if(!isborder(map, curr_row, curr_col, 3) && curr_row > 0){
                    add_link(graph, node, position, position - (map -> cols));
                    go_up(map, &position, &curr_row, &direction);
                    triangle_type = (curr_row + curr_col) % 2;
                    if(exit_check(exit_points, position, count)){
                        links_to_exit++;
                    }
                    continue;
                }
            }
            else if(triangle_type == 1){
                if(!isborder(map, curr_row, curr_col, 3) && curr_row < (map -> rows) - 1){
                    add_link(graph, node, position, position + (map -> cols));
                    go_down(map, &position, &curr_row, &direction);
                    triangle_type = (curr_row + curr_col) % 2;
                    if(exit_check(exit_points, position, count)){
                        links_to_exit++;
                    }
                    continue;
                }
            }
            else{
                fprintf(stderr, "ERROR: INVALID TRIANGLE TYPE\n");
                return;
            }
            if(!isborder(map, curr_row, curr_col, 2) && curr_col < (map -> cols) - 1){ // checks if we're not on the edge of the map
                add_link(graph, node, position, position + 1);
                go_right(&position, &curr_col, &direction);
                triangle_type = (curr_row + curr_col) % 2;
                if(exit_check(exit_points, position, count)){
                    links_to_exit++;
                }
                continue;
            }
        }
        else if(direction == 2){
            if(!isborder(map, curr_row, curr_col, 2) && curr_col < (map -> cols) - 1){
                add_link(graph, node, position, position + 1);
                go_right(&position, &curr_col, &direction);
                triangle_type = (curr_row + curr_col) % 2;
                if(exit_check(exit_points, position, count)){
                    links_to_exit++;
                }
                continue;
            }
            if(triangle_type == 0){
                if(!isborder(map, curr_row, curr_col, 3) && curr_row > 0){
                    add_link(graph, node, position, position - (map -> cols));
                    go_up(map, &position, &curr_row, &direction);
                    triangle_type = (curr_row + curr_col) % 2;
                    if(exit_check(exit_points, position, count)){
                        links_to_exit++;
                    }
                    continue;
                }
            }
            else if(triangle_type == 1){
                if(!isborder(map, curr_row, curr_col, 3) && curr_row < (map -> rows) - 1){
                    add_link(graph, node, position, position + (map -> cols));
                    go_down(map, &position, &curr_row, &direction);
                    triangle_type = (curr_row + curr_col) % 2;
                    if(exit_check(exit_points, position, count)){
                        links_to_exit++;
                    }
                    continue;
                }
            }
            else{
                fprintf(stderr, "ERROR: INVALID TRIANGLE TYPE\n");
                return;
            }
            if(!isborder(map, curr_row, curr_col, 1) && curr_col > 0){ // checks if we're not on the edge of the map
                add_link(graph, node, position, position - 1); // -1 -> moves to the left
                go_left(&position, &curr_col, &direction);
                triangle_type = (curr_row + curr_col) % 2;
                if(exit_check(exit_points, position, count)){
                    links_to_exit++;
                }
                continue;
            }
        }
        else if(direction == 3 || direction == 4){
            if(!isborder(map, curr_row, curr_col, 1) && curr_col > 0){
                add_link(graph, node, position, position - 1);
                go_left(&position, &curr_col, &direction);
                triangle_type = (curr_row + curr_col) % 2;
                if(exit_check(exit_points, position, count)){
                    links_to_exit++;
                }
                continue;
            }
            if(!isborder(map, curr_row, curr_col, 2) && curr_col < (map -> cols) - 1){
                add_link(graph, node, position, position + 1);
                go_right(&position, &curr_col, &direction);
                triangle_type = (curr_row + curr_col) % 2;
                if(exit_check(exit_points, position, count)){
                    links_to_exit++;
                }
                continue;
            }
            if(triangle_type == 0){
                if(!isborder(map, curr_row, curr_col, 3) && curr_row > 0){
                    add_link(graph, node, position, position - (map -> cols));
                    go_up(map, &position, &curr_row, &direction);
                    triangle_type = (curr_row + curr_col) % 2;
                    if(exit_check(exit_points, position, count)){
                        links_to_exit++;
                    }
                    continue;
                }
            }
            else if(triangle_type == 1){
                if(!isborder(map, curr_row, curr_col, 3) && curr_row < (map -> rows) - 1){
                    add_link(graph, node, position, position + (map -> cols));
                    go_down(map, &position, &curr_row, &direction);
                    triangle_type = (curr_row + curr_col) % 2;
                    if(exit_check(exit_points, position, count)){
                        links_to_exit++;
                    }
                    continue;
                }
            }
        }
        else{
            fprintf(stderr, "ERROR: INVALID DIRECTION\n");
            return;
        }
    }
    bfs(graph, q, node, start_position, size);
    for(int i = 0; i <= size; i++){
        free(graph -> adjacent_node_list[i]);
    }
    free(graph -> adjacent_node_list);
    free(graph -> visited);
    free(graph);
    free(node -> next); // careful here, methodically remove all
    free(node);
    free(q);
    free(exit_points);
}

void create_graph(Graph *graph, int size){

    graph -> index_amount = size;
    graph -> adjacent_node_list = malloc(sizeof(Node *) * size);
    if(graph -> adjacent_node_list == NULL){
        fprintf(stderr, "ERROR: MEMORY ALLOCATION UNSUCCESSFUL\n");
        return;
    }
    for(int i = 0; i < graph -> index_amount; i++){
        graph -> adjacent_node_list[i] = malloc(sizeof(Node));
        if(graph -> adjacent_node_list[i] == NULL){
            fprintf(stderr, "ERROR: MEMORY ALLOCATION UNSUCCESSFUL\n");
            return;
        }
    }
    graph -> visited = malloc(size * sizeof(int));
    if(graph -> visited == NULL){
        fprintf(stderr, "ERROR: MEMORY ALLOCATION UNSUCCESSFUL\n");
        return;
    }

    for (int i = 0; i < size; i++){ // no need to intitialise all indices, if you initialize one, then the rest all becom 0 automatically
        graph -> visited[i] = 0; // boolean map -> 0 = not visited; 1 = visited;
    }
}

void add_link(Graph *graph, Node *new_node, int src_index, int dest_index){ // creates a hypothetical link between two indices
    create_node(graph, new_node, src_index);
    if(src_index == dest_index + 1){ // left adjacent node
        new_node -> next[0] = malloc(sizeof(Node));
        if(new_node -> next[0] == NULL){
            fprintf(stderr, "ERROR: MEMORY ALLOCATION UNSUCCESSFUL!\n");
            return;
        }
        create_node(graph, new_node, dest_index);
        // Add link from dest_index node to src_index node
        graph -> adjacent_node_list[src_index] -> next[0] = graph -> adjacent_node_list[dest_index];
    }
    else if(src_index == dest_index - 1){ // right adjacent node
        new_node -> next[1] = malloc(sizeof(Node));
        if(new_node -> next[1] == NULL){
            fprintf(stderr, "ERROR: MEMORY ALLOCATION UNSUCCESSFUL!\n");
            return;
        }
        create_node(graph, new_node, dest_index);
        // Add link from dest_index node to src_index node
        graph -> adjacent_node_list[src_index] -> next[1] = graph -> adjacent_node_list[dest_index];
    }
    else if(src_index > dest_index + 1 || src_index < dest_index - 1){ // upper or lower adjacent node
        new_node -> next[2] = malloc(sizeof(Node));
        if(new_node -> next[2] == NULL){
            fprintf(stderr, "ERROR: MEMORY ALLOCATION UNSUCCESSFUL!\n");
            return;
        }
        create_node(graph, new_node, dest_index);
        // Add link from dest_index node to src_index node
        graph -> adjacent_node_list[src_index] -> next[2] = graph -> adjacent_node_list[dest_index];
    }
}

void create_node(Graph *graph, Node *new_node, int index){
    new_node = malloc(sizeof(Node));
    if(new_node == NULL){
        fprintf(stderr, "ERROR: MEMORY ALLOCATION UNSUCCESSFUL\n");
        return;
    }
    new_node -> curr_index = index;
    for(int i = 0; i < 3; i++){
        new_node -> next[i] = NULL;
    }
    graph -> adjacent_node_list[index] = new_node;
}

void bfs(Graph *graph, Queue *q, Node *temp, int start_index, int map_size){
    int amount = 0;
    create_queue(&q);
    if(q == NULL){
        return;
    }
    graph -> visited[start_index] = 1;
    enqueue(&q, start_index, map_size, &amount);
    while (!is_empty(q)) {
        print_queue(q);
        int curr_index = dequeue(q);
        printf("Visited %d\n", curr_index);

        temp = graph -> adjacent_node_list[curr_index];

        while(temp){
            int adjcurr_index = temp -> curr_index; //adjacent index to current one

            if (graph -> visited[adjcurr_index] == 0){
                graph -> visited[adjcurr_index] = 1;
                enqueue(&q, adjcurr_index, map_size, &amount);
            }
            temp = temp->next;
        }
    }
}

void create_queue(Queue **q) {
    *q = malloc(sizeof(Queue));
    if(*q == NULL){
        fprintf(stderr, "ERROR: MEMORY ALLOCATION UNSUCCESSFUL\n");
        return;
    }
    (*q) -> front = -1; // index can't be -1 so -1 is used for evaluating the queue
    (*q) -> rear = -1;
}

void enqueue(Queue **q, int curr_index, int map_size, int *amount){
    if ((*q) -> rear == map_size - 1){
        return;
    }
    else{
        if((*q) -> front == -1){
            (*q) -> front = curr_index;
        }
        (*q) -> rear = curr_index;
        int *new = realloc((*q) -> items, sizeof(int) * (*amount));
        if(new == NULL){
            fprintf(stderr, "ERROR: MEMORY ALLOCATION UNSUCCESSFUL\n");
            return;
        }
        (*q) -> items = new;
        (*q) -> items[*amount] = curr_index; //??? marks the current index as not visited (not right away, eventually, when it gets to the front) ???
        (*amount)++;
    }
}

void print_queue(Queue *q){
    if(is_empty(q)){
        return;
    }
    else{
        for (int i = 0; q -> items[i] != q -> rear; i++) {
            printf("%d ", q -> items[i]);
        }
        printf("%d ", q -> rear);
    }
}

int is_empty(Queue *q) {
    if (q -> rear == -1){
        return 1;
    }
    else{
        return 0;
    }
}

int dequeue(Queue *q) {
    int item;

    if(is_empty(q)){
        item = -1;
    }
    else{
        item = q -> front;
        q -> front = q -> items[1];
        for(int i = 0; q -> items[i] != q -> rear; i++){
            q -> items[i] = q -> items[i + 1];
        }
// =====================--UNSURE HERE--============================ //
        if((q -> items[0]) == (q -> rear)){ // watch out here
            q -> front = q -> rear = -1; // resets the queue
        }
    }
  return item;
}

int add_exit(unsigned char **exit_points, int count, int position){
    unsigned char *new_exit;
    new_exit = NULL;
    if((*exit_points) == NULL){
        *exit_points = malloc(sizeof(unsigned char));
        if((*exit_points) == NULL){
            fprintf(stderr, "ERROR: MEMORY ALLOCATION UNSUCCESSFUL\n");
            return 1;
        }
        (*exit_points)[count] = position;
        return 0;
    }
    if(exit_check((*exit_points), position, count)){
        return 1;
    }
    new_exit = realloc((*exit_points), sizeof(unsigned char) + (count * sizeof(unsigned char)));
    if(new_exit == NULL){
        fprintf(stderr, "ERROR MEMORY ALLOCATION UNSUCCESSFUL\n");
        return 1;
    }
    (*exit_points) = new_exit;
    (*exit_points)[count] = position;
    return 0;
}

int exit_check(unsigned char *exit_points, int position, int count){ //Probably done
    for(int i = 0; i < count; i++){
        if(position == exit_points[i]){
            return 1;
        }
    }
    return 0;
}

void temp(Graph *graph, int start, int *exit_points, int amount_of_exits){
    for(int i = 0; i < amount_of_exits; i++){ //changes which exit point we start at
        graph -> adjacent_node_list[exit_points[i]]; // the exit point we start at
    }
}
// ***