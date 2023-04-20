# Memory simulation
Authored by Yaki Kol

## Description
This class creates an object supporting methods that simulate the process of memory accessing using paging method.<br>
The object receives one or two "exec" files (.txt), it creates a "main memory" array (RAM).<br>
The main program loops "load" and "store" commands – getting the right page to the main memory and managing its capacity in FIFO. Once full changed pages are written to a swap file in "first fit" approach.<br>
### functions
1.	Constructor.<br>
2.	Destructor.<br>
3.	Load – This method reads a page to the main memory and returns the desired character from the exec file.<br>
4.	Store – This method reads a page to the main memory and stores a new value to the page in a specific index (offset).<br>
In both load and store if the "current frame" is occupied I check if the current page is "dirty" and if so, write it to the swap file. The "current frame" is determined using FIFO.<br>
5.	Initislize_page_table – This method resets the values of a page table to default.<br>
6.	Write_to_swap – this method writes a page to the swap file.<br>
7.	Print_memory – this method prints the main memory to the terminal.<br>
8.	Print_swap – this method prints the content of the swap file to the terminal.<br>
9.	Print_page_table – this method prints the page table to the terminal.<br>

## Program Files
Main.cpp – tester.<br>
sim_mem.h – header file.<br>
sim_mem.cpp - methods file.

## How to run
compile: g++ main.cpp sim_mem.cpp -o main<br>
run: ./main
       
## Input
one or two text files.
