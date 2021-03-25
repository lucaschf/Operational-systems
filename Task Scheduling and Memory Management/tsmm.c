#include "tsmm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

// region Global variables
QUEUE readyTasks;
QUEUE newTasks;
QUEUE suspendedTasks;
QUEUE endedTasks;
QUEUE abortedTasks;
System system_;
// endregion

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

// region string utils

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

void removeNewline(char *str) {
    if (str == NULL)
        return;

    unsigned int length = strlen(str);

    if (str[length - 1] == '\n')
        str[length - 1] = '\0';
}

int startsWith(const char *str, const char *prefix) {
    return strncmp(prefix, str, strlen(prefix)) == 0;
}
// endregion

char *getTaskName(char *pathname) {
    char *name = NULL;

    if (pathname) {
        if (contains(pathname, "/"))
            name = strrchr(pathname, '/') + 1;
        else
            name = pathname;
    }

    return name;
}

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

// prepare a task to be executed
void prepareTask(Task task) {
    task.state = READY;
    task.suspendedAt = -1;
    endInterval(&task);
    enqueue(readyTasks, task);
}

// suspends the task execution, usually when performing an IO action
void suspendTask(Task task) {
    task.suspendedAt = system_.currentCpuTime;
    task.state = SUSPENDED;
    task.ioTime += IO_SUSPENSION_TIME;
    task.ioInterval[task.suspendedTimes] = startInterval();
    enqueue(suspendedTasks, task);
}

// ends the task, which means that the task successfully executed all instructions
void endTask(Task task) {
    task.state = FINISHED;
    task.endTime = system_.currentCpuTime;
    task.suspendedAt = -1;
    endInterval(&task);
    enqueue(endedTasks, task);
}

// aborts the task, which means that the task has one or more unsupported instructions
void abortTask(Task task) {
    task.state = ABORTED;
    task.endTime = system_.currentCpuTime;
    task.suspendedAt = -1;
    endInterval(&task);
    enqueue(abortedTasks, task);
}

// sets the default values to a task and enqueue it
void createTask(Task task) {
    task.state = NEW;
    task.suspendedAt = -1;
    task.endTime = -1;
    task.lastInstruction = 0;
    task.ioTime = 0;
    task.cpuTime = 0;
    task.suspendedTimes = 0;
    task.cpuUsageTimes = 0;
    enqueue(newTasks, task);
}

/**
 * Releases the allocated memory for tasks queues
 */
void releaseQueues() {
    if (newTasks)
        free(newTasks);

    if (readyTasks)
        free(readyTasks);

    if (suspendedTasks)
        free(suspendedTasks);

    if (endedTasks)
        free(endedTasks);

    if (abortedTasks)
        free(abortedTasks);
}

/**
 * Tries to initialize the queues for tasks management
 *
 * @return 1 successfully initialized queues, 0 otherwise.
 */
int initQueues() {
    newTasks = newQueue();
    if (!newTasks)
        return 0;

    readyTasks = newQueue();
    if (!readyTasks) {
        releaseQueues();
        return 0;
    }

    suspendedTasks = newQueue();
    if (!suspendedTasks) {
        releaseQueues();
        return 0;
    }

    endedTasks = newQueue();
    if (!endedTasks) {
        releaseQueues();
        return 0;
    }

    abortedTasks = newQueue();
    if (!abortedTasks) {
        releaseQueues();
        return 0;
    }

    return 1;
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

    // there are suspended tasks, so we have to check if the suspension time is already passed.
    if (hasSuspended) {
        if (suspended.suspendedAt + IO_SUSPENSION_TIME <= system_.currentCpuTime) {
            dequeue(suspendedTasks, &suspended);
            endInterval(&suspended);
        } else
            hasSuspended = 0;
    }

    // new tasks have priority over suspended tasks.
    // Suspended tasks have priority over current running tasks.
    if (hasNew && hasSuspended) {
        if (suspended.suspendedAt + IO_SUSPENSION_TIME <= arrivedTask.arrivalTime) {
            prepareTask(suspended);
            prepareTask(arrivedTask);
        } else {
            prepareTask(arrivedTask);
            prepareTask(suspended);
        }
    } else {
        if (hasNew)
            prepareTask(arrivedTask);
        if (hasSuspended)
            prepareTask(suspended);
    }

    if (task) {
        switch (task->state) {
            case READY:
                prepareTask(*task);
                break;
            case FINISHED:
                endTask(*task);
                break;
            case SUSPENDED:
                suspendTask(*task);
                break;
            case ABORTED:
                abortTask(*task);
                break;
            default:
                break;
        }
    }
}

int getTaskSize(FILE *f) {
    const size_t length = 50;
    char line[length];

    rewind(f);

    if (!fgets(line, length, f) || !startsWith(line, TASK_SIZE_DELIMITER)) {
        return -1;
    }

    char *token = strtok(line, TASK_SIZE_DELIMITER);

    if (token) {
        return atoi(token); // NOLINT(cert-err34-c)
    }

    return -1;
}

