#ifndef VAR_OBFUSCATOR_H
#define VAR_OBFUSCATOR_H
#include "Obfuscator.h"

class VarObfuscator: public Obfuscator {
    public:
        void obfuscate(string fileName);
};
#endif