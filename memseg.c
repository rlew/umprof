#include <stdlib.h>
#include "memseg.h"
#include <stdio.h>
#include <mem.h>
#include "string.h"
#include <assert.h>

struct Mem{
    UArray_T* mappedIDs;
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

/*
 * Creates a copy of the segment at the specified ID and returns the segment
 */
static UArray_T segmentCopy(Mem* memorySegments, UM_Word ID){
    UArray_T segment = memorySegments->mappedIDs[ID];
    UArray_T copy = UArray_new(UArray_length(segment), sizeof(UM_Word));

    for(int i = 0; i < UArray_length(segment); i++){
        UM_Word* value = UArray_at(copy, i);
        *value = *(UM_Word*)UArray_at(segment, i);
    }
    return copy;
}

/*
 * Removes memorySegment 0 and copies the segment at ID to 0
 */
void loadSegment(Mem* memorySegments, UM_Word ID){
    UArray_T copy = segmentCopy(memorySegments, ID);
    unmapSegment(memorySegments, 0);
    mapSegment(memorySegments, UArray_length(copy));
    UArray_T toFree = memorySegments->mappedIDs[0];
    memorySegments->mappedIDs[0] = copy;
    UArray_free(&toFree);
}

/*
 * Returns the value of the word stored at the given ID and offset.
 */
UM_Word getWord(Mem* memorySegments, UM_Word ID, UM_Word offset){
    return *(UM_Word*)UArray_at((UArray_T)memorySegments->mappedIDs[ID], 
                                offset);
}

/*
 * Returns the length of the mapepd segment stored at the ID
 */
int segmentLength(Mem* memorySegments, UM_Word ID){
    return UArray_length((UArray_T)memorySegments->mappedIDs[ID]);
}

/*
 * Increase the available set of IDs (to at least the ID passed in) in 
 * unmappedIDs and sets the corresponding IDs in mappedIDs to NULL. 
 */
static void resizeMapped(Mem* memorySegments) {
    int length = memorySegments->mappedLength;
    memorySegments->mappedLength = length * 2;

    UArray_T* temp = malloc(sizeof(UArray_T)*memorySegments->mappedLength);
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
    mem->mappedIDs = malloc(sizeof(UArray_T)*length);
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

    UArray_T segment = UArray_new(length, sizeof(UM_Word));
    
    // Initializing each UM_Word in the memory segment to 0
    for(UM_Word i = 0; i < (UM_Word)length; i++) {
        UM_Word* elem = UArray_at(segment, i);
        *elem = 0;
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
    UArray_T segmentID = memorySegments->mappedIDs[index];
    memorySegments->mappedIDs[index] = NULL;
    UArray_free(&segmentID);
    
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
  return *(UM_Word*)UArray_at((UArray_T)memorySegments->mappedIDs[ID], offset);
}

/*
 * Stores the value passed at the specified index and offset in the memory
 * segments. It is a c.r.t. for the ID to be unmapped
 */
void segmentedStore(Mem* memorySegments, UM_Word ID, int offset, UM_Word
                       value){
    UArray_T temp = memorySegments->mappedIDs[ID];
    assert(temp);
    UM_Word* word = UArray_at(temp, offset);
    assert(word);
    *word = value;
}

/*
 * Free's all of the memory of the Mem struct
 */ 
void freeMem(Mem* memorySegments) {
    for(int i = 0; i < memorySegments->mappedLength; i++) {
        UArray_T seg = memorySegments->mappedIDs[i];
        if(seg != NULL) {
            UArray_free(&seg);
        }
    }
    free(memorySegments->mappedIDs);
    free(memorySegments->unmappedIDs);
    free(memorySegments);
}
