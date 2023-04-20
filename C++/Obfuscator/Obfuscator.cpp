#include "Obfuscator.h"

// this method generate a random valid variable name 
string Obfuscator:: generateRandomVar() {
    string firstChar = "_AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz";
    string ascii = "0123456789abcdefghijklmnopqrstuvwxyz";
    string str;
    int randomNum = (rand()%5)+4;   // generate random number between 4 to 9
    
    str += firstChar[rand()%firstChar.length()];
    for(int i = 0; i<randomNum; i++)
    {
        str += ascii[rand()%ascii.length()];
    }
    return str;
}

// this method creats an array of strings, filling the array with rows from a code file given as reference
string* Obfuscator:: parseFile(string fileName) {
    ifstream infile;
    infile.open(fileName);
    this->countLines = 0;
    string line;
    while(getline(infile, line))
    {
        this->countLines++; // counting how many code rows are in the file
    }
    infile.close();
    
    string *ansArray = new string[this->countLines];
    infile.open(fileName);
    int i = 0;
    while(getline(infile, line)) // loop for filling the array
    {
        ansArray[i] = line;
        i++;
    }
    infile.close();
    return ansArray;
}

// this method open a new file and write to it from a given array of strings
void Obfuscator:: writeToFile(string* codeArray) {
    ofstream outfile;
    outfile.open(this->OUT);
    for(int i = 0; i < this->countLines; i++)
    {
        outfile<<codeArray[i]<<endl;
    }
    outfile.close();
}
