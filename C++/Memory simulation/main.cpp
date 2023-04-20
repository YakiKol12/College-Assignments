#include <iostream>
#include "sim_mem.h"

#define MEMORY_SIZE 200

using namespace std;
char main_memory[MEMORY_SIZE];
int main() {
    srand(time(nullptr));
    char file1[] = "/home/student/CLionProjects/Ex5/exec_file1.txt";
    char file2[] = "/home/student/CLionProjects/Ex5/exec_file2.txt";
    char swap[] = "/home/student/CLionProjects/Ex5/swap_file.txt";
    sim_mem mem_sm(file1, file2,swap,25, 50, 25,25, 25, 5,2);


    for(int i = 0; i < 50; i++) {
        int val = rand() % 125;
        if(val % 3 == 0)
            mem_sm.store(1, val, '@');
        else
            mem_sm.store(2, val, '%');
    }

    for(int i = 0; i < 50; i++) {
        int val = rand() % 125;
        if(val % 3 == 0)
            mem_sm.load(2, val);
        else
            mem_sm.load(1, val);
    }


    mem_sm.print_memory();
    mem_sm.print_page_table();
    mem_sm.print_swap();
}