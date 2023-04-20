#include "LineObfuscator.h"
#include "VarObfuscator.h"
#include "CommentObfuscator.h"
#include <string>
#include <iostream>
using namespace std;

int main() {
    srand(time(NULL));
    Obfuscator* obfs[3];
    obfs[0] = new VarObfuscator();
    obfs[1] = new LineObfuscator();
    obfs[2] = new CommentObfuscator();
    obfs[0]->obfuscate("test.txt");
    for(int i = 1; i < 3; i++) 
	    obfs[i]->obfuscate(obfs[i - 1]->OUT);
}
