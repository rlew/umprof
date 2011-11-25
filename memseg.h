#ifndef MEMSEG_INCLUDED
#define MEMSEG_INCLUDED
#include <stdlib.h>
#include <stdint.h>
#include "seq.h"
#include "uarray.h"

typedef uint32_t UM_Word;
typedef struct Mem Mem;

Mem* newMem();
void loadSegment(Mem* memorySegments, UM_Word ID);
UM_Word getWord(Mem* memorySegments, UM_Word ID, UM_Word offset);
int segmentLength(Mem* memorySegments, UM_Word ID);
void instantiateMem(Mem* memorySegments, int length);
UM_Word mapSegment(Mem* memorySegments, int length);
void unmapSegment(Mem* memorySegments, UM_Word index);
UM_Word segmentedLoad(Mem* memorySegments, UM_Word ID, int offset);
void segmentedStore(Mem* memorySegments, UM_Word ID, int offset, UM_Word
                       value);
void freeMem(Mem* memorySegments);
#endif
