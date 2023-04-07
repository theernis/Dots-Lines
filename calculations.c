#include <Python.h>
#include <Math.h>
#include <stdbool.h>
#include <time.h>
#include "necessities.h"


int dis_width = 400;
int dis_height = 400;
bool stop_overlapping = false;
double view_distance = 40;

double start_time = 0;
double now_time = 0;
int frames = 0;

//generates dots
void generate_dots() {
    for (int i = 0; i < dots_size; i++) {
        Dot temp_dot;
        temp_dot.position.x = random() * dis_width;
        temp_dot.position.y = random() * dis_height;
        temp_dot.velocity.x = random() * 2 - 1;
        temp_dot.velocity.y = random() * 2 - 1;
        double temp_val = sqrt(pow(temp_dot.velocity.x, 2) + pow(temp_dot.velocity.y, 2));
        temp_dot.velocity.x /= temp_val;
        temp_dot.velocity.y /= temp_val;

        int cell_loc = (int)(floor(temp_dot.position.y / cell_size.y) * cell_x_amount + floor(temp_dot.position.x / cell_size.x));
        int dot_loc = dot_grid[cell_loc].current;
        dot_grid[cell_loc].dots[dot_loc] = temp_dot;
        dot_grid[cell_loc].current++;
    }
    return;
}

//allocates grid
void allocate_dot_grid(DotCell *temp_grid) {
    for (int i = 0; i < grid_capacity; i++)
    {
        temp_grid[i].dots = (Dot*)malloc(sizeof(Dot) * dot_cell_capacity);
        temp_grid[i].current = 0;
    }
}

//frees grid from memory
void free_dot_grid(DotCell *temp_grid) {
    if (temp_grid != NULL) {
        for (int i = 0; i < grid_capacity; i++)
        {
            free(temp_grid[i].dots);
            temp_grid[i].dots = NULL;
        }
        free(temp_grid);
        temp_grid = NULL;
    }
}

//sets a_grid values to B_grid values
void set_dot_grid(DotCell *a_grid, DotCell *b_grid) {
    for (int cell_loc = 0; cell_loc < grid_capacity; cell_loc++)
    {
        for (int dot_loc = 0; dot_loc < ((b_grid[cell_loc].current > a_grid[cell_loc].current) ? b_grid[cell_loc].current : a_grid[cell_loc].current); dot_loc++)
        {
            a_grid[cell_loc].dots[dot_loc] = b_grid[cell_loc].dots[dot_loc];
        }
        a_grid[cell_loc].current = b_grid[cell_loc].current;
    }
}

//allocates grid
void allocate_line_grid(LineCell *temp_grid) {
    for (int i = 0; i < grid_capacity; i++)
    {
        temp_grid[i].lines = (Line*)malloc(sizeof(Line) * line_cell_capacity);
        temp_grid[i].current = 0;
    }
}

//frees grid from memory
void free_line_grid(LineCell *temp_grid) {
    if (temp_grid != NULL) {
        for (int i = 0; i < grid_capacity; i++)
        {
            free(temp_grid[i].lines);
            temp_grid[i].lines = NULL;
        }
        free(temp_grid);
        temp_grid = NULL;
    }
}

//sets important values
static PyObject*
set_values(PyObject* self, PyObject* args) {
    int width;
    int height;
    bool overlapping;
    if (!PyArg_ParseTuple(args, "iip", &width, &height, &overlapping)) {
        return NULL;
    }
    dis_width = width;
    dis_height = height;

    stop_overlapping = overlapping;

    dots_size = (int)(sqrt(dis_width * dis_height));

    view_distance = sqrt(dots_size)*2;

    cell_x_amount = max((int)floor(dis_width/view_distance), 1);
    cell_y_amount = max((int)floor(dis_height/view_distance), 1);
    grid_capacity = cell_x_amount * cell_y_amount;
    cell_size.x = dis_width / (double)cell_x_amount;
    cell_size.y = dis_height / (double)cell_y_amount;
    dot_cell_capacity = (int)ceil(dots_size * dots_size / (double)(grid_capacity));
    dot_grid = (DotCell*)malloc(sizeof(DotCell) * grid_capacity);
    if (stop_overlapping) {
        line_cell_capacity = dot_cell_capacity * 7;
    }
    allocate_dot_grid(dot_grid);

    generate_dots();

    start_time = TIME;
    now_time = start_time;
    frames = 0;

    Py_RETURN_NONE;
}

