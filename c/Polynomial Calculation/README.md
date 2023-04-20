# Polynomial calculation
Authored by Yaki Kol

## Description
This program receives a polynomial and a value to insert, prints the result to the terminal.<br>
Calculation executed in parallel: (a) using threads. (b) using processes with shared memory.<br>
The code runs in a loop asking for computation problem, to exit enter "done".

## functions

### Part a
1.	isNumber – This method receives a string and checks if it represents a number.<br>
2.	countThreads – This method counts how many threads we need to compute the polynomial, returns number of '+' chars + 1.<br>
3.	Power – This method receives two numbers, x and y returning x to the y power (x ^ y).<br>
4.	Compute – This method receives part of a polynomial and computes its result, this done using a global value to insert. This method is called in parallel for every part of the polynomial that needs computation using threads.<br>
5.	isValid – This method receives a string and checks if it represents a polynomial in the correct format (determined in the assignment).<br>

### Part b
In this part the only difference in functions is in Compute, the function also receives an index, creating a child process to compute the polynomial and the result is assigned to a shared memory array at the given index. At the end of the main function the shared memory is scanned summing all its elements.

## Program Files
ex4a.c – part one.<br>
ex4b.c – part two.

## How to run 
compile: gcc ex4a.c -o ex4a -lpthread<br>
gcc ex4a.c -o ex4b<br>
run: ./ex4a<br>
./ex4b

## Input
Polynomial and value to insert in format: "<Poly>, <val>".

## Output
Computation value to the terminal.
