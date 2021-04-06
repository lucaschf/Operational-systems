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
    if (!queue)
        return 0;

    if (isFull(queue)) {
        return 0;
    }
    if (queue->front == -1)
        queue->front = 0;

    if (queue->rear == MAXIMUM_PARALLEL_TASKS - 1)// rear is at last position of queue
        queue->rear = 0;
    else
        queue->rear++;

    queue->tasks[queue->rear] = task;

    return 1;
}

int isFull(Queue *queue) {
    if (!queue)
        return 1;

    return (queue->front == 0 && queue->rear == MAXIMUM_PARALLEL_TASKS - 1) ||
           (queue->front == queue->rear + 1);
}

int isEmpty(Queue *queue) {
    if (!queue)
        return 0;

    return queue->front == -1;
}

int peek(Queue *queue, Task *task) {

    if (!queue || isEmpty(queue))
        return 0;

    *task = queue->tasks[queue->front];
    return 1;
}

int dequeue(Queue *queue, Task *task) {
    if (!queue || isEmpty(queue)) {
        return 0;
    }

    *task = queue->tasks[queue->front];
    if (queue->front == queue->rear) /* queue has only one element */
    {
        queue->front = -1;
        queue->rear = -1;
    } else if (queue->front == MAXIMUM_PARALLEL_TASKS - 1)
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

// retrieves the task name based on file name
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

/**
 * checks whether the arguments provided are valid. if not, it aborts execution;
 *
 * @param argc args count
 * @param argv args values
 */
void checkArgs(int argc, char **argv) {
    if (argc < 2 || argc > MAXIMUM_PARALLEL_TASKS + 1) {
        printf("%s\n", WRONG_COMMAND_SYNTAX);
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; i++) {
        if (!endsWith(argv[i], ".tsk")) {
            printf("\n%s: %s", INVALID_ARGUMENT, argv[i]);
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
    endInterval(&task);
    task.suspendedIntervals[task.suspendedTimes] = startInterval();
    enqueue(suspendedTasks, task);
}

// ends the task, which means that the task successfully executed all instructions
void endTask(Task task) {
    task.state = FINISHED;
    task.endTime = system_.currentCpuTime;
    task.suspendedAt = -1;
    fclose(task.instructionsFile);
    endInterval(&task);
    enqueue(endedTasks, task);
}

// aborts the task, which means that the task has one or more unsupported instructions
void abortTask(Task task) {
    task.state = ABORTED;
    task.endTime = system_.currentCpuTime;
    task.suspendedAt = -1;
    fclose(task.instructionsFile);
    endInterval(&task);
    enqueue(abortedTasks, task);
}

// sets the default values to a task and enqueue it as new task
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
        hasNew = arrivedTask.arrivalTime <= system_.currentCpuTime &&
                 dequeue(newTasks, &arrivedTask);
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
            if (!reachedLastInstruction(&suspended)) {
                prepareTask(suspended);
            } else {
                endTask(suspended);
            }
            prepareTask(arrivedTask);
        } else {
            prepareTask(arrivedTask);
            if (!reachedLastInstruction(&suspended)) {
                prepareTask(suspended);
            } else {
                endTask(suspended);
            }
        }
    } else {
        if (hasNew)
            prepareTask(arrivedTask);
        if (hasSuspended) {
            if (!reachedLastInstruction(&suspended)) {
                prepareTask(suspended);
            } else {
                endTask(suspended);
            }
        }
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
        task.id = i;
        fail = !task.instructionsFile;

        if (!fail) {
            task.size_ = getTaskSize(task.instructionsFile);
        }

        // if any argument is invalid or cannot open the file, abort.
        if (fail || task.size_ <= 0) {
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

int reachedLastInstruction(Task *task) {
    Instruction instruction;

    // Stores the current position of the file pointer
    size_t temp = ftell(task->instructionsFile);

    // not reading a line means that there is nothing else in the file, that is, there are no further instructions
    if (!fgets(instruction, INSTRUCTION_LENGTH, task->instructionsFile)) {
        return 1;
    }

    // A line has been read, we must check if it is empty, which signals the end of the instructions
    removeNewline(instruction);
    if (!strcmp(instruction, "")) {
        return 1;
    }

    // The line is not empty, potentially being a new instruction. Therefore, we must move the file pointer backwards
    fseek(task->instructionsFile, temp, SEEK_SET);
    return 0;
}

void scheduler() {

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
            if (!fgets(instruction, INSTRUCTION_LENGTH, task.instructionsFile)) {
                task.state = FINISHED;
                break;
            }

            removeNewline(instruction);

            // reached the last instruction
            if (!strcmp(instruction, "")) {
                task.state = FINISHED;
                break;
            }

            if (execute(&task, instruction))
                task.lastInstruction++;

            system_.currentCpuTime++;

            // if task is aborted, suspended or ended, we must remove it from cpu.
            if (task.state == FINISHED || task.state == SUSPENDED || task.state == ABORTED)
                break;
        }

        if (task.state == RUNNING) {
            task.state = reachedLastInstruction(&task) ? FINISHED : READY;
        }

        removeFromCpu(&task);
    }
}

int executeMemoryAllocation(Task *task, Instruction instruction) {

    // malformed instruction
    if (startsWith("new", instruction) || endsWith("new", instruction)) {
        task->state = ABORTED;
        return 0;
    }

    char *varName = strtok(instruction, " ");
    if (varName) {
        char *size = strtok(NULL, "new");

        if (size && allocMemory(task, varName, atoi(size))) {// NOLINT(cert-err34-c)
            if (task->cpuInterval[task->cpuUsageTimes].end != -1)
                task->cpuInterval[task->cpuUsageTimes] = startInterval();
            task->cpuTime++;

            return 1;
        }
    }

    // malformed instruction
    task->state = ABORTED;
    return 0;
}

int executeMemoryAccess(Task *task, Instruction instruction) {

    // malformed instruction
    if (startsWith("[", instruction) || startsWith("]", instruction)) {
        task->state = ABORTED;
        return 0;
    }

    char *varName = strtok(instruction, "[");
    if (varName) {
        char *strPos = strtok(NULL, "]");

        if (strPos) {
            if (accessMemory(task, varName, atoi(strPos)) == 1) { // NOLINT(cert-err34-c)
                if (task->cpuInterval[task->cpuUsageTimes].end != -1)
                    task->cpuInterval[task->cpuUsageTimes] = startInterval();

                task->cpuTime++;
                return 1;
            }

            // no variable allocated or requested position is out of bounds
            task->state = ABORTED;
            printf("\nA tarefa %s tentou realizar um acesso invalido a memoria na linha: %d - %s\n",
                   task->name,
                   task->lastInstruction + 1,
                   instruction
            );

            return 0;
        }

        task->state = ABORTED;
        return 0;
    }

    // malformed instruction
    return 0;
}

int execute(Task *task, Instruction instruction) {

    // this is a IO execution, so we should suspend the task
    if (!strcmp(instruction, "read disk")) {
        task->state = SUSPENDED;
        endInterval(task); // stop cpu interval
        return 1;
    }

    // this is a memory allocation instruction
    if (contains(instruction, "new")) {
        return executeMemoryAllocation(task, instruction);
    }

    // potentially a memory access instruction
    if (contains(instruction, "[") && contains(instruction, "]")) {
        return executeMemoryAccess(task, instruction);
    }

    // is not a recognized instruction. The task must be aborted.
    task->state = ABORTED;
    printf("%s '%s' %s\n%s: %d.\n",
           THE_TASK,
           task->name,
           WILL_BE_ABORTED_DUE_UNSUPPORTED_INSTRUCTION,
           LAST_LINE_EXECUTED,
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
    TimeInterval *interval;

    interval = &(task->cpuInterval[task->cpuUsageTimes]);
    if (interval->end == -1) {
        interval->end = system_.currentCpuTime;
        task->cpuUsageTimes++;
    }

    interval = &(task->suspendedIntervals[task->suspendedTimes]);
    if (interval->end == -1) {
        interval->end = system_.currentCpuTime;
        task->suspendedTimes++;
    }
}

void report() {
    Task task;

    printf("\n%s: %d\n\n", TOTAL_CPU_TIME, system_.currentCpuTime);

    while (dequeue(endedTasks, &task)) {
        int tt = task.endTime - task.arrivalTime;

        printf(">> %s\n", task.name);
        printf("\n\t%s: %d", END_TIME, task.endTime);
        printf("\n\t%s: %d", ARRIVAL_TIME, task.arrivalTime);
        printf("\n\t%s: %0.f%%", PERCENTAGE_OF_PROCESSOR_OCCUPATION, (double) task.cpuTime / system_.currentCpuTime *
                                                                     100.0); // NOLINT(cppcoreguidelines-narrowing-conversions)
        printf("\n\t%s: %d", CPU_TIME, task.cpuTime);
        printf("\n\t%s: %d", DISK_TIME, task.ioTime);
        printf("\n\t%s: %d", TURN_AROUND_TIME, tt);
        printf("\n\t%s: %d", WAIT_TIME, tt - task.cpuTime);
        printf("\n\t%s: %d", RESPONSE_TIME, task.cpuInterval[0].start - task.arrivalTime);

        printf("\n\n\t%s: \n", PROCESSOR);
        for (int i = 0; i < task.cpuUsageTimes; i++)
            printf("\t\t%d a %d ut\n", task.cpuInterval[i].start, task.cpuInterval[i].end);

        printf("\t%s: \n", DISK);
        if (!task.suspendedTimes) {
            printf("\t\t%s\n\n", NO_DISC_ACCESS_PERFORMED);
        } else
            for (int i = 0; i < task.suspendedTimes; i++)
                printf("\t\t%d a %d ut\n", task.suspendedIntervals[i].start, task.suspendedIntervals[i].end);

        printf("\t%s:\n", RESERVED_ADDRESSES);
        for (int i = 0; i < system_.pagesCount; i++) {
            Page p = system_.pages[i];

            if (p.taskIdentifier == task.id && p.logicalMemory[0].distanceFromFirstAddress == 0) {
                printf("\t\tIdentificador: %s -> ", p.logicalMemory[0].identifier);
                printf("%d : %d\n", p.number, p.logicalMemory[0].displacement);
            }
        }
    }
}

void run(int argc, char **argv) {
    Task task;

    system_.currentCpuTime = 0;
    system_.lastPhysicalAddress = SYSTEM_MEMORY;

    checkArgs(argc, argv);

    if (!initQueues()) {
        printf("Failed to initialize");
        exit(EXIT_FAILURE);
    }

    extractBaseTaskData(argc, argv);

    if (dequeue(newTasks, &task)) {
        prepareTask(task);
    }

    scheduler();
    report();
    releaseQueues();
}

int allocMemory(Task *task, char identifier[], int size) {
    if (task->size_ + size > TASK_LOGICAL_MEMORY)
        return 0;

    Page page;
    page.addressesCount = 0;
    page.number = 0;
    page.taskIdentifier = task->id;

    if (!system_.pagesCount)
        page.number = system_.lastPhysicalAddress / PAGE_SIZE;
    else
        page.number = system_.pages[system_.pagesCount - 1].number + 1;

    for (int i = 0; i < size; i++) {
        LogicalMemory logicalMemory;
        logicalMemory.displacement = system_.lastPhysicalAddress % PHYSICAL_MEMORY;
        system_.lastPhysicalAddress++;
        logicalMemory.distanceFromFirstAddress = i;
        logicalMemory.distanceFromLastAddress = size - 1 - i;
        strcpy(logicalMemory.identifier, identifier);

        if (page.addressesCount == PAGE_SIZE) {
            system_.pages[system_.pagesCount++] = page;
            page.addressesCount = 0;
            page.logicalMemory[page.addressesCount++] = logicalMemory;
            page.number = system_.pages[system_.pagesCount - 1].number + 1;
        } else {
            page.logicalMemory[page.addressesCount++] = logicalMemory;
        }
    }

    if (system_.pagesCount == 0 || system_.pages[system_.pagesCount - 1].number != page.number)
        system_.pages[system_.pagesCount++] = page;

    task->size_ += size;

    return 1;
}

int accessMemory(Task *task, char identifier[], int position) {

    // out of bounds
    if (position < 0)
        return -1;

    if (system_.pagesCount == 0)
        return 0; // no allocation made

    for (int i = 0; i < system_.pagesCount; i++) {
        Page p = system_.pages[i];

        if (p.taskIdentifier != task->id)
            continue;

        if (!strcmp(p.logicalMemory[0].identifier, identifier)) {
            if (p.addressesCount > position) { // accepted address
                return 1;
            }

            if (p.logicalMemory[p.addressesCount - 1].distanceFromLastAddress == 0)
                return -1; // access violation - out of bounds
        }
    }

    return 0; // no variable found for the identifier and task
}

int main(int argc, char **argv) {
    setlocale(LC_ALL, "Portuguese");

    argc = 5;

    argv[1] = "C:\\Users\\lucas\\Desktop\\t1.tsk";
    argv[2] = "C:\\Users\\lucas\\Desktop\\t2.tsk";
    argv[3] = "C:\\Users\\lucas\\Desktop\\t3.tsk";
    argv[4] = "C:\\Users\\lucas\\Desktop\\t4.tsk";
    run(argc, argv);
    return 0;
}