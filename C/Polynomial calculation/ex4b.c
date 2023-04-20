#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define TRUE 1
#define FALSE 0
#define MAX_INPUT 50

int isNumber(char *);

int countThreads(char *);

int power(int, int);

void compute(char *, int);

int isValid(char *);

int value,shmId, *shmPtr;

int main() {
    char input[MAX_INPUT];
    int sum, *res;

    key_t key;
    if((key = ftok("/", 'a')) == -1) {
        perror("ftok failed");
        exit(EXIT_FAILURE);
    }
    shmId = shmget(key, sizeof(int)*10, IPC_CREAT | IPC_EXCL | 0666);
    if(shmId == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }
    shmPtr = (int*) shmat(shmId, NULL, 0);
    if(shmPtr == NULL) {
        perror("shmat filed");
        exit(EXIT_FAILURE);
    }

    while (TRUE) {
        printf("Enter a polynomial and value, to exit enter \"done\"\n");
        fgets(input, MAX_INPUT, stdin);
        input[strlen(input) - 1] = '\0';

        if (strcmp(input, "done") == 0) {
            shmdt(shmPtr);
            shmctl(shmId, IPC_RMID, NULL);
            break;
        }

        // extracting value of x
        unsigned long valueIndex = strcspn(input, " ") + 1;
        if (valueIndex == strlen(input) + 1 || !isNumber(input + valueIndex)) {
            fprintf(stderr, "Invalid input\n");
            continue;
        }
        value = atoi(input + valueIndex);
        input[valueIndex - 2] = '\0';


        if (!isValid(input)) {
            fprintf(stderr, "Invalid input\n");
            continue;
        }

        sum = 0;
        int forks = 0;

        char *token = strtok(input, "+");
        while (token != NULL) {
            if (strcspn(token, "x") != strlen(token)) {
                compute(token, forks);
                forks++;
            } else {
                sum += atoi(token);
            }
            token = strtok(NULL, "+");
        }

        for (int i = 0; i < forks; i++) {
            wait(NULL);
        }

        for(int i = 0; i < forks; i++) {
            sum += shmPtr[i];
        }


        printf("%d\n", sum);
    }
}

/**
 * This method checks if a string represent a number
 * @param str
 * @return 1 if given string consist of digits only
 */
int isNumber(char *str) {
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] < '0' || str[i] > '9')
            return FALSE;
    }
    return TRUE;
}

/**
 * This method returns the max amount of threads needed to compute a polynomial
 * @param str
 * @return amount of '+' char + 1
 */
int countThreads(char *str) {
    int val = 0;
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] == '+')
            val++;
    }
    return val + 1;
}

/**
 * This method computes x to the power of y (i.e. x^y)
 * @param x
 * @param y
 * @return
 */
int power(int x, int y) {
    int res = 1;
    for (int i = 1; i <= y; i++)
        res *= x;
    return res;
}

/**
 * This method receives part of a polynomial and computes the result with a global defined value
 * @param arg
 * @return
 */
void compute(char *str, int index) {
    pid_t pid = fork();
    if(pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    if(pid == 0) {
        int coeff = 1, pow;

        // extracting x's coefficient if exists
        unsigned long temp = strcspn(str, "*");
        if (temp != strlen(str)) {
            char coeffStr[5];
            strncpy(coeffStr, str, temp);
            coeffStr[temp] = '\0';
            coeff = atoi(coeffStr);
        }

        // extracting x's power
        temp = strcspn(str, "^");
        char powStr[5];
        strcpy(powStr, str + temp + 1);
        powStr[temp] = '\0';
        pow = atoi(powStr);
        shmPtr[index] = coeff * power(value, pow);
        exit(EXIT_SUCCESS);
    }
}

/**
 * This method checks if a string represent a polynomial in correct format
 * @return 1 if string is valid, 0 otherwise
 */
int isValid(char *str) {
    for (int i = 0; i < strlen(str) - 1; i++) {
        if (str[i] == 'x' && str[i + 1] != '^') {
            return FALSE;
        }
        if (str[i] == '*' && str[i + 1] != 'x') {
            return FALSE;
        }
        if (str[i] == '+' && str[i + 1] != 'x' && (str[i + 1] <= '0' || str[i + 1] >= '9')) {
            return FALSE;
        }
        if (str[i] >= '0' && str[i] <= '9' &&
            (str[i + 1] != '*' && str[i + 1] != '+' && (str[i + 1] <= '0' || str[i + 1] >= '9'))) {
            return FALSE;
        }
    }
    return TRUE;
}
