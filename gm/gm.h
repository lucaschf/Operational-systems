//
// Created by lucas on 05/04/2021.
//

#ifndef GM_GM_H
#define GM_GM_H

#define TASK_LOGICAL_MEMORY 4096 // bytes - 4K
#define PAGE_SIZE 512 // bytes
#define VAR_NAME_LENGTH 20
#define INSTRUCTION_LENGTH 50
#define SYSTEM_MEMORY 20480 // bytes
#define TASK_SIZE_DELIMITER "#T="
#define KEY_NEW "new"

typedef struct {
    int page;
    int address;
} page_address;

typedef struct Allocation {
    char var_name[VAR_NAME_LENGTH];
    unsigned short logic_page;
    unsigned short start_pos;
    unsigned short index;
    unsigned int size;
    struct Allocation *next;
    struct Allocation *prev;
} Allocation;

typedef struct {
    const struct Allocation *allocation;
    int accessed_index;
    page_address logic_page;
    page_address physical_page;
} memory_address;

typedef struct {
    char name[INSTRUCTION_LENGTH];
    unsigned short cpu_time;
    unsigned int size;
    struct Allocation *first;
    struct Allocation *last;
    memory_address accesses[PAGE_SIZE];
    int memory_accesses;
    int aborted;
} Task;

int ends_with(const char *str, const char *end);

int allocate_memory(const char *var_name, size_t size, Task *task);

memory_address new_memory_address(
        const Allocation *allocation,
        int pos,
        int logic_page,
        int logic_address,
        int physical_page,
        int physical_address
);

int access_memory(
        const char *var_name,
        size_t index,
        const Task *task,
        memory_address *address
);

#endif //GM_GM_H