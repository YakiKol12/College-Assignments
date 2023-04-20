#ifndef OBFUSCATOR_H
#define OBFUSCATOR_H
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

class Obfuscator {
  protected:
    int countLines;
    
  public:
    const string OUT = "newCode.c";
    virtual void obfuscate(string fileName) = 0;
    string generateRandomVar();
    string* parseFile(string fileName);
    void writeToFile(string* codeArray);
};

#endif