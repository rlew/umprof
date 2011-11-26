#include "umlab.h"
#include "limits.h"

void Um_write_sequence(FILE *output, Seq_T stream){
    for(int i = 0; i < Seq_length(stream); i++) {
        Um_instruction instr = (uintptr_t)Seq_get(stream, i);
        for(int i = 24; i >= 0; i -= 8){
            fputc(Bitpack_getu(instr, 8, i), output);
        }
    }
}

static inline void emit(Seq_T stream, Um_instruction inst) {
    assert(sizeof(inst) <= sizeof(uintptr_t));
    Seq_addhi(stream, (void *)(uintptr_t)inst);
}

static inline Um_instruction get_inst(Seq_T stream, int i) {
    assert(sizeof(Um_instruction) <= sizeof(uintptr_t));
    return (Um_instruction)(uintptr_t)(Seq_get(stream, i));
}

static inline void put_inst(Seq_T stream, int i, Um_instruction inst) {
    assert(sizeof(inst) <= sizeof(uintptr_t));
    Seq_put(stream, i, (void *)(uintptr_t) inst);
}

static void add_label(Seq_T stream, int location_to_patch, int label_value) {
    Um_instruction inst = get_inst(stream, location_to_patch);
    unsigned k = Bitpack_getu(inst, 25, 0);
    inst       = Bitpack_newu(inst, 25, 0, label_value+k);
    put_inst(stream, location_to_patch, inst);
}

Um_instruction three_register(Um_opcode op, int ra, int rb, int rc){
    Um_instruction instr = 0;
    Um_instruction mask = instr | (Um_instruction)op;
    instr = Bitpack_newu(instr, 4, 28, mask);
    instr = Bitpack_newu(instr, 3, 6, ra);
    instr = Bitpack_newu(instr, 3, 3, rb);
    instr = Bitpack_newu(instr, 3, 0, rc);
    return instr;
}

Um_instruction loadval(unsigned ra, unsigned val){
    Um_opcode op = 13;
    Um_instruction instr = 0;
    instr = Bitpack_newu(instr, 4, 28, op);
    instr = Bitpack_newu(instr, 3, 25, ra);
    instr = Bitpack_newu(instr, 25, 0, val);
    return instr;
}

static inline Um_instruction halt(void) {
    return three_register(HALT, 0, 0, 0);
}

static inline Um_instruction conditionalMove(int ra, int rb, int rc){
    return three_register(MOVE,ra, rb, rc);
}

static inline Um_instruction addition(int ra, int rb, int rc){
    return three_register(ADD, ra, rb, rc);
}
static inline Um_instruction multiplication(int ra, int rb, int rc){
    return three_register(MULTIPLY, ra, rb, rc);
}
static inline Um_instruction output(int rc){
    return three_register(OUTPUT, 0, 0, rc);
}
static inline Um_instruction division(int ra, int rb, int rc){
    return three_register(DIVIDE, ra, rb, rc);
}
static inline Um_instruction bitwiseNAND(int ra, int rb, int rc){
    return three_register(NAND, ra, rb, rc);
}
static inline Um_instruction segmentedLoad(int ra, int rb, int rc){
    return three_register(SEGLOAD, ra, rb, rc);
}
static inline Um_instruction segmentedStore(int ra, int rb, int rc){
    return three_register(SEGSTORE, ra, rb, rc);
}
static inline Um_instruction map(int rb, int rc){
    return three_register(MAP, 0, rb, rc);
}
static inline Um_instruction unmap(int rc){
    return three_register(UNMAP, 0, 0, rc);
}
static inline Um_instruction input(int rc){
    return three_register(INPUT, 0, 0, rc);
}
static inline Um_instruction loadprogram(int rb, int rc){
    return three_register(LOADPROG, 0, rb, rc);
}

static void emit_out_string(Seq_T stream, const char *s, int aux_reg){
    for(int i = 0; s[i] != '\0'; i++) {
        emit(stream, loadval(aux_reg, s[i]));
        emit(stream, output(aux_reg));
    }
}

/* Halt testing */
void emit_halt_test(Seq_T stream) {
    (void)stream;
    emit(stream, halt());
    emit(stream, loadval(r1, 'B'));
    emit(stream, output(r1));
    emit(stream, loadval(r1, 'a'));
    emit(stream, output(r1));
    emit(stream, loadval(r1, 'd'));
    emit(stream, output(r1));
    emit(stream, loadval(r1, '!'));
    emit(stream, output(r1));
    emit(stream, loadval(r1, '\n'));
    emit(stream, output(r1));
}

