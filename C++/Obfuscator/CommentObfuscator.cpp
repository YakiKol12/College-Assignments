#include "CommentObfuscator.h"

// this method delete all comments from a given code and implants new comments in random places
void CommentObfuscator:: obfuscate(string fileName) {
    string *codeArray = parseFile(fileName);
    string str = "//";
    string str1 = "main";
    for(int i = 0; i < countLines; i++) // loop to delete all comments from the code
    {
        int temp = codeArray[i].find(str);
        if(temp != string::npos)
        {
            codeArray[i] = codeArray[i].substr(0, temp);
        }
    }
    int numOfComments = (rand()%6) + 1; // random number between 1 ot 6, how many comments will be implanted
    
    for(int i = 0; i < numOfComments; i++)
    {
        int choose = rand()%N;           // random comment from comments array of the class
        int line = (rand()%countLines-1) + 1; // random line number in the code
        int temp1 = codeArray[line].find(str);
        int temp2 = codeArray[line].find(str1);
        if(temp1 != string::npos || temp2 != string::npos) // making sure not to put comments on the same line and not in the main line
        {
            codeArray[line] += "\n//" + comments[choose];    
        }
        else
        {
            codeArray[line] += " //" + comments[choose];    
        }
    }
    writeToFile(codeArray);
    delete[] codeArray; // releasing aloocated memory for the array
}