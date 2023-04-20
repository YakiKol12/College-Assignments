#ifndef COMMENT_OBFUSCATOR_H
#define COMMENT_OBFUSCATOR_H
#include "Obfuscator.h"

class CommentObfuscator: public Obfuscator {
    private:
        static const int N = 11;
        string comments[N] = {"checking if num is valid", "reading file to an array", "loop to read info from user",
            "replacing i with j", "if u got to this line, ur awesome", "recursive method to find length between two nodes",
            "add i", "Be yourself; everyone else is already taken.", "Without music, life would be a mistake.",
            "The older you get, the better you get, unless you’re a banana", "if(true == false) -> go check urself"};
    public:
        void obfuscate(string fileName);
};
#endif