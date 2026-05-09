#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <string.h>
#include "parser.h"
#define STACK_SIZE 1024
#define REGISTER_COUNT 256
#define UPVALUES_COUNT 256
#define GLOBAL_ENV_SIZE 256
#define FPF 50



typedef struct UpvalueVM {
    char* name;
    Constant *val;
} UpvalueVM;

typedef struct LuaClosure {
    char *name;
    LuaChunk *proto;
    UpvalueVM *upvalues;
} LuaClosure;

typedef struct VM {
    Constant registers[REGISTER_COUNT];

    LuaChunk *chunk;
    LuaHeader header;
    uint32_t pc;
    Constant global_env[GLOBAL_ENV_SIZE];
    char *global_names[GLOBAL_ENV_SIZE];

    uint32_t return_count;
    UpvalueVM upvalues[UPVALUES_COUNT];
} VM;

void init_vm(VM *vm, LuaChunk *chunk, LuaHeader *header);
void run(VM *vm);
bool constant_equals(Constant a, Constant b);
void print_constant(Constant *c);

#endif