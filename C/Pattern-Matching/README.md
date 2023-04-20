# Pattern Matching
Authored by Yaki Kol

## Description 
This program finds set patterns inside a given text and notifies which patterns were found and where they were found (from index to index)

## Main data structurs 
1. generic doubly linked list.<br>
2. pattern matching state machine (tree).<br>
3. list of pm_match (struct).

## Main functions 
1. pm_init - 	initializing the data structure that stores the patterns.<br>
2. pm_addstring - adds a pattern to the data structure.<br>
3. pm_makeFSM - should be used after all the patterns were added to the data structure, finalizing the data structure by connecting all its states to their failure states.<br>
4. pm_fsm_search - going over the given text and finding all the patterns within it. returns list of pm_match, each match is a pattern found in the text.<br>
5. pm_destroy -	destroying the data structure of the patterns. 

## Program Files
dbllist.h - contains structs and declerations of functions of the doubly linked list.<br>
dbllist.c - the file containing the implementations of the functions.<br>
pattern_matching.h - contains structs and declarations of methods that are linked to the pattern matching algorithm.<br>
pattern_matching.c - containing the implementations of the functions.

## How to compile
assuming the main file is called 'main.c':<br>
compile: gcc main.c dbllist.c pattern_matching.c -o main<br>
run: ./main

## Input
no input files

## Output
console log