//calculates dots' position
static PyObject*
calculate_dots() {
    PyObject* temp = PyList_New(0);
    DotCell *temp_grid;
    temp_grid = (DotCell*)malloc(sizeof(DotCell) *grid_capacity);
    allocate_dot_grid(temp_grid);

    for (int cell_loc1 = 0; cell_loc1 < grid_capacity; cell_loc1++)
    {
        for (int dot_loc1 = 0; dot_loc1 < dot_grid[cell_loc1].current; dot_loc1++)
        {
            Dot temp_dot = dot_grid[cell_loc1].dots[dot_loc1];
            temp_dot.position.x = wrap(temp_dot.position.x + temp_dot.velocity.x, 0, dis_width);
            temp_dot.position.y = wrap(temp_dot.position.y + temp_dot.velocity.y, 0, dis_height);
            PyObject* obj = Py_BuildValue("(f f)", temp_dot.position.x, temp_dot.position.y);
            PyList_Append(temp, obj);
            Py_DECREF(obj);

            int cell_loc2 = (int)(floor(temp_dot.position.y / cell_size.y) * cell_x_amount + floor(temp_dot.position.x / cell_size.x));
            int dot_loc2 = temp_grid[cell_loc2].current;
            temp_grid[cell_loc2].dots[dot_loc2] = temp_dot;
            temp_grid[cell_loc2].current++;
        }
    }
    set_dot_grid(dot_grid, temp_grid);
    free_dot_grid(temp_grid);
    return temp;
}

//check for overlapping
bool check_overlapping(Pos pos1, Pos pos2) {
    double x1 = pos1.x;
    double x2 = pos2.x;
    double y1 = pos1.y;
    double y2 = pos2.y;

    double minx = min(pos1.x, pos2.x);
    double miny = min(pos1.y, pos2.y);
    int cell_loc = (int)(floor(miny / cell_size.y) * cell_x_amount + floor(minx / cell_size.x));
    for (int x = -1; x <= 1; x++)
    {
        if (cell_loc % cell_x_amount + x >= cell_x_amount || cell_loc % cell_x_amount + x < 0)
        {
            continue;
        }
        for (int y = -1; y <= 1; y++)
        {
            int temp_cell_loc = cell_loc + x + y * cell_x_amount;
            if (temp_cell_loc < 0 || temp_cell_loc >= grid_capacity)
            {
                continue;
            }
            for (int line_loc = 0; line_loc < line_grid[temp_cell_loc].current; line_loc++)
            {

                Line line = line_grid[temp_cell_loc].lines[line_loc];
                double x3 = line.a.x;
                double x4 = line.b.x;
                double y3 = line.a.y;
                double y4 = line.b.y;
                
                //cant explain how but it just works so dont touch it
                bool overlaps = ((
                    0
                    < (x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)
                    ) && (
                    (x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)
                    < (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4)
                    ) && (
                    0
                    < (x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2)
                    ) && (
                    (x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2)
                    < (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4)
                    ) || (
                    0
                    > (x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)
                    ) && (
                    (x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)
                    > (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4)
                    ) && (
                    0
                    > (x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2)
                    ) && (
                    (x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2)
                    > (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4)
                    ));
            
                if (overlaps) {
                    return true;
                }
            }
        }
    }

    return false;
}

