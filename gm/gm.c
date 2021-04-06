#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gm.h"

void run(int argc, char **argv);

void execute_instructions(Task *task, FILE *instructions_file);

int execute_allocation(char instruction[50], Task *task);

int execute_access(char instruction[50], Task *task);

void show_pages_ranges(Task task);

void show_allocation_ranges(Task task);

void show_allocation_index(Task task);

void show_address_interval(memory_address start, memory_address anEnd);

//region utils
int ends_with(const char *str, const char *end) {
    return strcmp(str + strlen(str) - strlen(end), end) == 0;
}

int starts_with(const char *str, const char *prefix) {
    return strncmp(prefix, str, strlen(prefix)) == 0;
}

void remove_newline(char *str) {
    if (!str)
        return;

    size_t length = strlen(str);

    if (str[length - 1] == '\n' || str[length - 1] == '\r')
        str[length - 1] = '\0';
}

int contains(const char *str, const char *another) {
    return strstr(str, another) != NULL;
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
    t->memory_accesses = 0;

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
    size_t available = last_page_size < PAGE_SIZE ? PAGE_SIZE - last_page_size : PAGE_SIZE;

    allocation->logic_page = PAGE_SIZE <= last_page_size ? last->logic_page + 1 : last->logic_page;
    allocation->index = allocation->logic_page * PAGE_SIZE + (PAGE_SIZE - available);

    if (available < size) {
        size_t remains = size - available;
        return allocate_memory(var_name, remains, task);
    }

    return 1;
}

memory_address new_memory_address(
        const Allocation *allocation,
        int pos,
        int logic_page,
        int logic_address,
        int physical_page,
        int physical_address
) {
    memory_address result;
    result.allocation = allocation;
    result.accessed_index = pos;
    result.logic_page.page = logic_page;
    result.logic_page.address = logic_address;
    result.physical_page.page = physical_page;
    result.physical_page.address = physical_address;

    return result;
}

int access_memory(const char *var_name, size_t index, const Task *task, memory_address *address) {
    memory_address access_violation = new_memory_address(
            NULL,
            -1,
            -1,
            -1,
            -1,
            -1);
    *address = access_violation;

    if (!var_name || !task->first) {
        return 0; // no variable
    }

    Allocation *iterator = task->first;

    while (iterator) {
//        if(address->allocation && strcmp(address->allocation->var_name, iterator->var_name) != 0)
//            *address = access_violation;

        if (!strcmp(var_name, iterator->var_name)) {

            size_t last_index = (PAGE_SIZE * (iterator->logic_page + 1)) - 1;

            if (last_index + iterator->start_pos >= index) {

                int physical_page = (SYSTEM_MEMORY / PAGE_SIZE) + iterator->logic_page;
                int physical_address =
                        SYSTEM_MEMORY + iterator->index + index - iterator->start_pos - 1;


                *address = new_memory_address(
                        iterator,
                        index,
                        iterator->logic_page,
                        (int) ((iterator->index + index - iterator->start_pos) - iterator->logic_page * PAGE_SIZE) - 1,
                        physical_page,
                        physical_address
                );

                return 1;
            }
        }

        iterator = iterator->next;
    }

    // no var found
    return 0;
}

void free_up_memory(Task *task) {
    if (!task->first)
        return;

    Allocation *al = task->first;

    while (task->first) {
        task->first = task->first->next;
        free(al);
        al = task->first;
    }

    task->first = NULL;
    task->last = NULL;
}

int get_task_size(FILE *f) {
    const size_t length = 50;
    char line[length];

    rewind(f);

    if (!fgets(line, length, f) || !starts_with(line, TASK_SIZE_DELIMITER)) {
        return -1;
    }

    char *token = strtok(line, TASK_SIZE_DELIMITER);

    if (token) {
        return atoi(token); // NOLINT(cert-err34-c)
    }

    return -1;
}

void run(int argc, char **argv) {

    if (argc != 2) {
        printf("\nSintaxe incorreta");
        exit(EXIT_FAILURE);
    }

    Task task;
    FILE *instructions_file = open_file(argv[1]);

    if (!instructions_file) {
        printf("\nFalha ao abrir arquivo de instruções");
        exit(EXIT_FAILURE);
    }

    int task_size = get_task_size(instructions_file);

    if (task_size == -1) {
        printf("\nLayout de arquivo incorreto");
        fclose(instructions_file);
        exit(EXIT_FAILURE);
    }

    strcpy(task.name, argv[1]);
    if (init_task(&task, task_size) != 1) {
        printf("\nFalha ao inicializar tarefa");
        fclose(instructions_file);
        exit(EXIT_FAILURE);
    }

    execute_instructions(&task, instructions_file);

    printf(">> Tempo de cpu: %d", task.cpu_time);
    printf("\n>> Tamanho final da tarefa: %d bytes", task.size);
    printf("\n>> Numero de paginas logicas: %d", task.last ? task.last->logic_page + 1 : 0);
    show_allocation_ranges(task);
    show_allocation_index(task);
    show_pages_ranges(task);

    free_up_memory(&task);
    fclose(instructions_file);
}

