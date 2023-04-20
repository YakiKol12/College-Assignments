#include "sim_mem.h"
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib>
#include <string>

using namespace std;

/**
 *  CONSTRUCTOR
 * @param exe_file_name1
 * @param exe_file_name2
 * @param swap_file_name
 * @param text_size
 * @param data_size
 * @param bss_size
 * @param heap_stack_size
 * @param num_of_pages
 * @param page_size
 * @param num_of_process
 */
sim_mem::sim_mem(char exe_file_name1[], char exe_file_name2[],
                 char swap_file_name[], int text_size, int data_size,
                 int bss_size, int heap_stack_size, int num_of_pages,
                 int page_size, int num_of_process) {

    this->text_size = text_size;
    this->data_size = data_size;
    this->bss_size = bss_size;
    this->heap_stack_size = heap_stack_size;
    this->page_size = page_size;
    this->num_of_pages = num_of_pages;
    this->num_of_proc = num_of_process;
    this->pages_written = 0;
    this->swap_size = num_of_process * (page_size * (num_of_pages - (text_size / page_size)));

    this->program_fd[0] = open(exe_file_name1, O_RDONLY);
    if (program_fd[0] == -1) {
        perror("open() failed (file 1)");
        exit(EXIT_FAILURE);
    }
    this->page_table = new page_descriptor *[num_of_process];
    this->page_table[0] = new page_descriptor[num_of_pages];
    initialize_page_table(page_table[0]);

    if (num_of_process == 2) {
        this->program_fd[1] = open(exe_file_name2, O_RDONLY);
        if (program_fd[1] == -1) {
            perror("open() failed (file 2)");
            exit(EXIT_FAILURE);
        }
        this->page_table[1] = new page_descriptor[num_of_pages];
        initialize_page_table(page_table[1]);
    }

    this->swapfile_fd = open(swap_file_name, O_RDWR | O_CREAT, 0644);
    if (swapfile_fd == -1) {
        perror("open() failed (swap)");
        exit(EXIT_FAILURE);
    }

    // init swap file
    for (int i = 0; i < this->swap_size; i++) {
        write(swapfile_fd, "0", 1);
    }

    // init main memory (RAM)
    for (char &i: main_memory) {
        i = '0';
    }

    // init ramId array - what currently in main memory
    this->ramId = new page_connections[MEMORY_SIZE / page_size];
    for (int i = 0; i < MEMORY_SIZE / page_size; i++) {
        ramId[i].process_id = -1;
        ramId[i].page_number = -1;
    }

    // init swapId array - what currently in swap
    this->swapId = new page_connections[swap_size / page_size];
    for (int i = 0; i < swap_size / page_size; i++) {
        swapId[i].process_id = -1;
        swapId[i].page_number = -1;
    }
}

/**
 *  DESTRUCTOR
 */
sim_mem::~sim_mem() {
    if (num_of_proc == 2) {
        close(program_fd[1]);
        delete[] page_table[1];
    }
    close(program_fd[0]);
    delete[] page_table[0];
    delete[] page_table;
    close(swapfile_fd);
    delete[] ramId;
    delete[] swapId;
}

/**
 * This method retrieves a wanted page from exec file / swap file and returns the desired char
 * @param process_id
 * @param address
 * @return
 */
