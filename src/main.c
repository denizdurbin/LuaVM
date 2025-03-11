#include <stdio.h>
#include <stdlib.h>
#include "../include/parser.h"


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

    LuaHeader header;
    parse_lua_header(file, &header);
    


    printf("Signature: %02X %c%c%c\n", header.signature_byte, header.signature[0], header.signature[1], header.signature[2]);
    printf("Lua VM version: 0x%X\n", header.vm_version);
    printf("Format: %02X\n", header.bytecode_format);
    printf("Big Endian: %s\n", header.big_endian ? "true" : "false");
    printf("int_size: %d\n", header.int_size);
    printf("size_t: %d\n", header.sizet);
    printf("instr_size: %d\n", header.instr_size);
    printf("l_number_size: %d\n", header.l_number_size);
    printf("integral_flag: %d\n", header.integral_flag);

    LuaChunk *chunk = malloc(sizeof(LuaChunk));
    if (chunk == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return 1;
    }

    parse_lua_chunk(file, chunk, header.big_endian,0);

    fclose(file);

    
    return 0;
}