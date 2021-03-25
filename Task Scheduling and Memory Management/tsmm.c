#include "tsmm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// region queue
Queue *newQueue() {
    Queue *q = (Queue *) malloc(sizeof(Queue));

    if (q) {
        q->front = -1;
        q->rear = -1;
    }

    return q;
}

int enqueue(Queue *queue, Task task) {
    if (isFull(queue)) {
        return 0;
    }
    if (queue->front == -1)
        queue->front = 0;

    if (queue->rear == MAX_QUEUE_SIZE - 1)/*rear is at last position of queue*/
        queue->rear = 0;
    else
        queue->rear++;

    queue->tasks[queue->rear] = task;

    return 1;
}

int isFull(Queue *queue) {
    return (queue->front == 0 && queue->rear == MAX_QUEUE_SIZE - 1) ||
           (queue->front == queue->rear + 1);
}

int isEmpty(Queue *queue) {
    return queue->front == -1;
}

int peek(Queue *queue, Task *task) {
    if (isEmpty(queue))
        return 0;

    *task = queue->tasks[queue->front];
    return 1;
}

int dequeue(Queue *queue, Task *task) {
    if (isEmpty(queue)) {
        return 0;
    }

    *task = queue->tasks[queue->front];
    if (queue->front == queue->rear) /* queue has only one element */
    {
        queue->front = -1;
        queue->rear = -1;
    } else if (queue->front == MAX_QUEUE_SIZE - 1)
        queue->front = 0;
    else
        queue->front++;

    return 1;
}
//endregion

// region utils

int contains(const char *str, const char *another) {
    return strstr(str, another) != NULL;
}

int endsWith(const char *s, const char *suffix) {
    char *comparator;
    char *anotherComparator;

    comparator = strstr(s, suffix); // gets the final suffixed string.

    if (!comparator)
        return 0;

    anotherComparator = strstr(comparator, suffix); //splits again to check if the suffix is at the end of last word.

    return !strcmp(suffix, anotherComparator);
}

int startsWith(const char *str, const char *prefix) {
    return strncmp(prefix, str, strlen(prefix)) == 0;
}
// endregion

int getTaskSize(FILE *f) {
    rewind(f);

    char line[50];

    if (fgets(line, 50, f)) {
        char *token = strtok(line, "=");

        if (token) {
            if (strcmp(TASK_SIZE_DELIMITER, token) != 0)
                return -1;

            token = strtok(NULL, "=");

            if (token)
                return atoi(token);
        }
    }

    return -1;
}

QUEUE readyTasks;
QUEUE newTasks;
QUEUE suspendedTasks;
QUEUE endedTasks;
System system_;


