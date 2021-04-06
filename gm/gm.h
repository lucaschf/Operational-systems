//
// Created by lucas on 05/04/2021.
//

#ifndef GM_GM_H
#define GM_GM_H

#define TASK_LOGICAL_MEMORY 4096 // bytes - 4K
#define PAGE_SIZE 512 // bytes
#define VAR_NAME_LENGTH 50 // bytes

typedef struct {
    unsigned short cpu_time;
    unsigned int size;
    struct Allocation *first;
    struct Allocation *last;
} Task;

typedef struct Allocation {
    char var_name[VAR_NAME_LENGTH];
    unsigned short logic_page;
    unsigned short start_pos;
    unsigned short index;
    unsigned int size;
    struct Allocation *next;
    struct Allocation *prev;
} Allocation;

int ends_with(const char *str, const char *end);

int allocate_memory(const char *var_name, size_t size, Task *task);

int access_memory(const char *var_name, size_t size, const Task *task);

#endif //GM_GM_H
