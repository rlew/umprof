#ifndef REGISTERS_INCLUDED
#define REGISTERS_INCLUDED
#include "memseg.h"
#include <except.h>

extern Except_T Register_Invalid;

void initializeRegisters(UM_Word* registers, int numRegisters);
void conditionalMove(UM_Word* registers, UM_Word toSet, UM_Word original,
                        UM_Word value);
void addition(UM_Word* registers, UM_Word sum, UM_Word val1, UM_Word val2);
void multiplication(UM_Word* registers, UM_Word product, UM_Word val1,
                    UM_Word val2);
void division(UM_Word* registers, UM_Word quotient, UM_Word numerator,
                UM_Word denominator);
void bitwiseNAND(UM_Word* registers, UM_Word result, UM_Word val1,
                UM_Word val2);
void loadValue(UM_Word* registers, UM_Word reg, UM_Word value);
void input(UM_Word* registers, UM_Word reg);
void output(UM_Word* registers, UM_Word reg);

#endif
