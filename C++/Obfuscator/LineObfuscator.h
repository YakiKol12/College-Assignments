#ifndef LINE_OBFUSCATOR_H
#define LINE_OBFUSCATOR_H
#include "Obfuscator.h"

class LineObfuscator: public Obfuscator {
    public:
        void obfuscate(string fileName);
};
#endif