void show_allocation_ranges(Task task) {

    Allocation *iterator = task.first;

    printf("\n\n>> Faixa de enderecos de alocacao:\n");

    memory_address start;
    memory_address end;

    while (iterator) {
        // task code
        if (!strcmp(iterator->var_name, ""))
            iterator = iterator->next;

        size_t last_index = (PAGE_SIZE * (iterator->logic_page + 1)) - 1;

        if (iterator->start_pos == 0) {
            start.logic_page.page = iterator->logic_page;
            start.logic_page.address = (int) (iterator->index - iterator->logic_page * PAGE_SIZE);

            start.physical_page.page = (SYSTEM_MEMORY / PAGE_SIZE) + iterator->logic_page;
            start.physical_page.address = (int) (SYSTEM_MEMORY + (iterator->index - iterator->logic_page * PAGE_SIZE));
        }

        size_t end_index = (iterator->size - iterator->start_pos) + iterator->index;
        if (end_index <= last_index) {
            end.logic_page.page = iterator->logic_page;
            end.logic_page.address =
                    (int) ((iterator->index + iterator->size - iterator->start_pos) -
                           iterator->logic_page * PAGE_SIZE) - 1;
            end.physical_page.page = (SYSTEM_MEMORY / PAGE_SIZE) + iterator->logic_page;
            end.physical_page.address =
                    SYSTEM_MEMORY + iterator->index + iterator->size - iterator->start_pos - 1;

            printf("\n\t%s[%d]\n", iterator->var_name, iterator->size);
            show_address_interval(start, end);
        }

        iterator = iterator->next;
    }
}

void show_address_interval(memory_address start, memory_address anEnd) {
    printf("\tEnderecos logicos = (%d : %d) a (%d : %d)\n",
           start.logic_page.page,
           start.logic_page.address,
           anEnd.logic_page.page,
           anEnd.logic_page.address
    );
    printf("\tEnderecos fisicos = (%d : %d) a (%d : %d)\n",
           start.physical_page.page,
           start.physical_page.address,
           anEnd.physical_page.page,
           anEnd.physical_page.address
    );
}

void show_allocation_index(Task task) {
    printf("\n>> Enderecos acessados:\n");

    int i;

    for (i = 0; i < task.memory_accesses; i++) {
        memory_address m = task.accesses[i];
        printf("\n\t%s[%d]\n", m.allocation->var_name, m.accessed_index);
        printf("\tEndereco logico = %d : %d\n", m.logic_page.page, m.logic_page.address);
        printf("\tEndereco fisico = %d : %d\n", m.physical_page.page, m.physical_page.address);
    }

    if (i == 0)
        printf("\n\tNenhum acesso a memoria realizado");
}

void show_pages_ranges(Task task) {

    printf("\n>> Tabela de paginas\n");
    if (!task.last)
        return;

    for (int i = 0; i <= task.last->logic_page; i++) {
        printf("\n\tPL %d (%d a %d)", i, i * PAGE_SIZE, PAGE_SIZE * (i + 1) - 1);
        int physical_address_start = SYSTEM_MEMORY + PAGE_SIZE * (i);

        printf(" --> PF %d (%d %d)",
               (PAGE_SIZE / PAGE_SIZE) + i,
               physical_address_start,
               physical_address_start + PAGE_SIZE - 1);
    }
}

void execute_instructions(Task *task, FILE *instructions_file) {
    char instruction[INSTRUCTION_LENGTH];
    const char key_access_start[] = "[";
    const char key_access_end[] = "]";

    while (fgets(instruction, INSTRUCTION_LENGTH, instructions_file)) {
        remove_newline(instruction);
        remove_newline(instruction);

        if (!strcmp(instruction, ""))
            return;

        if (contains(instruction, KEY_NEW)) {
            if (execute_allocation(instruction, task)) {
                task->cpu_time++;
                continue;
            }
        } else if (contains(instruction, key_access_start) && contains(instruction, key_access_end)) {
            if (execute_access(instruction, task)) {
                task->cpu_time++;
                continue;
            } else
                return;
        }

        printf("\nA tarefa %s foi cancelada porque tem a instrucao invalida \"%s\".", task->name, instruction);
        return;
    }
}

int execute_access(char instruction[], Task *task) {

    // malformed instruction
    if (starts_with(instruction, "[") || starts_with("]", instruction)) {
        return 0;
    }

    char *var_name = strtok(instruction, "[");

    if (var_name) {
        char *str_pos = strtok(NULL, "]");
        int pos;
        if (str_pos) {
            pos = atoi(str_pos);// NOLINT(cert-err34-c)
            memory_address addr;

            if (access_memory(var_name, pos, task, &addr) == 1) {
                task->accesses[task->memory_accesses++] = addr;
                return 1;
            }

            // no variable allocated or requested position is out of bounds
            printf("\nA tarefa %s tentou realizar um acesso invalido a memoria na linha: %s\n",
                   task->name,
                   instruction
            );

            return 0;
        }

        return 0;
    }

    // malformed instruction
    return 0;
}

int execute_allocation(char instruction[50], Task *task) {

    // malformed instruction
    if (starts_with(instruction, KEY_NEW) || ends_with(instruction, KEY_NEW)) {
        return 0;
    }

    char *varName = strtok(instruction, " ");
    if (varName) {
        char *str_size = strtok(NULL, KEY_NEW);
        int size;

        if (str_size && (size = atoi(str_size)) > 0) // NOLINT(cert-err34-c)
            return allocate_memory(varName, size, task);
    }

    // malformed instruction
    return 0;
}

int main(int argc, char **argv) {
    run(argc, argv);
    return 0;
}