void emit_IO_test(Seq_T stream) {
    emit(stream, loadval(r2, 1));
    emit(stream, loadval(r3, 12));
    emit(stream, map(r2, r3));
    for(int i = 0; i < 12; i++) {
        emit(stream, input(r1));
        emit(stream, loadval(r3, i));
        emit(stream, segmentedStore(r2, r3, r1));
    }

    for(int i = 0; i < 12; i++) {
        emit(stream, loadval(r3, i));
        emit(stream, segmentedLoad(r1, r2, r3));
        emit(stream, output(r1));
    }
}

void emit_add_test(Seq_T stream) {
    emit(stream, loadval(r0, 10));
    emit(stream, loadval(r1, 70));
    emit(stream, addition(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 6));
    emit(stream, loadval(r1, 59));
    emit(stream, addition(r2, r1, r0));
    emit(stream, output(r2));    
    emit(stream, loadval(r0, 33));
    emit(stream, loadval(r1, 50));
    emit(stream, addition(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 33));
    emit(stream, loadval(r1, 50));
    emit(stream, addition(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 3));
    emit(stream, loadval(r1, 66));
    emit(stream, addition(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 65));
    emit(stream, loadval(r1, 3));
    emit(stream, addition(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 30));
    emit(stream, loadval(r1, 16));
    emit(stream, addition(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 0));
    emit(stream, loadval(r1, 0));
    emit(stream, addition(r2, r1, r0));
    emit(stream, output(r2));

}

void emit_multiply_test(Seq_T stream) {
    emit(stream, loadval(r0, 2));
    emit(stream, loadval(r1, 40));
    emit(stream, multiplication(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 5));
    emit(stream, loadval(r1, 13));
    emit(stream, multiplication(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 83));
    emit(stream, loadval(r1, 1));
    emit(stream, multiplication(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 1));
    emit(stream, loadval(r1, 83));
    emit(stream, multiplication(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 3));
    emit(stream, loadval(r1, 23));
    emit(stream, multiplication(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 17));
    emit(stream, loadval(r1, 4));
    emit(stream, multiplication(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 2));
    emit(stream, loadval(r1, 23));
    emit(stream, multiplication(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 0));
    emit(stream, loadval(r1, 23));
    emit(stream, multiplication(r2, r1, r0));
    emit(stream, output(r2));
}

void emit_divide_test(Seq_T stream){
    emit(stream, loadval(r0, 2));
    emit(stream, loadval(r1, 160));
    emit(stream, division(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 2));
    emit(stream, loadval(r1, 130));
    emit(stream, division(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 3));
    emit(stream, loadval(r1, 249));
    emit(stream, division(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 4));
    emit(stream, loadval(r1, 332));
    emit(stream, division(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 1));
    emit(stream, loadval(r1, 69));
    emit(stream, division(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 5));
    emit(stream, loadval(r1, 340));
    emit(stream, division(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 1));
    emit(stream, loadval(r1, 46));
    emit(stream, division(r2, r1, r0));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 10));
    emit(stream, loadval(r1, 0));
    emit(stream, division(r2, r1, r0));
    emit(stream, output(r2));
}

void emit_nonMove_test(Seq_T stream){
    emit(stream, loadval(r0, 1));
    emit(stream, loadval(r1, 104));
    emit(stream, loadval(r2, 104));
    emit(stream, conditionalMove(r0, r1, r2));
    emit(stream, output(r0));
    emit(stream, output(r1));
    
}


void emit_move_test(Seq_T stream){
    int L1 = Seq_length(stream);
    emit(stream, loadval(r7, 0));
    int L2 = Seq_length(stream);
    emit(stream, loadval(r6, 0));
    
    emit(stream, conditionalMove(r7, r6, r0));
    emit(stream, loadprogram(r0, r7));
    add_label(stream, L2, Seq_length(stream));
    emit_out_string(stream, "Conditional Move on zero register failed.\n",
                    r5);
    emit(stream, halt());
    add_label(stream, L1, Seq_length(stream));
    emit_out_string(stream, "Conditional Move on zero register passed.\n",
                    r5);
    emit(stream, halt());
}

