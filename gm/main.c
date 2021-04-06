#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gm.h"

int cont = 0;

//region utils
int ends_with(const char *str, const char *end) {
    return strcmp(str + strlen(str) - strlen(end), end) == 0;
}

void remove_newline(char *str) {
    if (!str)
        return;

    size_t length = strlen(str);

    if (str[length - 1] == '\n' || str[length - 1] == '\r')
        str[length - 1] = '\0';
}

FILE *open_file(char *file_name) {
    const char *ext = ".tsk";

    char name[50];

    strcpy(name, file_name);
    remove_newline(name);

    if (!ends_with(name, ext))
        strcat(name, ext);

    FILE *f = fopen(name, "r");

    return f;
}
//endregion

int init_task(Task *t, size_t size) {
    t->size = size;
    t->cpu_time = 0;

    t->last = NULL;
    t->first = NULL;

    return allocate_memory("", size, t);
}

void manage_pointer(Allocation *allocation, Task *task) {

    if (!task->first) {
        allocation->next = allocation;
        task->first = allocation;

        task->last = allocation;
        task->last->prev = NULL;
        task->last->next = NULL;
        return;
    }

    allocation->next = NULL;
    allocation->prev = task->last;
    task->first->prev = NULL;
    task->last->next = allocation;
    task->last = allocation;
}

int allocate_memory(const char *var_name, size_t size, Task *task) {

    Allocation *allocation;

    if (task->size + size >= TASK_LOGICAL_MEMORY)
        return 0; // no task memory

    // assuming is first allocation (code)
    if (!strcmp("", var_name) && !task->last) {
        allocation = (Allocation *) malloc(sizeof(Allocation));

        if (!allocation)
            return 0;

        allocation->start_pos = 0;
        allocation->index = 0;
        allocation->logic_page = 0; // single task program means always starting on first page
        allocation->size = size;
        strcpy(allocation->var_name, var_name);
        manage_pointer(allocation, task);

        if (size > PAGE_SIZE) {
            return allocate_memory(var_name, size - PAGE_SIZE, task);
        }
        return 1;
    }

    allocation = (Allocation *) malloc(sizeof(Allocation));
    if (!allocation)
        return 0;

    Allocation *last = task->last;
    Allocation *iterator = last;

    char previous_var[VAR_NAME_LENGTH] = "";
    strcpy(previous_var, last->var_name);
    size_t last_page_size = iterator->size;

    while (iterator && iterator->logic_page == last->logic_page) {
        if (strcmp(iterator->var_name, previous_var) != 0) {
            last_page_size += iterator->size;
        }

        strcpy(previous_var, iterator->var_name);
        iterator = iterator->prev;
    }

    manage_pointer(allocation, task);

    allocation->logic_page = last_page_size >= PAGE_SIZE ? last->logic_page + 1 : last->logic_page;

    if (strcmp(last->var_name, var_name) != 0) {
        allocation->size = size;
        task->size += size;
    } else
        allocation->size = task->last->size;

    strcpy(allocation->var_name, var_name);
    size_t available = PAGE_SIZE - last_page_size;
    allocation->
            index = PAGE_SIZE - available;

    if (available < size) {
        size_t remaining = size - available;
        return allocate_memory(var_name, remaining, task);
    }

    return 1;
}

int main(int argc, char **argv) {
    Task t;
    init_task(&t, 150);

    allocate_memory("v", 363, &t);
    return 0;
}
