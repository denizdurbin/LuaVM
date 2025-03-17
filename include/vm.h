#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <string.h>
#include "parser.h" 

#define STACK_SIZE 1024
#define REGISTER_COUNT 256
#define GLOBAL_ENV_SIZE 256

typedef struct {
    double stack[STACK_SIZE];
    int stack_top;
    double registers[REGISTER_COUNT];
    LuaChunk *chunk;
    LuaHeader header; 
    uint32_t pc; 
    double global_env[GLOBAL_ENV_SIZE];
    char *global_names[GLOBAL_ENV_SIZE];
} VM;

void init_vm(VM *vm, LuaChunk *chunk, LuaHeader *header);
void push(VM *vm, double value);
double pop(VM *vm);
void run(VM *vm);


#endif