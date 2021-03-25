#include<stdio.h>
#include <regex.h>

void swap(int a[], int j, int k) {
    int temp;
    temp = a[j];
    a[j] = a[k];
    a[k] = temp;
}
int match(const char *string, const char *pattern)
{
    regex_t re;
    if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0)
        return 0;
    int status = regexec(&re, string, 0, NULL, 0);
    regfree(&re);

    if (status != 0)
        return 0;

    return 1;
}

int main(){
    char reg[] = "[A-Za-z]+[0-9.\\-\\[\\]]++$";
    char tst[] = "v[100]";

    printf("%s Given string matches %s? %s\n", tst, reg, match(tst, reg) ? "true" : "false");
    return 0;
}


//void main() {
//    int process[10];
//    int burstTime[10];
//    int arrivalTime[10];
//
//    int c;
//    int n;
//    int gantt[10][20];
//    int ready[10][20];
//    int q;
//    int flag = 1;
//    int sum = 0;
//    int wt[20];
//    int tt[20];
//    int count = 0;
//    int s;
//    int m;
//    int i
//    int j;
//
//    printf("enter number of processes :");
//    scanf("%d", &n);
//    printf("enter time quantum :");
//    scanf("%d", &q);
//
//    for ( i = 0; i < n; i++) {
//        printf("enter process number :");
//        scanf("%d", &process[i]);
//        printf("enter arrival time for process%d :", process[i]);
//        scanf("%d", &arrivalTime[i]);
//        printf("enter burst time for process%d :", process[i]);
//        scanf("%d", &burstTime[i]);
//        printf("\n");
//    }
//    for ( i = 0; i < n; i++) {
//        sum = sum + burstTime[i];
//    }
//
//    for ( i = 0; i < n; i++) {
//        for (int j = 0; j < n - i - 1; j++) {
//            if (arrivalTime[j] > arrivalTime[j + 1]) {
//                swap(arrivalTime, j, j + 1);
//                swap(burstTime, j, j + 1);
//                swap(process, j, j + 1);
//            }
//        }
//    }
//
//    int t = 0, k = 0;          //process[]  //gantt[]
//    gantt[0][k] = process[t];
//
//    c = 0;              //r[]
//    for (j = 1; j < n; j++) {
//        if (arrivalTime[j] <= burstTime[t]) {
//            ready[0][c] = process[j];
//            ready[1][c] = burstTime[j];
//            c++;
//        }
//    }
//
//
//    if (burstTime[0] >= q) {
//        gantt[1][k] = q;
//        if (burstTime[0] > q) {
//            ready[0][c] = process[0];
//            ready[1][c] = burstTime[0] - q;
//            c++;
//        }
//    } else {
//        gantt[1][k] = burstTime[0];
//    }
//    i = 0;
//    while (i < 20) {
//
//        flag = 1;
//        if (ready[1][i] >= q) {
//            k++;
//            gantt[0][k] = ready[0][i];
//            gantt[1][k] = gantt[1][k - 1] + q;
//
//            for (j = 0; j < n; j++) {
//                if (gantt[1][k] >= arrivalTime[j] && arrivalTime[j] > gantt[1][k - 1]) {
//                    ready[0][c] = process[j];
//                    ready[1][c] = burstTime[j];
//                    c++;
//                }
//            }
//            if (ready[1][i] > q) {
//                ready[0][c] = ready[0][i];
//                ready[1][c] = ready[1][i] - q;
//                c++;
//            }
//        } else {
//            if (ready[1][i] < q) {
//                if (ready[1][i] > 0) {
//                    k++;
//                    gantt[0][k] = ready[0][i];
//                    gantt[1][k] = gantt[1][k - 1] + ready[1][i];
//
//                    for (j = 0; j < n; j++) {
//                        if (gantt[1][k] >= arrivalTime[j] && arrivalTime[j] > gantt[1][k - 1]) {
//                            ready[0][c] = process[j];
//                            ready[1][c] = burstTime[j];
//                            c++;
//                        }
//                    }
//                }
//            }
//        }
//        i++;
//        if (gantt[1][k] == sum) {
//            break;
//        }
//
//    }
//    printf("Gantt chart:-\n|");
//    for (i = 0; i < k + 1; i++) {
//        printf("  process%d\t|", gantt[0][i]);
//
//    }
//    printf("\n");
//    printf("0");
//    for (i = 0; i < k + 1; i++) {
//        printf("\t%d ", gantt[1][i]);
//    }
//
//    printf("\nReady queue:-\n|");
//    for (i = 0; i < c; i++) {
//        printf("   process%d\t|", ready[0][i]);
//
//    }
//    printf("\n");
//    for (i = 0; i < c; i++) {
//        printf("    %d\t", ready[1][i]);
//
//    }
//    m = 0; //tt
//    for (i = 0; i < n; i++) {
//        count = 0;
//        s = 0;
//        for (j = 0; j < k + 1; j++) {
//            if (gantt[0][j] == process[i]) {
//                count++;
//            }
//        }
//        for (j = 0; j < k + 1; j++) {
//            if (gantt[0][j] == process[i]) {
//                s++;
//                if (s == count) {
//                    tt[m] = gantt[1][j] - arrivalTime[i];
//                    wt[m] = tt[m] - burstTime[i];
//
//                    m++;
//                }
//            }
//        }
//    }
//    float att = 0, awt = 0;
//    printf("\n-----------------------------------------------------------------------\nprocess id\tarrival time\tburst time\tturnaround time\t\twaiting time");
//    for (i = 0; i < n; i++) {
//        printf("\n%d\t\t%d\t\t%d\t\t%d\t\t\t%d", process[i], arrivalTime[i], burstTime[i], tt[i], wt[i]);
//        att = att + tt[i];
//        awt = awt + wt[i];
//
//    }
//
//    printf("\nAverage turnaround time=%.3f", att / n);
//    printf("\nAverage waiting time=%.3f\n", awt / n);
//}