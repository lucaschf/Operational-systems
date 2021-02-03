#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <errno.h>

#define MAX 100 // maximum number of availableInstructions
#define INSTRUCTION_LENGTH 10 // maximum length of parameterized instruction
#define INSTRUCTION_NAME_LENGTH 5 // maximum instruction name length

// accepted instructions
#define LOAD "LOAD"
#define STORE "STORE"
#define ADD "ADD"
#define SUB "SUB"
#define MUL "MUL"
#define DIV "DIV"
#define READ "READ"
#define WRITE "WRITE"
#define HALT "HALT"
#define AVAILABLE_INSTRUCTIONS 9

#define APP_NAME "Maquina de execucao LPAS"
#define EDIT_PROGRAM  "Editar Programa"
#define STOP_INSTRUCTION_READING_MESSAGE "Para finalizar tecle ENTER em uma linha em branco."
#define PROGRAM_NUMBER_ON_READ "Programa Nº"
#define PARAMETER_INVALID_OR_NOT_INFORMED "Parametro invalido ou não informado."
#define INVALID_INSTRUCTION "Instrucao invalida."
#define NO_PROGRAMS_FOUND "No programs found."
#define PROGRAM_NUMBER "Numero do programa"
#define INVALID_PROGRAM "Programa inválido"

typedef struct {
    char name[INSTRUCTION_NAME_LENGTH];
    int parameter;
} Instruction;

typedef struct {
    int instructionsCount;
    int recorder;
    Instruction instructions[MAX];
} Program;

const char *availableInstructions[AVAILABLE_INSTRUCTIONS] = {
        LOAD, STORE, ADD, SUB, MUL, DIV, READ, WRITE, HALT
};

Program programs[MAX];
int programsCount = 0;

void header(char *title) {
    printf("\t%s %s\n\n", APP_NAME, title);
}

/**
 * tries to retrieve a numeric value contained in a string.
 *
 * @param strValue the string containing the numeric value.
 * @param into pointer to variable where the retrieved value should be stored.
 * @return 1 if it was possible to retrieve a numerical value. 0 if there was an error retrieving the value.
 */
int recoverValue(char *strValue, int *into) {
    char *garbage = NULL;
    errno = 0;

    *into = strtol(strValue, &garbage, 10);

    return errno == ERANGE || errno == EINVAL ? 0 : 1;
}

/**
 * Assembles a properly parameterized instruction from a string containing the instruction data.
 *
 * @param parameterizedInstruction the complete parameterized instruction containing the instruction and its parameter
 * if applicable.
 * @param into pointer to the variable where the assembled statement is to be stored.
 * @return 1 if the instruction could be successfully assembled;
 *         0 if the required parameter has not been entered or is incorrect;
 *         -1 if the informed statement is not recognized.
 */
int assemblyInstruction(char parameterizedInstruction[INSTRUCTION_LENGTH], Instruction *into) {
    char delimiter[] = " ";
    char *operator;
    char *parameter;

    operator = strtok(parameterizedInstruction, delimiter);
    parameter = strtok(NULL, delimiter);

    for (int i = 0; i < AVAILABLE_INSTRUCTIONS; ++i) {
        if (!strcmp(availableInstructions[i], operator)) {
            strcpy(into->name, operator);

            if (strcmp(operator, HALT) != 0 && strcmp(operator, WRITE) != 0) {
                return recoverValue(parameter, &into->parameter);
            }

            return 1;
        }
    }

    return -1;
}

/**
 * Reads a fully parameterized instruction as string and removes the newLine character at the end.
 *
 * @param parameterizedInstruction pointer to variable where the read instruction should be stored.
 */
void readInstructionAsString(char *parameterizedInstruction) {
    setbuf(stdin, NULL);
    fgets(parameterizedInstruction, INSTRUCTION_LENGTH, stdin);
    size_t len = strlen(parameterizedInstruction);

    if (len > 0 && parameterizedInstruction[len - 1] == '\n') {
        parameterizedInstruction[--len] = '\0';
    }
}

/**
 * Performs a loop obtaining the instructions of a program through the user's input and adds the program in memory
 * for future execution.
 */
