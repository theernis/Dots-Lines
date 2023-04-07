#pragma once
#include <time.h>

//time since process starts
#define TIME ((double)clock() / CLK_TCK)

// random value between 1 and 0
#define random() ((double)rand() / (double)RAND_MAX)


typedef struct {
    double x;
    double y;
} Pos;

typedef struct {
    Pos position;
    Pos velocity;
} Dot;
int dots_size;
//Dot *dots;

typedef struct {
    Pos a;
    Pos b;
} Line;

Pos cell_size;
int cell_x_amount;
int cell_y_amount;
int grid_capacity;

typedef struct {
    int current;
    Dot *dots;
} DotCell;
int dot_cell_capacity;
DotCell *dot_grid;

typedef struct {
    int current;
    Line *lines;
} LineCell;
int line_cell_capacity;
LineCell *line_grid;


//clamps a value
static inline int clamp(int min_num, int max_num, int num) {
    return (num < min_num) ? min_num : ((num > max_num) ? max_num : num);
}

//wraps a value
static inline double wrap(double num, double range_a, double range_b) {
    /*double num_min = (range_a < range_b) ? range_a : range_b;
    double num_max = (range_a > range_b) ? range_a : range_b;
    double range = num_max - num_min;
    return (num < num_min) ? num + range * ceil((num_min - num) / range) : ((num > num_max) ? num - range * ceil((num - num_max) / range) : num);*/
    return (num < ((range_a < range_b) ? range_a : range_b)) ? num + (((range_a > range_b) ? range_a : range_b) - ((range_a < range_b) ? range_a : range_b)) * ceil((((range_a < range_b) ? range_a : range_b) - num) / (((range_a > range_b) ? range_a : range_b) - ((range_a < range_b) ? range_a : range_b))) : ((num > ((range_a > range_b) ? range_a : range_b)) ? num - (((range_a > range_b) ? range_a : range_b) - ((range_a < range_b) ? range_a : range_b)) * ceil((num - ((range_a > range_b) ? range_a : range_b)) / (((range_a > range_b) ? range_a : range_b) - ((range_a < range_b) ? range_a : range_b))) : num);
}