#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
#include "parser.h"

typedef enum {
    OP_ABC,
    OP_ABx,
    OP_AsBx
} OpcodeType;


extern const char* lua_opcode_names[40];
extern const OpcodeType lua_opcode_types[40];
void lua_print(VM *vm, uint32_t num_args, uint32_t num_returns, uint32_t register_index);
void register_builtin(VM *vm, const char *name, LuaCFunction fn);
#endif