void extractBaseTaskData(int argc, char **argv) {
    int fail;

    for (int i = 1; i < argc; i++) {
        Task task;

        task.arrivalTime = i - 1;
        task.instructionsFile = fopen(argv[i], "r");

        fail = !task.instructionsFile;

        if (!fail) {
            task.size_ = getTaskSize(task.instructionsFile);
        }

        // if any argument is invalid or cannot open the file, abort.
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

        task.name = getTaskName(argv[i]);
        createTask(task);
    }
}

void stagger() {

    Instruction instruction;
    Task task;

    while (!isEmpty(readyTasks) || !isEmpty(suspendedTasks) || !isEmpty(newTasks)) {
        if (!dequeue(readyTasks, &task)) { // no task to be executed right now
            removeFromCpu(NULL);
            system_.currentCpuTime++;
            continue;
        }

        task.state = RUNNING;
        for (int i = 0; i < TIME_QUANTUM; i++) {
            if (fgets(instruction, INSTRUCTION_LENGTH, task.instructionsFile)) {
                removeNewline(instruction);

                if (execute(&task, instruction))
                    task.lastInstruction++;

                system_.currentCpuTime++;
            } else
                task.state = FINISHED;

            if (task.state == FINISHED || task.state == SUSPENDED)
                break;
        }

        if (task.state == RUNNING)
            task.state = READY;

        removeFromCpu(&task);
    }
}

int execute(Task *task, Instruction instruction) {

    // this is a IO execution, so we should suspend the task
    if (!strcmp(instruction, "read disk")) {
        task->state = SUSPENDED;
        endInterval(task); // stop cpu interval
        return 1;
    }

    if (contains(instruction, "new")) {
        // TODO
        if (task->cpuInterval[task->cpuUsageTimes].end != -1)
            task->cpuInterval[task->cpuUsageTimes] = startInterval();
        task->cpuTime++;
        return 1;
    }

    if (contains(instruction, "[")) {
        // TODO
        if (task->cpuInterval[task->cpuUsageTimes].end != -1)
            task->cpuInterval[task->cpuUsageTimes] = startInterval();

        task->cpuTime++;
        return 1;
    }

    task->state = ABORTED; // is not a recognized instruction. It should be aborted
    printf("A tarefa '%s' nao sera executada, pois tem instrucoes diferentes do tipo 1, 2 e 3.\nUltima linha executada: %d.\n",
           task->name,
           task->lastInstruction
    );
    return 0;
}

TimeInterval startInterval() {
    TimeInterval interval;
    interval.start = system_.currentCpuTime;
    interval.end = -1;

    return interval;
}

void endInterval(Task *task) {
    if (task->cpuInterval[task->cpuUsageTimes].end == -1) {
        task->cpuInterval[task->cpuUsageTimes].end = system_.currentCpuTime;
        task->cpuUsageTimes++;
    }

    if (task->ioInterval[task->suspendedTimes].end == -1) {
        task->ioInterval[task->suspendedTimes].end = system_.currentCpuTime;
        task->suspendedTimes++;
    }
}

void printResult() {
    Task task;

    printf("Tempo total de CPU:  %d\n\n", system_.currentCpuTime);

    while (dequeue(endedTasks, &task)) {
        int tt = task.endTime - task.arrivalTime;

        printf(">> %s\n", task.name);
        printf("\n\tInstante de finalizacao: %d", task.endTime);
        printf("\n\tTurn around time: %d", tt);
        printf("\n\tWait time: %d",  tt - task.cpuTime);
        printf("\n\tResponse time: %d",  task.cpuInterval[0].start - task.arrivalTime);

        printf("\n\n\tProcessador: \n");
        for (int i = 0; i < task.cpuUsageTimes; i++)
            printf("\t\t%d a %d ut\n", task.cpuInterval[i].start, task.cpuInterval[i].end);

        printf("\tDisco: \n");
        if (task.suspendedTimes)
            for (int i = 0; i < task.suspendedTimes; i++)
                printf("\t\t%d a %d ut\n", task.ioInterval[i].start, task.ioInterval[i].end);
        else
            printf("\t\tNenhum acesso a disco realizado\n\n");
    }
}

void run(int argc, char **argv) {
    Task task;

    checkArgs(argc, argv);

    setlocale(LC_ALL, "Portuguese");

    if (!initQueues()) {
        printf("Failed to initialize");
        exit(EXIT_FAILURE);
    }

    system_.currentCpuTime = 0;

    extractBaseTaskData(argc, argv);

    if (dequeue(newTasks, &task))
        prepareTask(task);

    stagger();
    printResult();
    releaseQueues();
}

int main(int argc, char **argv) {

    argc = 3;
    argv[1] = "../t1.tsk";
    argv[2] = "../t2.tsk";

    run(argc, argv);
    return 0;
}