void checkArgs(int argc, char **argv) {
    if (argc < 2 || argc > MAXIMUM_PARALLEL_TASKS + 1) {
        printf("Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; i++) {
        if (!endsWith(argv[i], ".tsk")) {
            printf("\nThe argument [%d] is invalid", i);
            exit(EXIT_FAILURE);
        }
    }
}

void enqueueReady(Task task) {
    task.state = READY;
    task.suspendedAt = -1;
    enqueue(readyTasks, task);
}

void enqueueSuspended(Task task) {
    task.suspendedAt = system_.currentCpuTime;
    task.state = SUSPENDED;
    task.ioTime += IO_SUSPENSION_TIME;
    enqueue(suspendedTasks, task);
}

void enqueueDone(Task task) {
    task.state = FINISHED;
    task.suspendedAt = -1;
    task.endTime = system_.currentCpuTime;
    enqueue(endedTasks, task);
}

void enqueueNew(Task task) {
    task.state = NEW;
    task.suspendedAt = -1;
    task.endTime = -1;
    task.lastInstruction = 0;
    task.ioTime = 0;
    task.cpuTime = 0;
    enqueue(newTasks, task);
}

int initQueues() {
    newTasks = newQueue();
    if (!newTasks)
        return 0;

    readyTasks = newQueue();
    if (!readyTasks) {
        free(newTasks);
        return 0;
    }

    suspendedTasks = newQueue();
    if (!suspendedTasks) {
        free(newTasks);
        free(readyTasks);
        return 0;
    }

    endedTasks = newQueue();
    if (!endedTasks) {
        free(newTasks);
        free(readyTasks);
        free(suspendedTasks);
        return 0;
    }

    return 1;
}

void releaseQueues() {
    free(newTasks);
    free(readyTasks);
    free(suspendedTasks);
    free(endedTasks);
}

void removeFromCpu(Task *task) {
    Task arrivedTask;
    Task suspended;

    int hasNew = peek(newTasks, &arrivedTask); // check for new tasks
    int hasSuspended = peek(suspendedTasks, &suspended); // check for suspended tasks

    if (hasNew) { // there are new tasks. So we should check for the arrival time
        hasNew = arrivedTask.arrivalTime <= system_.currentCpuTime;

        if (hasNew) {
            dequeue(newTasks, &arrivedTask);
        }
    }

    if (hasSuspended) { // there are suspended tasks, so we have to check if the suspension time is already passed.
        if (suspended.suspendedAt + IO_SUSPENSION_TIME <= system_.currentCpuTime) {
            dequeue(suspendedTasks, &suspended);
        } else
            hasSuspended = 0;
    }

    // new tasks have priority over suspended tasks. Suspended tasks have priority over current running tasks.
    if (hasNew && hasSuspended) {
        if (suspended.suspendedAt + IO_SUSPENSION_TIME <= arrivedTask.arrivalTime) {
            enqueueReady(suspended);
            enqueueReady(arrivedTask);
        } else {
            enqueueReady(arrivedTask);
            enqueueReady(suspended);
        }
    } else {
        if (hasNew)
            enqueueReady(arrivedTask);
        if (hasSuspended)
            enqueueReady(suspended);
    }

    if(task) {
        switch (task->state) {
            case READY:
                enqueueReady(*task);
                break;
            case FINISHED:
                enqueueDone(*task);
                break;
            case SUSPENDED:
                enqueueSuspended(*task);
                break;
            default:
                break;
        }
    }
}

void extractTasks(int argc, char **argv) {
    int fail;

    for (int i = 1; i < argc; i++) {
        Task task;
        task.arrivalTime = i - 1;
        task.instructionsFile = fopen(argv[i], "r");

        fail = !task.instructionsFile;

        if (!fail) {
            task.size_ = getTaskSize(task.instructionsFile);
        }

        if (fail || task.size_ == -1) {
            for (int j = 0; j < i; j++) {
                dequeue(newTasks, &task);
                fclose(task.instructionsFile);
            }

            releaseQueues();
            if (fail) {
                printf("Unable to open task file: %s\n", argv[i]);
            } else
                printf("Invalid task size at: %s\n", argv[i]);

            exit(EXIT_FAILURE);
        }
        enqueueNew(task);
    }
}


int main(int argc, char **argv) {

    argc = 2;
    argv[1] = "../t1.tsk";

    checkArgs(argc, argv);
    Task runningTask;

    if (!initQueues()) {
        printf("Failed to initialize");
        exit(EXIT_FAILURE);
    }

    extractTasks(argc, argv);

    system_.currentCpuTime = 0;

    if (dequeue(newTasks, &runningTask))
        enqueueReady(runningTask);

    do {
        run();
    } while (!isEmpty(readyTasks) || !isEmpty(suspendedTasks) || !isEmpty(newTasks));

    printf("CPU TIME %d", system_.currentCpuTime);

    releaseQueues();

    return 0;
}

void run() {
    Task runningTask;
    Instruction instruction;

    if (dequeue(readyTasks, &runningTask)) {
        for (int i = 0; i < TIME_QUANTUM && runningTask.state != FINISHED && runningTask.state != SUSPENDED; i++) {
            if (fgets(instruction, INSTRUCTION_LENGTH, runningTask.instructionsFile)) {
                runningTask.lastInstruction++;
                if (strcmp(instruction, "") == 0) {
                    runningTask.state = FINISHED;
                } else {
                    system_.currentCpuTime++;
                    execute(&runningTask, instruction);
                }
            }else
                runningTask.state = FINISHED;
        }

        removeFromCpu(&runningTask);
    } else{
        removeFromCpu(NULL);
        system_.currentCpuTime++;
    }
}

void execute(Task *task, Instruction instruction) {
    if (!strcmp(instruction, "read disk")) {
        task->suspendedAt = system_.currentCpuTime;
        task->state = SUSPENDED;
        task->ioTime += IO_SUSPENSION_TIME;
    }

    if (contains(instruction, "new")) {
        task->cpuTime++;
    }

    if (contains(instruction, "[")) {
        task->cpuTime++;
    }
}
