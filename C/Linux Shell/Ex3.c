// Author: Yaki Kol
// 315429019
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <wait.h>

#define TRUE 1
#define FALSE 0
#define COMMAND_MAX 512
#define PATH_MAX 4096

char **createCommand(char *, int);

int countWords(char *);

void freeCommand(char **, int);

void runCommand(char *, FILE *);

void printDirectory();

void findInHistory(FILE *, char[]);

void printHistory(FILE *);

void splitCommands(char[], char *, char *);

int pipedCommand(char *);

void runPipe(char[], int *, FILE *);

void execPipe(char *, char *, FILE *);

void execTwoPipe(char *, char *, char *, FILE *);

void sigHandler(int);

int findAmp(char *);

void deleteAmp(char *);

int numOfCommands = 0;

int main() {
    char stringCommand[COMMAND_MAX];
    int numOfPipes = 0;

    FILE *fp = fopen("file.txt", "a+"); // opening history file
    if (fp == NULL) {
        perror("fopen() failed");
        exit(1);
    }
    signal(SIGCHLD, sigHandler);

    while (TRUE) {
        printDirectory();

        fgets(stringCommand, COMMAND_MAX, stdin);
        stringCommand[strlen(stringCommand) - 1] = '\0';

        // check if command given is valid
        if (stringCommand[0] == ' ' || stringCommand[strlen(stringCommand - 1)] == ' ') {
            printf("Command is not valid");
            continue;
        }

        // check for piped command
        if (pipedCommand(stringCommand)) {
            runPipe(stringCommand, &numOfPipes, fp);
            continue;
        }

        // check if user called a history command
        if (stringCommand[0] == '!') {
            if (stringCommand[1] == ' ') {
                fprintf(stderr, "Command is not valid");
                continue;
            }
            findInHistory(fp, stringCommand);
            if (pipedCommand(stringCommand)) {
                runPipe(stringCommand, &numOfPipes, fp);
                continue;
            }
        }

        if (strcmp(stringCommand, "cd") == 0) {
            printf("Command not supported (Yet)\n");
            numOfCommands++;
            continue;
        }

        if (strcmp(stringCommand, "done") == 0) {
            printf("Number of commands: %d\n", numOfCommands + 1);
            printf("Number of pipes: %d\n", numOfPipes);
            fclose(fp);
            exit(0);
        }

        if (strcmp(stringCommand, "history") == 0) {
            fprintf(fp, "%s\n", stringCommand);
            printHistory(fp);
            numOfCommands++;
            continue;
        }

        runCommand(stringCommand, fp);
    }
}

/**
 * this function creates an array of strings for execvp format
 * @param stringCommand
 * @param words
 * @return pointer to array of strings
 */
char **createCommand(char *stringCommand, int words) {
    char **argv = (char **) malloc((sizeof(char *) * words) + 1);
    if (argv == NULL) {
        perror("malloc() failed");
        exit(1);
    }

    int inWord = TRUE;
    // in this loop we allocate a string for every word in the command given by the user and copy it
    for (int i = 0, j = 0, k = 0; i < strlen(stringCommand); j++) {
        if (stringCommand[j] != ' ' && !inWord) {
            inWord = TRUE;
        } else if (stringCommand[j] == ' ') {
            if (!inWord) {
                i++;
                continue;
            } else {
                argv[k] = (char *) malloc(sizeof(char) * (j - i + 1));
                if (argv[k] == NULL) {
                    perror("malloc() failed");
                    exit(1);
                }
                strncpy(argv[k], stringCommand + i, j - i);
                argv[k][j - i] = '\0';
                i = j + 1;
                k++;
                inWord = FALSE;
            }
        }
    }

    argv[words] = NULL;
    return argv;
}

/**
 * this method counts how many words are in a given string
 * @param str
 * @return number of words in @str
 */
int countWords(char *str) {
    int countWords = 0;
    int inWord = FALSE;

    // in this loop we check every letter for spaces, maintaining a flag for "am I counting a word"
    for (int i = 0; i <= strlen(str); i++) {
        if (str[i] == ' ') {
            // if we found a space, and we were counting a word it means it's over than we increase the count
            if (inWord) {
                countWords++;
                inWord = FALSE;
            }
        } else if (str[i] == '\0') {
            if (inWord) {
                countWords++;
            }
        } else {
            if (!inWord) {
                inWord = TRUE;
            }
        }
    }
    return countWords;
}

