#include "VarObfuscator.h"

// this method chooses a random int variable in a given code and replaces its name throughout the code
void VarObfuscator:: obfuscate(string fileName) {
    string *codeArray = parseFile(fileName);
    int codeIndexes[countLines];
    string str = "int";
    string str1 = "main";
    string str2 = "for";
    
    // creat an array representing indexes of the code array given to the function
    for(int i = 0; i < countLines; i++)
    {
        int temp = codeArray[i].find(str);
        if(temp != string::npos)
        {
            int temp1 = codeArray[i-1].find(str1);
            int temp2 = codeArray[i].find(str2);
            if(temp1 != string::npos || temp2 != string::npos)
            {
                codeIndexes[i] = 0;
            }
            else if(temp == 0 || (temp > 0 && (codeArray[i][temp-1] == ' ' || codeArray[i][temp-1] == '(')))
            {
                codeIndexes[i] = 1; // if this line of code contains an int variable, flag it with '1'
            }
            else
            {
                codeIndexes[i] = 0; // if not, flag it with '0'     
            }
        }
        else
        {
            codeIndexes[i] = 0; // if not, flag it with '0'     
        }
    }
    
    string varToChange = "";
    bool flag = false;
    while(!flag)
    {
        int temp = (rand()%countLines) + 1;
        if(codeIndexes[temp] == 1)
        {
            varToChange = codeArray[temp].substr(codeArray[temp].find(str)+str.length()+1, codeArray[temp].length()-codeArray[temp].find(str)-str.length()-2);
            flag = true;
        }
    }
    
    string changedVar = generateRandomVar();
    for(int i = 0; i < countLines; i++)
    {
        int temp = codeArray[i].find(varToChange);
        while(temp != string::npos)
        {
            codeArray[i].replace(temp, varToChange.length(), changedVar);
            temp = codeArray[i].find(varToChange, temp+1);
        }
    }
    writeToFile(codeArray);
    delete[] codeArray; // releasing aloocated memory for the array
}