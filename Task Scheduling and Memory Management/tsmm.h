#ifndef TASK_SCHEDULING_AND_MEMORY_MANAGEMENT_TSMM_H
#define TASK_SCHEDULING_AND_MEMORY_MANAGEMENT_TSMM_H

#include <stdlib.h>
#include <stdio.h>

#define TIME_QUANTUM 2
#define INSTRUCTION_LENGTH 50
#define MAXIMUM_PARALLEL_TASKS 4

#define MAX_QUEUE_SIZE 30
#define IO_SUSPENSION_TIME 5
#define TASK_LOGICAL_MEMORY 4096 // bytes - 4K
#define PHYSICAL_MEMORY 65536 // bytes - 64K
#define SYSTEM_MEMORY 20480 // bytes
#define PAGE_SIZE 512 // bytes
#define IDENTIFIER_LENGTH 20

//region Strings
#define TASK_SIZE_DELIMITER "#T="
#define WRONG_COMMAND_SYNTAX "A sintaxe do comando esta incorreta"
#define INVALID_ARGUMENT "Argumento invalido"
#define TOTAL_CPU_TIME "Tempo total de CPU"
#define NO_DISC_ACCESS_PERFORMED "Nenhum acesso a disco realizado"
#define END_TIME "Instante de finalizacao"
#define ARRIVAL_TIME "Instante de chegada"
#define CPU_TIME "Tempo de cpu"
#define DISK_TIME "Tempo de disco"
#define TURN_AROUND_TIME "Turn around time"
#define WAIT_TIME "Tempo de espera"
#define RESPONSE_TIME "Response time"
#define PROCESSOR "Processador"
#define PERCENTAGE_OF_PROCESSOR_OCCUPATION "Porcentagem de ocupacao do processador"
#define DISK "Disco"
#define THE_TASK "A tarefa"
#define LAST_LINE_EXECUTED "Ultima linha executada"
#define WILL_BE_ABORTED_DUE_UNSUPPORTED_INSTRUCTION "nao sera executada, pois tem instrucoes diferentes do tipo 1, 2 e 3."
#define RESERVED_ADDRESSES "Enderecos reservados"
//endregion

typedef char Instruction[INSTRUCTION_LENGTH];

typedef enum {
    NEW, READY, RUNNING, SUSPENDED, FINISHED, ABORTED
} State;

typedef struct {
    int start;
    int end;
} TimeInterval;

typedef struct {
    char identifier[IDENTIFIER_LENGTH];
    int distanceFromLastAddress;
    int distanceFromFirstAddress;
    int displacement;
} LogicalMemory;

typedef struct {
    int taskIdentifier;
    int number;
    int addressesCount;
    LogicalMemory logicalMemory[PAGE_SIZE];
} Page;

typedef struct {
    int id;
    char *name;
    State state;
    int cpuTime;
    int ioTime;
    int size_;
    int arrivalTime;
    int suspendedAt;
    int endTime;
    int lastInstruction; // counts the successful performed instruction
    FILE *instructionsFile; // file that contains task instructions

    int suspendedTimes;
    TimeInterval suspendedIntervals[MAX_QUEUE_SIZE];

    int cpuUsageTimes;
    TimeInterval cpuInterval[MAX_QUEUE_SIZE];
} Task;

typedef struct {
    int currentCpuTime;
    int lastPhysicalAddress;
    Page pages[PAGE_SIZE];
    int pagesCount;
} System;

// region task queue

typedef struct {
    Task tasks[MAXIMUM_PARALLEL_TASKS];
    int front;
    int rear;
} Queue;

typedef Queue *QUEUE;

Queue *newQueue();

int enqueue(Queue *queue, Task task);

int isFull(Queue *queue);

int isEmpty(Queue *queue);

int peek(Queue *queue, Task *task);

int dequeue(Queue *queue, Task *task);

int allocMemory(Task *task, char identifier[], int size);

int accessMemory(Task *task, char identifier[], int position);

// endregion

// region string utils

/**
 * Returns 1 if and only if the string contains the specified
 * sequence of char values.
 *
 * @param str the sequence to checked
 * @param another the sequence to search for
 * @return true 1 this string contains the specified string, 0 otherwise
 */
int contains(const char *str, const char *another);

/**
 * Checks whether the string is ending with user-specified substring or not. Based on this comparison it will return 1 or 0.
 *
 * @param s the string to be checked.
 * @param suffix this is a suffix.
 * @return 1 if character sequence supplied in "suffix" matches the end sequence of the string, 0 otherwise.
 */
int endsWith(const char *s, const char *suffix);

/**
 * Removes trailing newline of a given string
 *
 * @param str
 */
void removeNewline(char *str);

/**
 * Checks whether the string is starting with user-specified substring or not. Based on this comparison it will return 1 or 0.
 *
 * @param s the string to be checked.
 * @param prefix this is a prefix.
 * @return 1 if character sequence supplied in "prefix" matches the start sequence of the string, 0 otherwise.
 */
int startsWith(const char *str, const char *prefix);

// endregion

/**
 * Creates a TimeInterval begin with system current cpu time, the end of this interval has not been defined yet.
 *
 * @return the created interval
 */
TimeInterval startInterval();

/**
 * Check in task if there are any intervals open and if found any, ends it with system current cpu time.
 *
 * @param task
 */
void endInterval(Task *task);

/**
 * Task scheduler
 */
void scheduler();

/**
 * Executes an instruction on a specific task
 *
 * @param task
 * @param instruction
 * @return 1 if successful, 0 if instruction is malformed or execution fails
 */
int execute(Task *task, Instruction instruction);

/**
 * Checks whether the task reached its last instruction or not
 *
 * @param task the task to be verified
 * @return 1 if last instruction is reached, 0 otherwise
 */
int reachedLastInstruction(Task *task);

#endif //TASK_SCHEDULING_AND_MEMORY_MANAGEMENT_TSMM_H