//creates list of lines
static PyObject*
calculate_lines() {
    PyObject* py_lines = PyList_New(0);

    Pos pos1;
    Pos pos2;

    if (stop_overlapping) {
        line_grid = (LineCell*)malloc(sizeof(LineCell) * grid_capacity);
        allocate_line_grid(line_grid);
    }

    for (int cell_loc = 0; cell_loc < grid_capacity; cell_loc++)
    {
        for (int dot_loc1 = 0; dot_loc1 < dot_grid[cell_loc].current; dot_loc1++)
        {
            pos1 = dot_grid[cell_loc].dots[dot_loc1].position;
            for (int x = 0; x < 2; x++)
            {
                if (cell_loc % cell_x_amount + x >= cell_x_amount)
                {
                    continue;
                }
                for (int y = 0; y < 2; y++)
                {
                    int temp_cell_loc = cell_loc + x + y * cell_x_amount;
                    if (temp_cell_loc >= grid_capacity)
                    {
                        continue;
                    }

                    for (int dot_loc2 = 0; dot_loc2 < dot_grid[temp_cell_loc].current; dot_loc2++)
                    {
                        pos2 = dot_grid[temp_cell_loc].dots[dot_loc2].position;

                        double distance = sqrt(pow((pos1.x - pos2.x), 2) + pow((pos1.y - pos2.y), 2));
                        if (distance > view_distance) {
                            continue;
                        }

                        //check for overlapping
                        bool overlaps = (stop_overlapping && check_overlapping(pos1, pos2));

                        if (!overlaps) {
                            PyObject* temp1 = Py_BuildValue("(f f)", pos1.x, pos1.y);
                            PyObject* temp2 = Py_BuildValue("(f f)", pos2.x, pos2.y);
                            PyObject* temp3 = Py_BuildValue("[O O i]", temp1, temp2, clamp(0, 255, (int)(sqrt(1 - pow((distance / view_distance), 2)) * 255)));
                            PyList_Append(py_lines, temp3);
                            Py_DECREF(temp1);
                            Py_DECREF(temp2);
                            Py_DECREF(temp3);
                            if (stop_overlapping) {
                                double minx = min(pos1.x, pos2.x);
                                double miny = min(pos1.y, pos2.y);
                                int line_cell_loc = (int)(floor(miny / cell_size.y) * cell_x_amount + floor(minx / cell_size.x));
                                int line_loc = line_grid[line_cell_loc].current;
                                line_grid[line_cell_loc].lines[line_loc].a = pos1;
                                line_grid[line_cell_loc].lines[line_loc].b = pos2;
                                line_grid[line_cell_loc].current++;
                            }
                        }
                    }
                }
            }
        }
    }
    if (stop_overlapping) {
        free_line_grid(line_grid);
    }
    return py_lines;
}

//fps counter
static PyObject*
display_fps() {
    double temp = TIME;
    printf("%f fps", 1 / (temp - now_time));
    now_time = temp;
    frames++;
    printf(", average - %f fps\n", frames / (temp - start_time));

    Py_RETURN_NONE;
}

//evaluate Dot value for camparing
double dot_eval(Dot a) {
    return max(a.position.x / dis_width, a.position.y / dis_height);
}

void dot_merge_sort(Dot *list, int start, int end) {
    if (start == end) {
        return;
    }
    int split = (start + end) / 2;
    dot_merge_sort(list, start, split);
    dot_merge_sort(list, split + 1, end);

    //merge
    Dot* temp = malloc(sizeof(Dot) * (end - start + 1));
    int counter_a = 0;
    int counter_b = 0;
    for (int i = 0; i < (end - start + 1); i++)
    {
        if (split + 1 + counter_b <= end && start + counter_a <= split)
        {
            if (dot_eval(list[start + counter_a]) < dot_eval(list[split + 1 + counter_b]))
            {
                temp[i] = list[start + counter_a];
                counter_a++;
            }
            else
            {
                temp[i] = list[split + 1 + counter_b];
                counter_b++;
            }
        }
        else
        {
            if (start + counter_a <= split)
            {
                temp[i] = list[start + counter_a];
                counter_a++;
            }
            else
            {
                temp[i] = list[split + 1 + counter_b];
                counter_b++;
            }
        }
    }
    for (int i = 0; i < (end - start + 1); i++)
    {
        list[start + i] = temp[i];
    }
    free(temp);
}