void readInstruction() {
    char strInstruction[INSTRUCTION_LENGTH];
    Instruction instruction;

    header(EDIT_PROGRAM);

    printf("%s\n\n%s %d\n----------------\n\n",
           STOP_INSTRUCTION_READING_MESSAGE,
           PROGRAM_NUMBER_ON_READ,
           programsCount + 1);

    Program program;
    program.instructionsCount = 0;

    do {
        printf("%02d.", program.instructionsCount + 1);
        readInstructionAsString(strInstruction);

        if (strcmp(strInstruction, "") == 0) {
            if (program.instructions > 0)
                programs[programsCount++] = program;
            return;
        }

        size_t len = strlen(strInstruction);

        if (len > 0 && strInstruction[len - 1] == '\n') {
            strInstruction[--len] = '\0';
        }

        int assemblyResult = assemblyInstruction(strInstruction, &instruction);

        if (assemblyResult == 1) {
            program.instructions[program.instructionsCount++] = instruction;
            continue;
        }

        printf("\n%s\n\n", assemblyResult == 0 ? PARAMETER_INVALID_OR_NOT_INFORMED : INVALID_INSTRUCTION);
    } while (strcmp(strInstruction, "") != 0 && program.instructionsCount < MAX);
}

/**
 * Prompts the user for the program number and returns the relative position in the array.
 *
 * @return the position of the requested program in the programs array or -1 if not valid.
 */
int readProgramNumber() {
    if (programsCount == 0) {
        printf("\n%s\n\n", NO_PROGRAMS_FOUND);
        return -1;
    }

    int target;
    printf("\n%s [1 a %d]: ", PROGRAM_NUMBER, programsCount);
    scanf(" %d", &target);

    if (target > programsCount || target < 1) {
        printf("%s\n", INVALID_PROGRAM);
        return -1;
    }

    return target - 1;
}

/**
 * Displays all instructions given to a program from its position in the array.
 *
 * @param position the program position in the array of programs.
 */
void showProgram(int position) {
    printf("\n\n");

    Program program = programs[position];

    for (int i = 0; i < program.instructionsCount; i++) {
        Instruction instruction = program.instructions[i];
        printf("%s %d\n", instruction.name, instruction.parameter);
    }

    printf("\n");
}

void showPrograms() {
    int target = readProgramNumber();

    if (target != -1) {
        showProgram(target);
        system("pause");
    }
}

/**
 * Performs a loop executing every instruction of the program.
 *
 * @param program the program to be executed.
 */
void executeProgram(Program program) {
    program.recorder = 0;

    for (int i = 0; i < program.instructionsCount; i++) {
        Instruction instruction = program.instructions[i];

        if (!strcmp(instruction.name, HALT)) {
            return;
        }

        if (!strcmp(instruction.name, LOAD)) {
            program.recorder = instruction.parameter;
            continue;
        }

        if (!strcmp(instruction.name, ADD)) {
            program.recorder += instruction.parameter;
            continue;
        }

        if (!strcmp(instruction.name, SUB)) {
            program.recorder -= instruction.parameter;
            continue;
        }

        if (!strcmp(instruction.name, MUL)) {
            program.recorder *= instruction.parameter;
            continue;
        }

        if (!strcmp(instruction.name, DIV)) {
            program.recorder /= instruction.parameter;
            continue;
        }

//       TODO if (!strcmp(instruction.name, READ)) {
//
//            continue;
//        }
//
//        if (!strcmp(instruction.name, WRITE)) {
//
//            continue;
//        }
//
//        if (!strcmp(instruction.name, STORE)) {
//
//            continue;
//        }
    }
}

/**
 * Prompts the user for the program number to be executed and starts executing it, if found.
 */
void execute() {
    int target = readProgramNumber();

    if (target != -1) {
        executeProgram(programs[target]);
        system("pause");
    }
}

void menu() {
    int chosen;

    do {
        header("");

        printf("1 - Editar Programa");
        printf("\n2 - Exibir Programa");
        printf("\n3 - Executar Programa");
        printf("\n4 - Sair\n\n");
        scanf("%d", &chosen);

        switch (chosen) {
            case 1:
                readInstruction();
                break;
            case 2:
                showPrograms();
                break;
            case 3:
                execute();
                break;
            case 4:
                exit(0);
            default:
                break;
        }

    } while (chosen != 4);
}

int main() {
    setlocale(LC_ALL, "");
    menu();
    return 0;
}