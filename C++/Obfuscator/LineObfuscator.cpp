#include "LineObfuscator.h"

// this method creats a random name variable, implant it in a given strings array and give a value to it in a different part of the array
void LineObfuscator:: obfuscate(string fileName) {
    string *codeArray = parseFile(fileName); 
    string randomVar = generateRandomVar();
    int randomVarInd = 0;
    string str1 = "main";
    string str2 = "{";
    string str3 = "for";
    string str4 = "while";
    string str5 = "if";
    string str6 = "else";
    string str7 = "}";
    
    // loop to implant a random var after main line
    for(int i = 0, j = 0; i < countLines; i++, j++)
    {
        int found1 = codeArray[i].find(str1);
        int found2 = codeArray[i].find(str2);
        if(found1 != string::npos)
        {
            randomVarInd = i;                              
        }
        if(found2 != string::npos)
        {
            codeArray[i] += "\n   int " + randomVar + ";";
        }
    }
    
    int temp = ( (rand()%((countLines-1)-randomVarInd)+1) + (randomVarInd+1));
    int found3 = codeArray[temp].find(str3);
    int found4 = codeArray[temp].find(str4);
    int found5 = codeArray[temp].find(str5);
    int found6 = codeArray[temp].find(str6);
    int found7 = codeArray[temp].find(str7);
    if(found3 != string::npos || found4 != string::npos || found5 != string::npos || found6 != string::npos)
    {
        codeArray[temp+1] += "\n   " + randomVar + " = " + to_string(rand()%2000) + " + " + to_string(rand()%500) + ";";
    }
    else if(found7 != string::npos)
    {
        codeArray[temp-1] += "\n   " + randomVar + " = " + to_string(rand()%2000) + " + " + to_string(rand()%500) + ";";
    }
    else
    {
        codeArray[temp] += "\n   " + randomVar + " = " + to_string(rand()%2000) + " + " + to_string(rand()%500) + ";";    
    }
    writeToFile(codeArray);
    delete[] codeArray; // releasing aloocated memory for the array
}




