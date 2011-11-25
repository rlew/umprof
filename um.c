#include "um.h"
#include "mem.h"
#include "bitpack.h"

/* Registers and memory statically defined upon creation of the machine. */
#define numRegisters 8
static UM_Word registers[numRegisters];
static Mem* memorySegments;
static int programCounter;
static int INITIAL_SET_SIZE = 5000; // Number of memory segment IDs
static int PROGRAM_HINT = 500;     // Number of program instructions

/*
 * Returns a Instruction with a opcode and filled registers.
 */
static Instruction parseInstruction(UM_Word instructionCode){
    Instruction instr = { HALT, 0, 0, 0, 0 };
    instr.op = Bitpack_getu(instructionCode, 4, 28);
    if(instr.op == LOADVAL){
        instr.reg1 = Bitpack_getu(instructionCode, 3, 25);
        instr.value = Bitpack_getu(instructionCode, 25, 0);
    }
    else{
        instr.reg1 = Bitpack_getu(instructionCode, 3, 6);
        instr.reg2 = Bitpack_getu(instructionCode, 3, 3);
        instr.reg3 = Bitpack_getu(instructionCode, 3, 0);
    }
    return instr;
}

/*
 * Creates memeory for the program to run and gets the instruction code along
 * with the registers.
 */
void build_and_execute_um(FILE* program){
    memorySegments = newMem();
    instantiateMem(memorySegments, INITIAL_SET_SIZE);
    initializeRegisters(registers, numRegisters);

    mapProgram(program);
    programCounter = 0;
    int numInstr = 0;

    while(programCounter < segmentLength(memorySegments, 0)){
        UM_Word instruction = getWord(memorySegments, 0, programCounter);
        Instruction instr = parseInstruction(instruction);
        execute_instruction(instr);
        numInstr++;
        if(instr.op == HALT) break;
    }

    freeMem(memorySegments);
}

/*
 * Initializes the program counter and returns the number of instructions.
 */
void mapProgram(FILE* program) {
    Seq_T words = Seq_new(PROGRAM_HINT);

    int c = getc(program);
    while(c != EOF) {
        UM_Word temp = Bitpack_newu(temp, 8, 24, c);
        for(int bit = 16; bit >=0; bit -=8){
            int b = getc(program);
            temp = Bitpack_newu(temp, 8, bit, b);
        }

        UM_Word* instr;
        NEW(instr);
        *instr = temp;
        Seq_addhi(words, instr);
        c = getc(program);
    }

    mapSegment(memorySegments, Seq_length(words));

    for(UM_Word locToLoad = 0; locToLoad < (UM_Word)Seq_length(words);
                    locToLoad++){
        UM_Word* value = (UM_Word*)Seq_get(words, locToLoad);
        segmentedStore(memorySegments, 0, locToLoad, *value);
        FREE(value);
    }

    Seq_free(&words);
}

/*
 * Executes the Instruction based on the opcode.
 */
void execute_instruction(Instruction instr){
    switch(instr.op) {
        case MOVE:{
            conditionalMove(registers, instr.reg1, instr.reg2, instr.reg3);
            programCounter++;
            break;
        }
        case SEGLOAD:{
            UM_Word ID = registers[instr.reg2];
            UM_Word offset = registers[instr.reg3];
            UM_Word toStore = segmentedLoad(memorySegments, ID, offset);
            registers[instr.reg1] = toStore;
            programCounter++;
            break;
        }
        case SEGSTORE:{
            UM_Word ID = registers[instr.reg1];
            UM_Word offset = registers[instr.reg2];
            UM_Word value = registers[instr.reg3];
            segmentedStore(memorySegments, ID, offset, value);
            programCounter++;
            break;
        }
        case ADD:{
            addition(registers, instr.reg1, instr.reg2, instr.reg3);
            programCounter++;
            break;
        }
        case MULTIPLY:{
            multiplication(registers, instr.reg1, instr.reg2, instr.reg3);
            programCounter++;
            break;
        }
        case DIVIDE:{
            division(registers, instr.reg1, instr.reg2, instr.reg3);
            programCounter++;
            break;
        }
        case NAND:{
            bitwiseNAND(registers, instr.reg1, instr.reg2, instr.reg3);
            programCounter++;
            break;
        }
        case HALT: {
            programCounter = 0;
            break;
        }
        case MAP:{
            UM_Word length = registers[instr.reg3];
            registers[instr.reg2] = mapSegment(memorySegments, length);
            programCounter++;
            break;
        }
        case UNMAP:{
            UM_Word ID = registers[instr.reg3];
            unmapSegment(memorySegments, ID);
            programCounter++;
            break;
        }
        case OUTPUT:{
            output(registers, instr.reg3);
            programCounter++;
            break;
        }
        case INPUT:{
            input(registers, instr.reg3);
            programCounter++;
            break;
        }
        case LOADPROG:{
            UM_Word ID = registers[instr.reg2];
            if(ID != 0)
                loadSegment(memorySegments, ID);

            programCounter = registers[instr.reg3];
            break;
        }
        case LOADVAL:{
            loadValue(registers, instr.reg1, instr.value);
            programCounter++;
            break;
        }
    }
}