//resizes grid and changes other values
static PyObject*
resize(PyObject* self, PyObject* args) {
    int width;
    int height;
    if (!PyArg_ParseTuple(args, "ii", &width, &height)) {
        return NULL;
    }
    printf("%i %i\n", width, height);

    //temporary values
    int old_dis_width = dis_width;
    int old_dis_height = dis_height;
    int old_dots_size = dots_size;
    int old_grid_capacity = grid_capacity;

    //get dot list from dot_grid
    Dot *dot_list = (Dot*)malloc(sizeof(Dot) * old_dots_size);
    int counter = 0;
    for (int cellloc = 0; cellloc < old_grid_capacity; cellloc++)
    {
        for (int dotloc = 0; dotloc < dot_grid[cellloc].current; dotloc++)
        {
            dot_list[counter] = dot_grid[cellloc].dots[dotloc];
            counter++;
        }
    }

    free_dot_grid(dot_grid);
    dis_width = width;
    dis_height = height;

    dots_size = (int)(sqrt(dis_width * dis_height));

    view_distance = sqrt(dots_size) * 2;

    cell_x_amount = max((int)floor(dis_width / view_distance), 1);
    cell_y_amount = max((int)floor(dis_height / view_distance), 1);
    grid_capacity = cell_x_amount * cell_y_amount;
    cell_size.x = dis_width / (double)cell_x_amount;
    cell_size.y = dis_height / (double)cell_y_amount;
    dot_cell_capacity = (int)ceil(dots_size * dots_size / (double)(grid_capacity));
    dot_grid = (DotCell*)malloc(sizeof(DotCell) * grid_capacity);
    if (stop_overlapping) {
        line_cell_capacity = dot_cell_capacity * 7;
    }
    allocate_dot_grid(dot_grid);

    //sort dot list
    //make it only sort when making window smaller
    if (old_dots_size > dots_size)
    {
        dot_merge_sort(dot_list, 0, old_dots_size - 1);
    }

    //fill new dot grid with old dots
    //if new dots_size is less than old
    //fill in only new dots_size amount
    //if new dots_size is more than old
    //generate new dots to fill in space
    for (int i = 0; i < dots_size; i++)
    {
        Dot temp_dot;
        if (i < old_dots_size)
        {
            temp_dot = dot_list[i];
            temp_dot.position.x = wrap(temp_dot.position.x + temp_dot.velocity.x, 0, dis_width);
            temp_dot.position.y = wrap(temp_dot.position.y + temp_dot.velocity.y, 0, dis_height);
        }
        else
        {
            if ((random() > .5 && old_dis_height < dis_height) || old_dis_width >= dis_width)
            {
                temp_dot.position.x = random() * dis_width;
                temp_dot.position.y = old_dis_height + random() * (dis_height - old_dis_height);
            }
            else
            {
                temp_dot.position.x = old_dis_width + random() * (dis_width - old_dis_width);
                temp_dot.position.y = random() * dis_height;
            }
            temp_dot.velocity.x = random() * 2 - 1;
            temp_dot.velocity.y = random() * 2 - 1;
            double temp_val = sqrt(pow(temp_dot.velocity.x, 2) + pow(temp_dot.velocity.y, 2));
            temp_dot.velocity.x /= temp_val;
            temp_dot.velocity.y /= temp_val;
        }
        int cell_loc = (int)(floor(temp_dot.position.y / cell_size.y) * cell_x_amount + floor(temp_dot.position.x / cell_size.x));
        int dot_loc = dot_grid[cell_loc].current;
        dot_grid[cell_loc].dots[dot_loc] = temp_dot;
        dot_grid[cell_loc].current++;
    }


    free(dot_list);

    start_time = TIME;
    now_time = start_time;
    frames = 0;

    Py_RETURN_NONE;
}

static PyMethodDef SomeMethods[] = {
    {"set_values", set_values, METH_VARARGS, NULL},
    {"calculate_dots", calculate_dots, METH_NOARGS, NULL},
    {"calculate_lines", calculate_lines, METH_NOARGS, NULL},
    {"display_fps", display_fps, METH_NOARGS, NULL},
    {"resize", resize, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}
};


static struct PyModuleDef calculations = {
    PyModuleDef_HEAD_INIT,
    "calculations",
    "Some lib",
    -1,
    SomeMethods
};


PyMODINIT_FUNC PyInit_calculations(void) {
    return PyModule_Create(&calculations);
}