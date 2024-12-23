#ifndef TASK_H
#define TASK_H
#include "main.h"

enum mcpInputRegister_e
{
    mpcI_0_0 = 0x1,
    mpcI_0_1 = 0x2,
    mpcI_0_2 = 0x4,
    mpcI_0_3 = 0x8,
    mpcI_0_4 = 0x10,
    mpcI_0_5 = 0x20,
    mpcI_0_6 = 0x40,
    mpcI_0_7 = 0x80,
    mpcI_1_0 = 0x100,
    mpcI_1_1 = 0x200,
    mpcI_1_2 = 0x400,
    mpcI_1_3 = 0x800,
    mpcI_1_4 = 0x1000,
    mpcI_1_5 = 0x2000,
    mpcI_1_6 = 0x4000,
    mpcI_1_7 = 0x8000,
    mpcI_2_0 = 0x10000,
    mpcI_2_1 = 0x20000,
    mpcI_2_2 = 0x40000,
    mpcI_2_3 = 0x80000,
    mpcI_2_4 = 0x100000,
    mpcI_2_5 = 0x200000,
    mpcI_2_6 = 0x400000,
    mpcI_2_7 = 0x800000,
    mpcI_3_0 = 0x1000000,
    mpcI_3_1 = 0x2000000,
    mpcI_3_2 = 0x4000000,
    mpcI_3_3 = 0x8000000,
    mpcI_3_4 = 0x10000000,
    mpcI_3_5 = 0x20000000,
    mpcI_3_6 = 0x40000000,
    mpcI_3_7 = 0x80000000,
};
enum mcpOutputRegister_e
{
    mpcO_0_0 = 0x1,
    mpcO_0_1 = 0x2,
    mpcO_0_2 = 0x4,
    mpcO_0_3 = 0x8,
    mpcO_0_4 = 0x10,
    mpcO_0_5 = 0x20,
    mpcO_0_6 = 0x40,
    mpcO_0_7 = 0x80,
    mpcO_1_0 = 0x100,
    mpcO_1_1 = 0x200,
    mpcO_1_2 = 0x400,
    mpcO_1_3 = 0x800,
    mpcO_1_4 = 0x1000,
    mpcO_1_5 = 0x2000,
    mpcO_1_6 = 0x4000,
    mpcO_1_7 = 0x8000,
    mpcO_2_0 = 0x10000,
    mpcO_2_1 = 0x20000,
    mpcO_2_2 = 0x40000,
    mpcO_2_3 = 0x80000,
    mpcO_2_4 = 0x100000,
    mpcO_2_5 = 0x200000,
    mpcO_2_6 = 0x400000,
    mpcO_2_7 = 0x800000,
    mpcO_3_0 = 0x1000000,
    mpcO_3_1 = 0x2000000,
    mpcO_3_2 = 0x4000000,
    mpcO_3_3 = 0x8000000,
    mpcO_3_4 = 0x10000000,
    mpcO_3_5 = 0x20000000,
    mpcO_3_6 = 0x40000000,
    mpcO_3_7 = 0x80000000,
};

void FloorMechanism(void *pvParam);
void Dialla(void *pvParam);
void FalseSun(void *pvParam);

#endif