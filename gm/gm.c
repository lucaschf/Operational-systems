#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gm.h"

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

void arrange_allocated_pointers(Allocation *allocation, Task *task) {
    if (!task->first) {
        allocation->next = NULL;
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

    if (task->size + size > TASK_LOGICAL_MEMORY)
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
        arrange_allocated_pointers(allocation, task);

        if (size > PAGE_SIZE) { // split in more pages
            return allocate_memory(var_name, size - PAGE_SIZE, task);
        }

        return 1;
    }

    allocation = (Allocation *) malloc(sizeof(Allocation));
    if (!allocation)
        return 0;

    Allocation *last = task->last;
    arrange_allocated_pointers(allocation, task);

    // iterates over previous allocations summing allocation sizes
    Allocation *iterator = last;
    char previous_var[VAR_NAME_LENGTH] = "";
    strcpy(previous_var, last->var_name);
    size_t last_page_size = iterator->size;

    while (iterator && iterator->logic_page == last->logic_page) {
        if (strcmp(iterator->var_name, previous_var) != 0) {
            last_page_size += iterator->size - iterator->start_pos;
        }

        strcpy(previous_var, iterator->var_name);
        iterator = iterator->prev;
    }

    // this is a page split
    if (!strcmp(last->var_name, var_name)) {
        allocation->size = last->size;
        allocation->start_pos = allocation->size - size;
    } else {
        allocation->size = size;
        allocation->start_pos = 0;
        task->size += size;
    }

    strcpy(allocation->var_name, var_name);
    size_t available = PAGE_SIZE - last_page_size;

    if (available == 0) {
        available = PAGE_SIZE;
    }

    allocation->logic_page = PAGE_SIZE <= last_page_size ? last->logic_page + 1 : last->logic_page;
    allocation->index = allocation->logic_page * PAGE_SIZE + (PAGE_SIZE - available);

    if (available < size) {
        size_t remains = size - available;
        return allocate_memory(var_name, remains, task);
    }

    return 1;
}

int access_memory(const char *var_name, size_t position, const Task *task) {
    if (!var_name || !task->first)
        return 0;

    Allocation *iterator = task->first;

    while (iterator) {
        if (!strcmp(var_name, iterator->var_name)) {
            if (position >= iterator->size)
                return -1; // Access violation

            if ((PAGE_SIZE - iterator->start_pos) - 1 >= position) {
                int physical_page = SYSTEM_MEMORY / PAGE_SIZE + iterator->logic_page;
                int physical_address = SYSTEM_MEMORY + (iterator->logic_page * PAGE_SIZE) + iterator->index + position;
                printf("\nPhysical address: %d: %d", physical_page, physical_address);

                return (int) (iterator->start_pos + position + iterator->index);
            }
        }

        iterator = iterator->next;
    }

    return 0;
}

memory_address new_memory_address(int logic_page, int logic_address, int physical_page, int physical_address) {
    memory_address result;
    result.logic_page.page = logic_page;
    result.logic_page.address = logic_address;
    result.physical_page.page = physical_page;
    result.physical_page.address = physical_address;

    return result;
}

memory_address access_memory2(const char *var_name, size_t position, const Task *task) {
    memory_address access_violation = new_memory_address(-1, -1, -1, -1);

    if (!var_name || !task->first) {
        return access_violation;
    }

    Allocation *iterator = task->first;

    while (iterator) {
        if (!strcmp(var_name, iterator->var_name)) {
            if (position >= iterator->size)
                return access_violation; // Access violation

            if ((PAGE_SIZE - iterator->start_pos) - 1 >= position) {
                int physical_page = SYSTEM_MEMORY / PAGE_SIZE + iterator->logic_page;
                int physical_address = SYSTEM_MEMORY + (iterator->logic_page * PAGE_SIZE) + iterator->index + position;

                return new_memory_address(
                        iterator->logic_page,
                        (int) (iterator->index + position),
                        physical_page,
                        physical_address
                );
            }
        }

        iterator = iterator->next;
    }

    return access_violation;
}

void show_address(memory_address address) {
    if(address.logic_page.page == -1){
        printf("\nAcesso invalido a memoria\n");
        return;
    }

    printf("\nEndereco logico = %d: %d", address.logic_page.page, address.logic_page.address);
    printf("\nEndereco fisico = %d: %d\n", address.physical_page.page, address.physical_page.address);
}

int main(int argc, char **argv) {
    Task t;
    init_task(&t, 150);

    allocate_memory("v", 100, &t);
    allocate_memory("v2", 50, &t);
    allocate_memory("v3", 300, &t);
    allocate_memory("v4", 100, &t);

    show_address(access_memory2("v", 10, &t));
    show_address(access_memory2("v2", 15, &t));
    show_address(access_memory2("v3", 0, &t));
    show_address(access_memory2("v3", 300, &t));
    show_address(access_memory2("v4", 0, &t));

    printf("Last page: %d", t.last->logic_page);

    return 0;
}
