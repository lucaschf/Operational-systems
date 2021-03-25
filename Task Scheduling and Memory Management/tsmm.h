#ifndef TASK_SCHEDULING_AND_MEMORY_MANAGEMENT_TSMM_H
#define TASK_SCHEDULING_AND_MEMORY_MANAGEMENT_TSMM_H

#include <stdlib.h>
#include <stdio.h>

#define TIME_QUANTUM 2
#define INSTRUCTION_LENGTH 50
#define MAXIMUM_PARALLEL_TASKS 4
#define TASK_SIZE_DELIMITER "#T="

#define MAX_QUEUE_SIZE 100
#define IO_SUSPENSION_TIME 5
#define MAX_TASK_SIZE 4000 // bytes

typedef char Instruction[INSTRUCTION_LENGTH];

typedef enum {
    NEW, READY, RUNNING, SUSPENDED, FINISHED, ABORTED
} State;

typedef struct {
    int start;
    int end;
} TimeInterval;

typedef struct {
    char* name;
    State state;
    int cpuTime;
    int ioTime;
    int size_;
    int arrivalTime;
    int suspendedAt;
    int endTime;
    int lastInstruction; // counts the successful performed instruction
    FILE *instructionsFile; // file that contains task instruction
    int suspendedTimes;
    TimeInterval ioInterval[MAX_QUEUE_SIZE];
    int cpuUsageTimes;
    TimeInterval cpuInterval[MAX_QUEUE_SIZE];
} Task;

typedef struct {
    int currentCpuTime;
    int totalMemory;
    int usedMemory;
    int freMemory;
} System;

// region task queue

typedef struct {
    Task tasks[MAX_QUEUE_SIZE];
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

TimeInterval startInterval();

void endInterval(Task *task);

void stagger();

int execute(Task *task, Instruction instruction);

#endif //TASK_SCHEDULING_AND_MEMORY_MANAGEMENT_TSMM_H