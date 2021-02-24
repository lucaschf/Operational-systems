#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PROCESS_IDENTIFIERS "ABCDEF"

void show_running_message(int identifier) {
    printf("Process %c started...\n", PROCESS_IDENTIFIERS[identifier]);
}

void show_process_ended_message(int identifier) {
    printf("Process %c ended.\n", PROCESS_IDENTIFIERS[identifier]);
}

int main() {
    pid_t pidB;
    pid_t pidC;
    pid_t pidD;
    pid_t pidE;
    pid_t pidF;

    show_running_message(0);

    if ((pidB = fork()) < 0) {
        perror("fork");
        exit(1);
    }

    if (pidB == 0) {
        show_running_message(1);

        if ((pidD = fork()) < 0) {
            perror("fork");
            exit(1);
        }

        if (pidD == 0) {
            show_running_message(3);
            show_process_ended_message(3);
        } else {
            if ((pidE = fork()) < 0) {
                perror("fork");
                exit(1);
            }

            if (pidE == 0) {
                show_running_message(4);
                show_process_ended_message(4);
            } else {
                if ((pidF = fork()) < 0) {
                    perror("fork");
                    exit(1);
                }

                if (pidF == 0) {
                    show_running_message(5);
                    show_process_ended_message(5);
                } else {
                    waitpid(pidD, NULL, WCONTINUED);
                    waitpid(pidE, NULL, WCONTINUED);
                    waitpid(pidF, NULL, WCONTINUED);

                    show_process_ended_message(1);
                }
            }
        }
    } else {
        if ((pidC = fork()) < 0) {
            perror("fork");
            exit(1);
        }

        if (pidC == 0) {
            show_running_message(2);
            show_process_ended_message(2);
        } else {
            waitpid(pidC, NULL, WCONTINUED);
            waitpid(pidB, NULL, WCONTINUED);

            show_process_ended_message(0);
        }
    }

    return 0;
}