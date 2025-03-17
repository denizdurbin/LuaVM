#ifndef PARSER_H
#define PARSER_H    

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct{
    uint8_t opcode;
    char type[4];
    uint32_t A;
    uint32_t B;
    uint32_t C;
    uint32_t Bx;
    int32_t sBx;
} Instruction;

typedef struct{
    uint8_t type;
    union{
        bool boolean;
        double number;
        char *string;
    } data;
} Constant;

typedef struct{
    char *name;
    uint32_t start_line;
    uint32_t end_line;
} LocalVariable;

typedef struct{
    char *name;
} Upvalue;


typedef struct{
    uint8_t signature_byte;
    char signature[3];
    uint8_t vm_version;
    uint8_t bytecode_format;
    bool big_endian;
    uint8_t int_size;
    uint8_t sizet;
    uint8_t instr_size;
    uint8_t l_number_size;
    uint8_t integral_flag;
} LuaHeader;

typedef struct LuaChunk{
    char* name;
    uint32_t first_line;
    uint32_t last_line;
    uint8_t upvalues;
    uint8_t arguments;
    uint8_t varg;
    uint8_t stack;
    
    uint32_t nb_instructions;
    Instruction *instructions;

    uint32_t nb_constants;
    Constant *constants;

    uint32_t nb_protos;
    struct LuaChunk **prototypes;

    uint32_t nb_lines;
    uint32_t *lines;

    uint32_t nb_locals;
    LocalVariable *locals;

    uint32_t nb_upvalues;
    Upvalue *upvalues_list;
} LuaChunk;

uint32_t get_uint32(FILE *file, bool big_endian);

void parse_lua_header(FILE *file, LuaHeader *header);
void parse_lua_chunk(FILE *file, LuaChunk *chunk, bool big_endian);
#endif