#include <stdint.h>
#include <stdlib.h>
#include "bitpack.h"
#include <stdio.h>
#include "seq.h"
#include "assert.h"

typedef uint32_t Um_instruction;
typedef char Um_opcode;

extern void Um_write_sequence(FILE *output, Seq_T stream);

Um_instruction three_register(Um_opcode op, int ra, int rb, int rc);
Um_instruction loadval(unsigned ra, unsigned val);

enum regs { r0 = 0, r1, r2, r3, r4, r5, r6, r7 };

extern void emit_halt_test(Seq_T stream);
extern void emit_goto_test(Seq_T stream);
extern void emit_IO_test(Seq_T stream);
extern void emit_add_test(Seq_T stream);
extern void emit_multiply_test(Seq_T stream);
extern void emit_divide_test(Seq_T stream);
extern void emit_move_test(Seq_T stream);
extern void emit_NAND_test(Seq_T stream);
extern void emit_mapUnmap_test(Seq_T stream);
extern void emit_non0Move_test(Seq_T stream);
extern void emit_segmentLoadStore_test(Seq_T stream);
extern void emit_loadprog_test(Seq_T stream);
extern void emit_nonMove_test(Seq_T stream);
extern void emit_mapLoadStore_test(Seq_T stream);
extern void emit_50mil_test(Seq_T stream);

enum Um_opcode {
    MOVE = 0,
    SEGLOAD,
    SEGSTORE,
    ADD,
    MULTIPLY,
    DIVIDE,
    NAND,
    HALT,
    MAP,
    UNMAP,
    OUTPUT,
    INPUT,
    LOADPROG,
    LOADVAL
};
