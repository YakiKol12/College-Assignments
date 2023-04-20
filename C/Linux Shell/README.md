# Linux shell
Authored by Yaki Kol

## Description

This program “mimics” the behavior of the Linux shell terminal, running basic Linux commands.<br>
In a main loop the program:<br>
-	Prints current directory.<br>
-	Read a command from the user as a string.<br>
-	Creates an array of strings from the input.<br>
-	Creates a new process and run the command entered. <br>
-	To end the loop and exit the program, type “done” – this will print the number of commands entered and how many pipes were received. <br>
-	Printing history of commands using the “history” keyword.<br>
-	Supporting “history” functions, enabling the use of “!<n>” commands to run previous commands <br>
-	Supporting "piped" ('|') commands (up to two pipes).<br>
-	Supporting '&' - executing commands in the "background".<br>
-	Supporting 'nohup' commands - creating an output file and ignoring SIGHUP to allow processes to continue after shell close.<br>
  
### functions:
1.	createCommand – this method receives a string representing a command and creates an array of strings, word by word with NULL at the last place (execvp format array).<br>
2.	countWords – this method receives a string and counts how many words it contains.<br>
3.	freeCommand – this method frees all allocated memory for the command array.<br>
4.	runCommand – this method creates a new process and runs the command entered.<br>
5.	printDirectory – this method prints current directory to the console.<br>
6.	findInHistory – this method receives a string in “! <n>” format where n is the line number in the history file containing a command to run.<br>
7.	printHistory – this method receives a file pointer, read the file line by line printing each line to the terminal.<br>
8.	splitCommands – this method receives a string with '|' character and divides it to two new strings before the '|' and after it (split pipe command to two separate commands).<br>
9.	pipedCommand – this method receives a string, return 1 if the string contains the '|' char.<br>
10.	runPipe – this method receives a piped command and manages its running, splitting the string into two or three new command strings, calling the accurate run function (one or two pipes).<br>
11.	execPipe – this method runs a piped command, creating two new processes, redirecting first command's output and second command's input to a pipe.<br>
12.	execTwoPipe – this method runs a dual pipe command, creating three new processes linked via two pipes.<br>
13.	sigHandler – this method replaces the default managing of CIGCHLD.<br>
14.	findAmp – this method receives a string, return 1 if the string contains the '&' char.<br>
15.	deleteAmp – this method deletes the '&' and all spaces before it from a given string.<br>
  
## Program Files
Ex3.c – main program. 

## How to compile?
compile: gcc Ex3.c -o main<br>
run: ./main

## Input
Command to run as a string

## Output
Commands count<br>
Pipes count<br>
History file<br>
Nohup file<br>
Basic commands output to the console<br>
