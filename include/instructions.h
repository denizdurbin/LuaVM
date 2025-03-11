#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>

typedef enum {
    OP_ABC,
    OP_ABx,
    OP_AsBx
} OpcodeType;


extern const char* lua_opcode_names[40];
extern const OpcodeType lua_opcode_types[40];

#endif