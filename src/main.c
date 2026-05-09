#include <stdio.h>
#include <stdlib.h>
#include "../include/parser.h"
#include"../include/instructions.h"
#include "../include/vm.h"

int main(int argc, char *argv[]) {
   
    if (argc < 2) {
        printf("Usage: %s <lua bytecode file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    LuaHeader* header = malloc(sizeof(LuaHeader));
    if (header == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return 1;
    }

    parse_lua_header(file, header);
    


    // printf("Signature: %02X %c%c%c\n", header.signature_byte, header.signature[0], header.signature[1], header.signature[2]);
    // printf("Lua VM version: 0x%X\n", header.vm_version);
    // printf("Format: %02X\n", header.bytecode_format);
    // printf("Big Endian: %s\n", header.big_endian ? "true" : "false");
    // printf("int_size: %d\n", header.int_size);
    // printf("size_t: %d\n", header.sizet);
    // printf("instr_size: %d\n", header.instr_size);
    // printf("l_number_size: %d\n", header.l_number_size);
    // printf("integral_flag: %d\n", header.integral_flag);

    LuaChunk *chunk = malloc(sizeof(LuaChunk));
    if (chunk == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return 1;
    }

    parse_lua_chunk(file, chunk, header->big_endian);

    fclose(file);

    VM *vm = malloc(sizeof(VM));
    if (vm == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    init_vm(vm, chunk,header);
    run(vm);

    for(int i = 0; i < REGISTER_COUNT; i++){
        if(vm->registers[i].type == LUA_TSTRING ){
            free(vm->registers[i].data.string);
        }
        else if(vm->registers[i].type == LUA_TTABLE){
            free_lua_table(vm->registers[i].data.table);
        }
    }

    for(int i = 0; i < GLOBAL_ENV_SIZE; i++){
        if(vm->global_names[i] != NULL){
            free(vm->global_names[i]);
        }
    }
    
    free_lua_chunk(chunk);
    free(chunk);
    free(header);
    return 0;
}