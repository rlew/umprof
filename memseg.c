#include <stdlib.h>
#include "memseg.h"
#include <stdio.h>
#include <mem.h>
#include "string.h"
#include <assert.h>

struct Mem{
    UM_Word** mappedIDs;
    int numMapped;
    int mappedLength;
    UM_Word* unmappedIDs;
    int unmappedLength;     
    int numRemapped;
};

/*
 * Creates a new Mem structure and returns it
 */
Mem* newMem(){
    Mem* mem;
    NEW(mem);
    return mem;
}

static inline int adjustForSizeIndex(int offset) {
    return offset + 1;
}

/*
 * Creates a copy of the segment at the specified ID and returns the segment
 */
static UM_Word* segmentCopy(Mem* memorySegments, UM_Word ID){
    UM_Word* segment = memorySegments->mappedIDs[ID];
    UM_Word* copy = malloc(sizeof(UM_Word)*segment[0]);

    for(UM_Word i = 0; i < segment[0]; i++){
        copy[i] = segment[i];
    }
    return copy;
}

/*
 * Removes memorySegment 0 and copies the segment at ID to 0
 */
void loadSegment(Mem* memorySegments, UM_Word ID){
    UM_Word* copy = segmentCopy(memorySegments, ID);
    unmapSegment(memorySegments, 0);
    mapSegment(memorySegments, copy[0]);
    free(memorySegments->mappedIDs[0]);
    memorySegments->mappedIDs[0] = copy;
}

/*
 * Returns the value of the word stored at the given ID and offset.
 */
UM_Word getWord(Mem* memorySegments, UM_Word ID, UM_Word offset){
    offset = adjustForSizeIndex(offset);
    return memorySegments->mappedIDs[ID][offset];
}
/*
 * Returns the length of the mapepd segment stored at the ID
 */
int segmentLength(Mem* memorySegments, UM_Word ID){
    return memorySegments->mappedIDs[ID][0] - 1;
}

/*
 * Increase the available set of IDs (to at least the ID passed in) in 
 * unmappedIDs and sets the corresponding IDs in mappedIDs to NULL. 
 */
static void resizeMapped(Mem* memorySegments) {
    int length = memorySegments->mappedLength;
    memorySegments->mappedLength = length * 2;

    UM_Word** temp = malloc(sizeof(UM_Word*)*memorySegments->mappedLength);
    for(int i = 0; i < length; i++) {
        temp[i] = memorySegments->mappedIDs[i];
    }

    free(memorySegments->mappedIDs);
    memorySegments->mappedIDs = temp;

    for(UM_Word i = length; i < (UM_Word)memorySegments->mappedLength; i++) {
        memorySegments->mappedIDs[i] = NULL;
    }
}

static void resizeUnmapped(Mem* memorySegments) {
    int length = memorySegments->unmappedLength;
    memorySegments->unmappedLength = length * 2;

    UM_Word* temp = malloc(sizeof(UM_Word)*memorySegments->unmappedLength);
    for(int i = 0; i < length; i++) {
        temp[i] = memorySegments->unmappedIDs[i];
    }
    
    free(memorySegments->unmappedIDs);
    memorySegments->unmappedIDs = temp;

    for(int i = length; i < memorySegments->unmappedLength; i++){
        memorySegments->unmappedIDs[i] = 0;
    }
}

/*
 * Allocates memory for the mapped and unmapped IDs in memory that can be used
 * to created mapped memory segments. It is a c.r.t. to pass in a length less
 * then or equal to zero.
 */
void instantiateMem(Mem* mem, int length) {
    assert(length > 0);
    mem->mappedIDs = malloc(sizeof(UM_Word*)*length);
    mem->numMapped = 0;
    mem->mappedLength = length;

    mem->unmappedLength = length/2;
    mem->numRemapped = 0;
    mem->unmappedIDs = malloc(sizeof(UM_Word)*mem->unmappedLength);
    
    for(int i = 0; i < length; i++) {
        mem->mappedIDs[i] = NULL;
    }
    for(int i = 0; i < mem->unmappedLength; i++){
        mem->unmappedIDs[i] = 0;
    }
}

/*
 * Maps a segment in memory by marking an ID as in use and allocating a
 * segment of memory of the specified length. Returns the index of the mapped
 * segment
 */
UM_Word mapSegment(Mem* memorySegments, int length) {
    if(memorySegments->numMapped == memorySegments->mappedLength){
        resizeMapped(memorySegments);
    }

    length = adjustForSizeIndex(length);
    UM_Word* segment = malloc(sizeof(UM_Word)*length);
    
    // Storing the length of each segment as the first index
    segment[0] = length;
    // Initializing each UM_Word in the memory segment to 0
    for(int i = 1; i < length; i++) {
        segment[i] = 0;
    }

    UM_Word index;
    if(memorySegments->numRemapped == 0) {
        index = memorySegments->numMapped;
    }
    else {
        index = memorySegments->unmappedIDs[memorySegments->numRemapped-1];
        memorySegments->numRemapped--;
    }
    memorySegments->mappedIDs[index] = segment;
    memorySegments->numMapped++;

    return index;
}

/*
 * Returns an ID to the pool of available IDs and frees all associated memory
 * with the given ID
 */
void unmapSegment(Mem* memorySegments, UM_Word index) {
    UM_Word* segmentID = memorySegments->mappedIDs[index];
    memorySegments->mappedIDs[index] = NULL;
    free(segmentID);
    
    if(memorySegments->numRemapped == memorySegments->unmappedLength){
        resizeUnmapped(memorySegments);
    }

    memorySegments->unmappedIDs[memorySegments->numRemapped] = index;
    memorySegments->numRemapped++;
    memorySegments->numMapped--;
}

/*
 * Returns the memory segment stored at the specified ID and offset. It is a
 * c.r.t. for the ID to be unmapped.
 */
UM_Word segmentedLoad(Mem* memorySegments, UM_Word ID, int offset){
  assert(memorySegments->mappedIDs[ID]);
  offset = adjustForSizeIndex(offset);
  return memorySegments->mappedIDs[ID][offset];
}

/*
 * Stores the value passed at the specified index and offset in the memory
 * segments. It is a c.r.t. for the ID to be unmapped
 */
void segmentedStore(Mem* memorySegments, UM_Word ID, int offset, UM_Word
                       value){
    UM_Word* temp = memorySegments->mappedIDs[ID];
    assert(temp);
    offset = adjustForSizeIndex(offset);
    assert((UM_Word)offset < memorySegments->mappedIDs[ID][0]);
    memorySegments->mappedIDs[ID][offset] = value;
}

/*
 * Free's all of the memory of the Mem struct
 */ 
void freeMem(Mem* memorySegments) {
    for(int i = 0; i < memorySegments->mappedLength; i++) {
        UM_Word* seg = memorySegments->mappedIDs[i];
        if(seg != NULL) {
            free(seg);
        }
    }
    free(memorySegments->mappedIDs);
    free(memorySegments->unmappedIDs);
    free(memorySegments);
}
