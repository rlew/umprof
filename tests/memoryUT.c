#include "memseg.h"
#include <stdio.h>

int main(){
    Mem* memory = newMem();
    instantiateMem(memory, 500);
    
    return 0;
}