char sim_mem::load(int process_id, int address) {
    process_id--;
    int page_num = address / this->page_size;
    int offset = address % this->page_size;

    if (page_num >= this->num_of_pages) {
        fprintf(stderr, "page out of scope\n");
        return '\0';
    }

    if (this->page_table[process_id][page_num].V == 1) { // page already in main memory
        return main_memory[this->page_table[process_id][page_num].frame * this->page_size + offset];
    }

    int frame = this->pages_written % (MEMORY_SIZE / this->page_size); // frame to insert new page to
    this->pages_written++;


    if (this->page_table[process_id][page_num].P == 0) { // if page is text
        if (this->ramId[frame].process_id != -1) { // if current memory slot is full
            write_to_swap(frame);
        }

        // copy from exec file
        lseek(this->program_fd[process_id], page_num * this->page_size, SEEK_SET);
        char tmp[this->page_size];
        if ((read(this->program_fd[process_id], tmp, this->page_size)) == -1) {
            perror("read() failed");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < this->page_size; i++) { // write to main memory
            main_memory[frame * this->page_size + i] = tmp[i];
        }
        this->ramId[frame].page_number = page_num;
        this->ramId[frame].process_id = process_id;
        this->page_table[process_id][page_num].V = 1;
        this->page_table[process_id][page_num].frame = frame;
        return main_memory[frame * this->page_size + offset];
    }

    if (this->page_table[process_id][page_num].D == 1) { // if page is in swap
        if (this->ramId[frame].process_id != -1) { // if current memory slot is full
            write_to_swap(frame);
        }

        // copy from swap
        int swap_index = this->page_table[process_id][page_num].swap_index;
        lseek(this->swapfile_fd, swap_index * this->page_size, SEEK_SET);
        char tmp[this->page_size];
        read(this->swapfile_fd, tmp, this->page_size);

        // write to main memory + delete from swap
        lseek(this->swapfile_fd, swap_index * this->page_size, SEEK_SET);
        for (int i = 0; i < this->page_size; i++) {
            write(this->swapfile_fd, "0", 1);
            main_memory[frame * this->page_size + i] = tmp[i];
        }
        this->page_table[process_id][page_num].V = 1;
        this->page_table[process_id][page_num].frame = frame;
        this->page_table[process_id][page_num].swap_index = -1;
        this->ramId[frame].process_id = process_id;
        this->ramId[frame].page_number = page_num;
        this->swapId[swap_index].process_id = -1;
        this->swapId[swap_index].page_number = -1;
        return main_memory[frame * this->page_size + offset];
    }

    // check for bss or heap / stack
    if (page_num >= num_of_pages - ((bss_size + heap_stack_size) / page_size)) {
        if (page_num >= num_of_pages - (heap_stack_size / page_size)) { // if heap/stack - error
            fprintf(stderr, "cannot access non allocated memory\n");
            this->pages_written--;
            return '\0';
        }

        // page is bss
        if (this->ramId[frame].process_id != -1) { // if current memory slot is full
            write_to_swap(frame);
        }
        // init new page to main memory - malloc
        for (int i = 0; i < this->page_size; i++) {
            main_memory[frame * this->page_size + i] = '0';
        }
        this->page_table[process_id][page_num].V = 1;
        this->page_table[process_id][page_num].frame = frame;
        this->page_table[process_id][page_num].swap_index = -1;
        this->ramId[frame].process_id = process_id;
        this->ramId[frame].page_number = page_num;
        return main_memory[frame * this->page_size + offset];
    }

    // here page is data
    if (this->ramId[frame].process_id != -1) { // if current memory slot is full
        write_to_swap(frame);
    }

    // copy from exec file
    lseek(this->program_fd[process_id], page_num * this->page_size, SEEK_SET);
    char tmp[this->page_size];
    if ((read(this->program_fd[process_id], tmp, this->page_size)) == -1) {
        perror("read() failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < this->page_size; i++) { // write to main memory
        main_memory[frame * this->page_size + i] = tmp[i];
    }
    this->ramId[frame].page_number = page_num;
    this->ramId[frame].process_id = process_id;
    this->page_table[process_id][page_num].V = 1;
    this->page_table[process_id][page_num].frame = frame;
    return main_memory[frame * this->page_size + offset];
}

/**
 * This method retrieves a wanted page from exec file / swap file and insert a value to a specific location
 * @param process_id
 * @param address
 * @param value
 */
void sim_mem::store(int process_id, int address, char value) {
    process_id--;
    int page_num = address / this->page_size;
    int offset = address % this->page_size;

    if (page_num >= this->num_of_pages) {
        fprintf(stderr, "page out of scope\n");
        return;
    }

    if (this->page_table[process_id][page_num].P == 0) { // no permission - text page - can't change
        fprintf(stderr, "cannot store on text pages\n");
        return;
    }

    if (this->page_table[process_id][page_num].V == 1) { // page already in main memory
        main_memory[this->page_table[process_id][page_num].frame * this->page_size + offset] = value;
        this->page_table[process_id][page_num].D = 1;
        return;
    }

    int frame = this->pages_written % (MEMORY_SIZE / this->page_size); // frame to insert new page to
    this->pages_written++;

    if (this->page_table[process_id][page_num].D == 1) { // dirty page - copy from swap
        if (this->ramId[frame].process_id != -1) { // if current memory slot is full
            write_to_swap(frame);
        }

        // copy from swap
        int swap_index = this->page_table[process_id][page_num].swap_index;
        lseek(this->swapfile_fd, swap_index * this->page_size, SEEK_SET);
        char tmp[this->page_size];
        read(this->swapfile_fd, tmp, this->page_size);

        // write to main memory + delete from swap
        lseek(this->swapfile_fd, swap_index * this->page_size, SEEK_SET);
        for (int i = 0; i < this->page_size; i++) {
            write(this->swapfile_fd, "0", 1);
            main_memory[frame * this->page_size + i] = tmp[i];
        }
        this->page_table[process_id][page_num].V = 1;
        this->page_table[process_id][page_num].frame = frame;
        this->page_table[process_id][page_num].swap_index = -1;
        this->ramId[frame].process_id = process_id;
        this->ramId[frame].page_number = page_num;
        this->swapId[swap_index].process_id = -1;
        this->swapId[swap_index].page_number = -1;
        main_memory[frame * this->page_size + offset] = value;
        return;
    }

    // check for bss or heap / stack - allocate new page - malloc
    if (page_num >= num_of_pages - ((bss_size + heap_stack_size) / page_size)) {
        if (this->ramId[frame].process_id != -1) { // if current memory slot is full
            write_to_swap(frame);
        }
        for (int i = 0; i < this->page_size; i++) {
            main_memory[frame * this->page_size + i] = '0';
        }
        this->page_table[process_id][page_num].V = 1;
        this->page_table[process_id][page_num].D = 1;
        this->page_table[process_id][page_num].frame = frame;
        this->page_table[process_id][page_num].swap_index = -1;
        this->ramId[frame].process_id = process_id;
        this->ramId[frame].page_number = page_num;
        main_memory[frame * this->page_size + offset] = value;
        return;
    }

    // here page is data
    if (this->ramId[frame].process_id != -1) { // if current memory slot is full
        write_to_swap(frame);
    }
    //copy from exec file
    lseek(this->program_fd[process_id], page_num * this->page_size, SEEK_SET);
    char tmp[this->page_size];
    if ((read(this->program_fd[process_id], tmp, this->page_size)) == -1) {
        perror("read() failed");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < this->page_size; i++) {
        main_memory[frame * this->page_size + i] = tmp[i];
    }
    this->ramId[frame].page_number = page_num;
    this->ramId[frame].process_id = process_id;
    this->page_table[process_id][page_num].V = 1;
    this->page_table[process_id][page_num].D = 1;
    this->page_table[process_id][page_num].frame = frame;
    main_memory[frame * this->page_size + offset] = value;
}

/**
 * This method resets the values of a page table to default
 * @param table
 */
void sim_mem::initialize_page_table(page_descriptor table[]) const {
    for (int i = 0; i < num_of_pages; i++) {
        table[i].V = 0;
        table[i].D = 0;
        table[i].frame = -1;
        table[i].swap_index = -1;
        if (i < text_size / page_size)
            table[i].P = 0;
        else
            table[i].P = 1;
    }
}

/**
 * This method receive a frame number and stores copies its content to the swap file
 * @param frame
 */
void sim_mem::write_to_swap(int frame) {
    if (this->page_table[ramId[frame].process_id][ramId[frame].page_number].D == 1) { // dirty file move to swap
        int swap_index = 0;
        while (this->swapId[swap_index].process_id != -1) {
            swap_index++;
        }
        lseek(this->swapfile_fd, swap_index * this->page_size, SEEK_SET);
        write(this->swapfile_fd, &main_memory[frame * this->page_size], this->page_size);
        this->swapId[swap_index].process_id = this->ramId[frame].process_id;
        this->swapId[swap_index].page_number = this->ramId[frame].page_number;
        this->page_table[this->ramId[frame].process_id][this->ramId[frame].page_number].swap_index = swap_index;
    }
    // update deleted page descriptor
    this->page_table[this->ramId[frame].process_id][this->ramId[frame].page_number].V = 0;
    this->page_table[this->ramId[frame].process_id][this->ramId[frame].page_number].frame = -1;
}

/**************************************************************************************/
void sim_mem::print_memory() {
    int i;
    printf("\n Physical memory\n");
    for (i = 0; i < MEMORY_SIZE; i++) {
        printf("[%c]\t", main_memory[i]);
        if ((i + 1) % page_size == 0)
            printf("\n");
    }
}

/************************************************************************************/
void sim_mem::print_swap() {
    char *str = (char *) malloc(this->page_size * sizeof(char));
    int i;
    printf("\n Swap memory\n");
    lseek(swapfile_fd, 0, SEEK_SET);    // go to the start of the file
    while (read(swapfile_fd, str, this->page_size) == this->page_size) {
        for (i = 0; i < page_size; i++) {
            printf("%d - [%c]\t", i, str[i]);
        }
        printf("\n");
    }
    free(str);
}

/***************************************************************************************/
void sim_mem::print_page_table() {
    int i;
    for (int j = 0; j < num_of_proc; j++) {
        printf("\n page table of process: %d \n", j);
        printf("Valid\t Dirty\t Permission\t Frame\t Swap index\n");
        for (i = 0; i < num_of_pages; i++) {
            printf("[%d]\t [%d]\t [%d]\t\t [%d]\t [%d]\n",
                   page_table[j][i].V,
                   page_table[j][i].D,
                   page_table[j][i].P,
                   page_table[j][i].frame, page_table[j][i].swap_index);
        }
    }
}
