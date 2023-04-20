# Disk simulation
Authored by Yaki Kol

## Description

The program is a simulation of disk memory management, we use the "Index allocation" method that reads from the disk in two levels, accessing the file's index block and from there accessing its data blocks.<br>
The disk is divided into blocks, the size is determined when calling format function. These blocks are filled with data by need when writing to files.<br>
In this program the simulation will be implemented by a few functions, creating files, write and read data, open and close files and deleting files from the Hard disk (file in the directory).<br>
### Program DATABASE
1. FsFile - struct contains information on a specific file (represent a file in the directory).<br>
2. FileDescriptor - struct contains FsFile, file name (string) and inUse (bool).<br>
3. MainDir – vector of type FileDescriptor*, represent all the current files in the directory.<br>
4.OpenFileDescriptoprs – map of type <int, FileDescriptor*>, represent all the currently open files in the system.<br>

### functions
1.listAll - this method prints details of all the files and the content of the disk (sim disk file).<br>
2. fsFormat – this method clears all data structures of the disk and clear the disk content.<br>
3. CreateFile - this method creates a new file in the directory.<br>
4. OpentFile – this method opens an existing file and assigns fd (int) to it.<br>
5. CloseFile – this method closes an open file erasing it from the OpenFileDiscriptors.<br>
6. WriteToFile – this method writes data to a given file in disk.<br>
7. ReadFromFile – this method reads data of a given file from the disk.<br>
8. DelFile – this method deletes a file from the disk.

## Program Files
FsFile.h - the file contains the FsFile class.<br>
FileDescriptor.h – the file contains the file descriptor class.<br>
fsDisk.h- a header file, contains structs, declarations of functions.<br>
fsDisk.cpp – the file contains the functions. <br>
main.cpp- contain the main only.<br>

## How to run
compile: gcc main.cpp fsDisk.cpp -o ex6<br>
run: ./ex6

## Input
no input

## Output
main dir content<br>
Disk content