/**
 * this method frees all allocated memory for argv
 * @param argv
 * @param words
 */
void freeCommand(char **argv, int words) {
    for (int i = 0; i < words; i++)
        if (argv[i] != NULL)
            free(argv[i]);
    free(argv);
}

/**
 * this method receives a command to run as a string, creates a new process and runs it
 * @param sCommand
 * @param fp
 */
void runCommand(char *sCommand, FILE *fp) {
    int inBack = findAmp(sCommand); // checking if there is '&'
    int status = 0;

    pid_t x = fork();
    if (x == -1) {
        perror("fork() failed");
        exit(1);
    }

    if (x == 0) {
        if (inBack) {
            deleteAmp(sCommand);
        }
        int words = countWords(sCommand);
        if (words == 0) {
            return;
        }
        char **command = createCommand(sCommand, words);

        if (strcmp(command[0], "nohup") == 0) {
            for (int i = 0; i <= words; i++) {
                command[i] = command[i + 1];
            }
            // creating nohup file
            int fd = open("nohup.txt",
                          O_WRONLY | O_CREAT | O_APPEND,
                          S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

            // redirecting output to new file
            if (dup2(fd, STDOUT_FILENO) == -1) {
                fprintf(stderr, "dup2() failed");
                exit(EXIT_FAILURE);
            }
            close(STDIN_FILENO);
            signal(SIGHUP, SIG_IGN);
        }
        if (execvp(command[0], command) == -1) {
            perror("execvp() failed");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    if (!inBack) {
        pause();
    }
    fprintf(fp, "%s\n", sCommand);
    numOfCommands++;
}

/**
 * this method get the current directory and print it to console
 */
void printDirectory() {
    char filePath[PATH_MAX];
    if (getcwd(filePath, sizeof(filePath)) == NULL) {
        perror("getcwd() error");
        exit(1);
    }
    printf("%s>", filePath); // print current directory
}

/**
 * this method searches the history file for the desired command
 * @param fp
 * @param str
 */
void findInHistory(FILE *fp, char str[]) {

    // finding the line number the user asked for
    strncpy(str, str + 1, strlen(str));
    char *ptr;
    int line = (int) strtol(str, &ptr, 10);
    if (line == 0) {
        fprintf(stderr, "strtol failed");
        return;
    }
    rewind(fp);

    // in this loop we go through the file until the command asked for
    int i = 1;
    while (i <= line && fgets(str, COMMAND_MAX, fp)) {
        i++;
    }
    if (i <= line) {
        printf("NOT IN HISTORY\n");
        return;
    }
    str[strlen(str) - 1] = '\0';
}

/**
 * this function prints the history file to the terminal
 * @param fp
 */
void printHistory(FILE *fp) {
    char line[COMMAND_MAX];
    rewind(fp); // resets the reading file pointer to the start
    int i = 1;
    while (fgets(line, sizeof(line), fp)) {
        printf("%d. %s", i, line);
        i++;
    }
}

/**
 * This method splits a given string in two by '|' character
 * @param sentence
 * @param firstCommand
 * @param secondCommand
 */
void splitCommands(char sentence[], char *firstCommand, char *secondCommand) {
    int pipe = 0;
    while (sentence[pipe] != '|')
        pipe++;

    strncpy(firstCommand, sentence, pipe);
    strncpy(secondCommand, sentence + pipe + 1, strlen(sentence) - pipe);

    firstCommand[pipe] = '\0';
    int i = pipe - 1;
    while (firstCommand[i] == ' ') {
        firstCommand[i--] = '\0';
    }
    i = 0;
    while (secondCommand[i] == ' ') {
        memmove(secondCommand, secondCommand + 1, strlen(secondCommand));
    }
}

/**
 * this method checks if a given string contains the "pipe" character '|'
 * @param sCommand
 * @return 1 (true) if the string contains '|'
 */
int pipedCommand(char *sCommand) {
    for (int i = 0; i < strlen(sCommand); i++) {
        if (sCommand[i] == '|') {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * This method manages the running of piped commands
 * @param stringCommand
 * @param numOfPipes
 * @return end status of the final command, -1 if failed
 */
void runPipe(char stringCommand[], int *numOfPipes, FILE *fp) {
    char firstStringCommand[COMMAND_MAX];
    char secondStringCommand[COMMAND_MAX];
    char thirdStringCommand[COMMAND_MAX];
    int pipes = 1;

    splitCommands(stringCommand, firstStringCommand, secondStringCommand);

    if (pipedCommand(secondStringCommand)) {
        strcpy(stringCommand, secondStringCommand);
        splitCommands(stringCommand, secondStringCommand, thirdStringCommand);
        pipes++;
    }

    if (firstStringCommand[0] == '!') {
        findInHistory(fp, firstStringCommand);
        if (pipedCommand(firstStringCommand)) {
            strcpy(thirdStringCommand, secondStringCommand);
            strcpy(stringCommand, firstStringCommand);
            splitCommands(stringCommand, firstStringCommand, secondStringCommand);
            pipes++;
        }
    }

    if (secondStringCommand[0] == '!') {
        findInHistory(fp, secondStringCommand);
        if (pipedCommand(secondStringCommand)) {
            strcpy(stringCommand, secondStringCommand);
            splitCommands(stringCommand, secondStringCommand, thirdStringCommand);
            pipes++;
        }
    }

    if (thirdStringCommand[0] == '!') {
        findInHistory(fp, thirdStringCommand);
        if (pipedCommand(thirdStringCommand)) {
            strcpy(stringCommand, thirdStringCommand);
            splitCommands(stringCommand, firstStringCommand, secondStringCommand);
            pipes++;
        }
    }

    if (pipes == 1) {
        execPipe(firstStringCommand, secondStringCommand, fp);
        *numOfPipes += 1;
    }
    else if (pipes == 2) {
        execTwoPipe(firstStringCommand, secondStringCommand, thirdStringCommand, fp);
        *numOfPipes += 2;
    }
    else {
        fprintf(stderr, "Too many pipes..");
        return;
    }
    if (pipes == 1)
        fprintf(fp, "%s|%s\n", firstStringCommand, secondStringCommand);
    else
        fprintf(fp, "%s|%s|%s\n", firstStringCommand, secondStringCommand, thirdStringCommand);
    numOfCommands++;
}

/**
 * This method creates two new processes and runs two commands in pipe configuration
 * @param firstStringCommand
 * @param secondStringCommand
 * @param fp
 */
void execPipe(char *firstStringCommand, char *secondStringCommand, FILE *fp) {
    int status = 0;
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("pipe() failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();

    if (pid1 < 0) {
        perror("fork() failed");
        exit(EXIT_FAILURE);
    }

    // son1 - executes first command (output goes to pipe)
    if (pid1 == 0) {
        int firstWords = countWords(firstStringCommand);
        char **firstCommand = createCommand(firstStringCommand, firstWords);

        // redirecting std-output to pipe
        if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
            fprintf(stderr, "dup2 failed");
        }
        close(pipe_fd[0]);
        close(pipe_fd[1]);

        if (strcmp(firstCommand[0], "history") == 0) {
            printHistory(fp);
        }
        else if (execvp(firstCommand[0], firstCommand) == -1) {
            perror("execvp() failed");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork() failed");
        exit(EXIT_FAILURE);
    }

    // son2 - executes second command (input comes from pipe)
    if (pid2 == 0) {
        int secondWords = countWords(secondStringCommand);
        char **secondCommand = createCommand(secondStringCommand, secondWords);

        if (strcmp(secondCommand[0], "nohup") == 0) {
            for (int i = 0; i <= secondWords; i++) {
                secondCommand[i] = secondCommand[i + 1];
            }
            // creating nohup file
            int fd = open("nohup.txt",
                          O_WRONLY | O_CREAT | O_APPEND,
                          S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

            // redirecting output to new file
            if (dup2(fd, STDOUT_FILENO) == -1) {
                fprintf(stderr, "dup2() failed");
                exit(EXIT_FAILURE);
            }
            close(STDIN_FILENO);
            signal(SIGHUP, SIG_IGN);
        }

        // redirecting std-input to pipe
        if (dup2(pipe_fd[0], STDIN_FILENO) == -1) {
            fprintf(stderr, "dup2 failed");
        }
        close(pipe_fd[0]);
        close(pipe_fd[1]);

        if (execvp(secondCommand[0], secondCommand) == -1) {
            perror("execvp() failed");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    waitpid(pid2, NULL, 0);
}

/**
 * This method creates three new processes and runs three commands in dual pipe configuration
 * @param firstStringCommand
 * @param secondStringCommand
 * @param thirdStringCommand
 * @param fp
 */
void execTwoPipe(char *firstStringCommand, char *secondStringCommand, char *thirdStringCommand, FILE *fp) {
    int pipe_fd_1[2];
    int pipe_fd_2[2];
    if (pipe(pipe_fd_1) == -1 || pipe(pipe_fd_2) == -1) {
        perror("pipe() failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();

    if (pid1 < 0) {
        perror("fork() failed");
        exit(EXIT_FAILURE);
    }
    // son1 - executes first command (output goes to pipe1)
    if (pid1 == 0) {
        int firstWords = countWords(firstStringCommand);
        char **firstCommand = createCommand(firstStringCommand, firstWords);
        if (dup2(pipe_fd_1[1], STDOUT_FILENO) == -1) {
            fprintf(stderr, "dup2 failed");
            exit(EXIT_FAILURE);
        }
        close(pipe_fd_1[0]);
        close(pipe_fd_1[1]);
        close(pipe_fd_2[0]);
        close(pipe_fd_2[1]);
        if (strcmp(firstCommand[0], "history") == 0) {
            printHistory(fp);
        } else if (execvp(firstCommand[0], firstCommand) == -1) {
            perror("execvp() failed");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }

    pid_t pid2 = fork();

    if (pid2 < 0) {
        perror("fork() failed");
        exit(EXIT_FAILURE);
    }
    // son2 - executes second command (input comes from pipe1, output goes to pipe2)
    if (pid2 == 0) {
        int secondWords = countWords(secondStringCommand);
        char **secondCommand = createCommand(secondStringCommand, secondWords);
        // redirecting: input from pipe1, output to pipe2
        int value1 = dup2(pipe_fd_1[0], STDIN_FILENO);
        int value2 = dup2(pipe_fd_2[1], STDOUT_FILENO);
        if (value1 == -1 || value2 == -1) {
            fprintf(stderr, "dup2 failed");
            exit(EXIT_FAILURE);
        }
        close(pipe_fd_1[0]);
        close(pipe_fd_1[1]);
        close(pipe_fd_2[0]);
        close(pipe_fd_2[1]);
        if (execvp(secondCommand[0], secondCommand) == -1) {
            perror("execvp() failed");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }

    pid_t pid3 = fork();

    if (pid3 < 0) {
        perror("fork() failed");
        exit(EXIT_FAILURE);
    }
    // son3 - executes third command (input comes from pipe2)
    if (pid3 == 0) {
        int thirdWords = countWords(thirdStringCommand);
        char **thirdCommand = createCommand(thirdStringCommand, thirdWords);

        if (strcmp(thirdCommand[0], "nohup") == 0) {
            for (int i = 0; i <= thirdWords; i++) {
                thirdCommand[i] = thirdCommand[i + 1];
            }
            // creating nohup file
            int fd = open("nohup.txt",
                          O_WRONLY | O_CREAT | O_APPEND,
                          S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

            // redirecting output to new file
            if (dup2(fd, STDOUT_FILENO) == -1) {
                fprintf(stderr, "dup2() failed");
                exit(EXIT_FAILURE);
            }
            close(STDIN_FILENO);
            signal(SIGHUP, SIG_IGN);
        }

        if (dup2(pipe_fd_2[0], STDIN_FILENO) == -1) {
            fprintf(stderr, "dup2 failed");
            exit(EXIT_FAILURE);
        }
        close(pipe_fd_1[0]);
        close(pipe_fd_1[1]);
        close(pipe_fd_2[0]);
        close(pipe_fd_2[1]);
        if (execvp(thirdCommand[0], thirdCommand) == -1) {
            perror("execvp() failed");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }

    close(pipe_fd_1[0]);
    close(pipe_fd_1[1]);
    close(pipe_fd_2[0]);
    close(pipe_fd_2[1]);
    waitpid(pid3, NULL, 0);
}

/**
 * signal handler for SIGCHLD to release processes
 * @param signum
 */
void sigHandler(int signum) {
    waitpid(-1, NULL, WNOHANG);
}

/**
 * this method checks if a string contains the '&' character
 * @param command
 * @return
 */
int findAmp(char *command) {
    for (int i = 0; i < strlen(command); i++)
        if (command[i] == '&')
            return TRUE;
    return FALSE;
}

/**
 * this method removes the '&' character from a string
 * @param command
 */
void deleteAmp(char *command) {
    int i;
    for (i = 0; i < strlen(command); i++) {
        if (command[i] == '&')
            command[i] = '\0';
    }
    while (command[--i] == ' ') {
        command[i] = '\0';
    }
}