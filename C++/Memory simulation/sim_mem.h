#ifndef SIM_MEM_H
#define SIM_MEM_H

#define MEMORY_SIZE 200
extern char main_memory[MEMORY_SIZE];

typedef struct page_descriptor {
    int V;            // valid
    int D;            // dirty
    int P;            // permission
    int frame;            //the number of a frame if in case it is page-mapped
    int swap_index;        // where the page is located in the swap file.
} page_descriptor;

typedef struct page_connections {
    int process_id;
    int page_number;
} page_connections;


class sim_mem {
    int swapfile_fd;        //swap file fd
    int program_fd[2]{};        //executable file fd
    int text_size;
    int data_size;
    int bss_size;
    int heap_stack_size;
    int num_of_pages;
    int page_size;
    int num_of_proc;
    int pages_written;
    int swap_size;
    page_descriptor **page_table;    //pointer to page table
    page_connections *ramId;
    page_connections *swapId;

protected:
    void initialize_page_table(page_descriptor[]) const;
    void write_to_swap(int);

public:
    sim_mem(char exe_file_name1[], char exe_file_name2[],
            char swap_file_name2[], int text_size, int data_size,
            int bss_size, int heap_stack_size, int num_of_pages,
            int page_size, int num_of_process);

    ~sim_mem();

    char load(int process_id, int address);

    void store(int process_id, int address, char value);

    void print_memory();

    void print_swap();

    void print_page_table();

};

#endif