void emit_NAND_test(Seq_T stream){
    emit(stream, loadval(r0, 175));
    emit(stream, loadval(r1, 175));
    emit(stream, bitwiseNAND(r2, r0, r1));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 255));
    emit(stream, loadval(r1, 190));
    emit(stream, bitwiseNAND(r2, r0, r1));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 172));
    emit(stream, loadval(r1, 255));
    emit(stream, bitwiseNAND(r2, r0, r1));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 188));
    emit(stream, loadval(r1, 237));
    emit(stream, bitwiseNAND(r2, r0, r1));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 250));
    emit(stream, loadval(r1, 186));
    emit(stream, bitwiseNAND(r2, r0, r1));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 191));
    emit(stream, loadval(r1, 251));
    emit(stream, bitwiseNAND(r2, r0, r1));
    emit(stream, output(r2));
    emit(stream, loadval(r0, 223));
    emit(stream, loadval(r1, 209));
    emit(stream, bitwiseNAND(r2, r0, r1));
    emit(stream, output(r2));
}


void emit_mapUnmap_test(Seq_T stream){
    int NUM_MAP = 6000;
    emit(stream, loadval(r3, 1));
    emit(stream, loadval(r1, 97));
    /* Map NUM_MAP - 2 */
    for (int i = 1; i < NUM_MAP; i++) {
        emit(stream, loadval(r0, i));
        emit(stream, map(r0, r3));
        for(int j = 0; j < 1; j++) {
            emit(stream, loadval(r4, j));
            emit(stream, segmentedStore(r0, r4, r1));
            emit(stream, segmentedLoad(r2, r0, r4));
        }
    }
    /* Unmap all ID's mapped */
    for(int i = 1; i < NUM_MAP; i++) {
        emit(stream, loadval(r0, i));
        emit(stream, unmap(r0));
    }
    /* Map half the ID's */
    for (int i = 1; i < NUM_MAP/2; i++) {
        emit(stream, loadval(r0, i));
        emit(stream, map(r0, r3));
        for(int j = 0; j < 1; j++) {
            emit(stream, loadval(r4, j));
            emit(stream, segmentedStore(r0, r4, r1));
            emit(stream, segmentedLoad(r2, r0, r4));
        }
    }

    /* Unmap a random ID */
    emit(stream, loadval(r0, NUM_MAP-5));
    emit(stream, unmap(r0));
    /* Map the random ID */
    emit(stream, map(r0, r0));
    /* Unmap the rest */
    for(int i = NUM_MAP-1; i > NUM_MAP/2; i--) {
        emit(stream, loadval(r0, i));
        emit(stream, unmap(r0));
    }

    emit_out_string(stream, "Map / unmap passed.\n", r7);
}

void emit_loadprog_test(Seq_T stream){
    emit(stream, loadval(r2, 7));
    emit(stream, loadprogram(r0, r2));

    emit(stream, output(r0));
    emit(stream, loadval(r2, 2));
    emit(stream, loadval(r3, 3));
    emit(stream, loadval(r4, 4));
    emit(stream, loadval(r5, 5));
    emit(stream, loadval(r1, 1));
    emit(stream, loadval(r6, 6));
    emit(stream, loadval(r7, 104));
    emit(stream, output(r7));

    /* Creating a 32 bit word to load into the program */
    emit(stream, loadval(r0, 1));
    /* mapping new segment for instr */
    emit(stream, map(r1, r0));  
    emit(stream, loadval(r2, 0));
    Um_instruction out = output(r4); 
    /* bit shifting, multiplication, and addition to store instr code */
    uint32_t lhalf = out >> 16;
    uint32_t rhalf = out << 16;
    rhalf = rhalf >> 16;
    uint32_t multiply = 1 << 16;
    emit(stream, loadval(r4, lhalf));
    emit(stream, loadval(r5, rhalf));
    emit(stream, loadval(r6, multiply)); 
    emit(stream, multiplication(r7, r4, r6));
    emit(stream, addition(r3, r7, r5));
    emit(stream, segmentedStore(r1, r2, r3));
    /* loading the instruction that was loaded into $m[1][0] */
    emit(stream, loadval(r4, 105));
    emit(stream, loadprogram(r1, r2));
    emit(stream, halt());
}


void emit_goto_test(Seq_T stream) {
    int patch_L = Seq_length(stream);
    emit(stream, loadval(r7, 0));  // will be patched to 'r7 := L'
    emit(stream, loadprogram(r0, r7)); // should goto label L
    emit_out_string(stream, "GOTO failed.\n", r1);
    emit(stream, halt());
    add_label(stream, patch_L, Seq_length(stream)); // define 'L' to be here
    emit_out_string(stream, "GOTO passed.\n", r1);
    emit(stream, halt());
} 
