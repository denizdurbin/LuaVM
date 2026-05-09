#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/parser.h"
#include "../include/instructions.h"



uint8_t get_byte(FILE *file){
    uint8_t byte;
    fread(&byte, sizeof(uint8_t), 1, file);
    return byte;
}

uint32_t get_bits(uint32_t num, int pos, int size) {
    return (num >> pos) & ((1 << size) - 1);
}

//String of predetermined size
void get_string(FILE *file, char* buff, size_t size){
    fread(buff, sizeof(char),size,file);
    buff[size] = '\0';
}

//Dynamic string
void read_string(FILE *file, char** buff, bool big_endian) {
    uint32_t length = get_uint32(file, big_endian);
    //printf("String length: %u\n", length); 

    *buff = malloc(length + 1);
    fseek(file, 4, SEEK_CUR); //4 octet de padding avant chaque nom , a voir
    if (*buff == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    fread(*buff, sizeof(char), length, file);
    (*buff)[length] = '\0';
    //printf("Read string (length %u): ", length); 
    for (size_t i = 0; i < length; i++) {
        printf("%02X ", (unsigned char)(*buff)[i]);
    }
    printf("\n");
    
    
}

void read_string_sans_padding(FILE *file, char** buff, bool big_endian) {
    uint32_t length = get_uint32(file, big_endian);
    *buff = malloc(length + 1);
    fseek(file, 4, SEEK_CUR);
    if (*buff == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    fread(*buff, sizeof(char), length, file);
    (*buff)[length] = '\0';

}

uint32_t get_uint32(FILE *file, bool big_endian){
    uint32_t val;
    fread(&val, 1, 4, file);
    if(big_endian){
        return __builtin_bswap32(val);
    }
    return val;
}



uint64_t get_uint64(FILE *file, int int_size, bool big_endian) {
    uint64_t val = 0;
    fread(&val, int_size, 1, file);
    if (big_endian) {
        switch (int_size) {
            case 1:
                return val; 
            case 2:
                return __builtin_bswap16((uint16_t)val);
            case 4:
                return __builtin_bswap32((uint32_t)val);
            case 8:
                return __builtin_bswap64(val);
            default:
                return val;
        }
    }
    
    return val;
}
size_t get_sizet(FILE *file, bool big_endian) {
    size_t val;
    fread(&val, sizeof(size_t), 1, file);

    if (big_endian) {
        if (sizeof(size_t) == 8) {
            return __builtin_bswap64(val); 
        }
        return __builtin_bswap32((uint32_t)val);
    }
    return val;
}

double get_double(FILE *file, bool big_endian) {
    uint8_t bytes[8];
    fread(bytes, sizeof(uint8_t), 8, file);

    double result;
    if (big_endian) {
        uint64_t swapped = ((uint64_t)bytes[0] << 56) | ((uint64_t)bytes[1] << 48) |
                           ((uint64_t)bytes[2] << 40) | ((uint64_t)bytes[3] << 32) |
                           ((uint64_t)bytes[4] << 24) | ((uint64_t)bytes[5] << 16) |
                           ((uint64_t)bytes[6] << 8)  | (uint64_t)bytes[7];
        memcpy(&result, &swapped, sizeof(double));
    } else {
        memcpy(&result, bytes, sizeof(double));
    }

    return result;
}

void parse_lua_header(FILE *file, LuaHeader *header){
    header->signature_byte = get_byte(file);
    get_string(file, header->signature, 3);
    header->vm_version = get_byte(file);
    header->bytecode_format = get_byte(file);
    header->big_endian = (get_byte(file) == 0);
    header->int_size = get_byte(file);
    header->sizet = get_byte(file);
    header->instr_size = get_byte(file);
    header->l_number_size = get_byte(file);
    header->integral_flag = get_byte(file);
}


void parse_lua_chunk(FILE *file, LuaChunk *chunk, bool big_endian){
    
    read_string(file, &chunk->name, big_endian);

    chunk->first_line = get_uint32(file, big_endian);
    chunk->last_line = get_uint32(file, big_endian);
/*     if(flag ==1 ){
        uint8_t debug = get_byte(file);
        printf("debug %u\n", debug);
        uint8_t debug2 = get_byte(file);
        printf("debug %u\n", debug2);
        uint8_t debug3 = get_byte(file);
        printf("debug %u\n", debug3);
        uint8_t debug4 = get_byte(file);
        printf("debug %u\n", debug4);
    } */
    chunk->upvalues = get_byte(file);
    chunk->arguments = get_byte(file);
    chunk->varg = get_byte(file);
    chunk->stack = get_byte(file);

    printf("Chunk Name: %s\n", chunk->name);
    printf("First Line: %u\n", chunk->first_line);
    printf("Last Line: %u\n", chunk->last_line);
    printf("Upvalues: %u\n", chunk->upvalues);
    printf("Arguments: %u\n", chunk->arguments);
    printf("VArg: %u\n", chunk->varg);
    printf("Stack: %u\n", chunk->stack);

    chunk->nb_instructions = get_uint32(file, big_endian);
    
    chunk->instructions = malloc(chunk->nb_instructions * sizeof(Instruction));
    if (chunk->instructions == NULL) {
        fprintf(stderr, "Memory allocation failed for instructions\n");
        exit(1);
    }
    printf("Instruction size: %u\n", chunk->nb_instructions);
    printf("*** DECODING INSTRUCTIONS *** \n");
    for(int i = 0; i < (int)chunk->nb_instructions; i++){
        uint32_t data = get_uint32(file, big_endian);
        Instruction instr =  {0};
        instr.opcode = get_bits(data, 0, 6);
        OpcodeType tp = lua_opcode_types[instr.opcode];
    
        strcpy(instr.type, (tp == OP_ABC) ? "ABC" : (tp == OP_ABx) ? "ABx" : "AsBx");

        instr.A = get_bits(data, 6, 8);
        
        if(tp == OP_ABC){
            instr.B = get_bits(data, 23, 9);
            instr.C = get_bits(data, 14, 9);
        } else if (tp == OP_ABx) {
            instr.Bx = get_bits(data, 14, 18);
        } else if (tp == OP_AsBx) {
            instr.sBx = get_bits(data, 14, 18);
            printf ("Raw sBx bits: %u\n", instr.sBx); // Debug print
            instr.sBx -= 131071;
        }
        chunk->instructions[i] = instr;
        printf("Instruction %d: %s,Opcode=%u, Type=%s, A=%u, B=%u, C=%u, Bx=%u, sBx=%d\n", i+1, lua_opcode_names[instr.opcode], instr.opcode, instr.type, instr.A, instr.B, instr.C, instr.Bx, instr.sBx);
      
    }
    printf("*** DECODING CONSTANTS ***\n");
    chunk->nb_constants = get_uint32(file, big_endian);
    printf("Constants: %u\n", chunk->nb_constants);
    chunk->constants = malloc(chunk->nb_constants * sizeof(Constant));
    if (chunk->constants == NULL) {
        fprintf(stderr, "Memory allocation failed for constants\n");
        exit(1);
    }
    for (size_t i = 0; i < chunk->nb_constants; i++) {
        Constant constant = {0};
        constant.type = get_byte(file);
        if (constant.type == LUA_TNIL) {
            constant.data.string = NULL;
        } else if (constant.type == LUA_TBOOLEAN) {
            constant.data.boolean = (get_byte(file) != 0);
        } else if (constant.type == LUA_TNUMBER) {
            constant.data.number = get_double(file, big_endian);
        } else if (constant.type == LUA_TSTRING) {
            read_string_sans_padding(file, &constant.data.string, big_endian);
            printf("string, %s\n", constant.data.string);
        }
        chunk->constants[i] = constant;
        printf("Constant %zu: Type=%u, ", i, constant.type);
        if (constant.type == LUA_TNIL) {
            printf("Nil\n");
        } else if (constant.type == LUA_TBOOLEAN) {
            printf("Boolean=%s\n", constant.data.boolean ? "true" : "false");
        } else if (constant.type == LUA_TNUMBER) {
            printf("Number=%f\n", constant.data.number);
        } else if (constant.type == LUA_TSTRING) {
            printf("String=%s\n", constant.data.string);
        }
    }


    printf("*** DECODING PROTOTYPES ***\n");
    chunk->nb_protos = get_uint32(file, big_endian);
    printf("nb_protos: %u\n", chunk->nb_protos);
    chunk->prototypes = malloc(chunk->nb_protos * sizeof(LuaChunk*));
    if (chunk->prototypes == NULL) {
        fprintf(stderr, "Memory allocation failed for prototypes\n");
        exit(1);
    }
    for (size_t i = 0; i < chunk->nb_protos; i++) {
        chunk->prototypes[i] = malloc(sizeof(LuaChunk));
        
        if (chunk->prototypes[i] == NULL) {
            fprintf(stderr, "Memory allocation failed for prototype\n");
            exit(1);
        }
  
        parse_lua_chunk(file, chunk->prototypes[i], big_endian);
        printf("Parsed prototype %zu\n", i);
    }

    printf("*** DECODING DEBUG SYMBOLS\n");
    chunk->nb_lines = get_uint32(file, big_endian);
    printf("nb_lines: %u\n", chunk->nb_lines);
    chunk->lines = malloc(chunk->nb_lines * sizeof(uint32_t*));
    if(chunk->lines == NULL){
        fprintf(stderr, "Memory allocation failed for lines\n");
        exit(1);
    }
    for(size_t i = 0; i < chunk->nb_lines; i++){
        chunk->lines[i] = get_uint32(file, big_endian);
        printf("Line %zu: %u\n", i, chunk->lines[i]);
    }

    printf("*** DECODING LOCALS ***\n");
    chunk->nb_locals = get_uint32(file, big_endian);
    chunk->locals = malloc(chunk->nb_locals * sizeof(LocalVariable));
    if (chunk->locals == NULL) {
        fprintf(stderr, "Memory allocation failed for locals\n");
        exit(1);
    }
    for(size_t i = 0; i < chunk->nb_locals; i++){
        LocalVariable local = {0};
        read_string(file, &local.name, big_endian);
        local.start_line = get_uint32(file, big_endian);
        local.end_line = get_uint32(file, big_endian);
        chunk->locals[i] = local;
        printf("Local %zu: Name=%s, Start Line=%u, End Line=%u\n", i+1, local.name, local.start_line, local.end_line);
    }

    printf("*** DECODING UPVALUES ***\n");
    chunk->nb_upvalues = get_uint32(file, big_endian);
    chunk->upvalues_list = malloc(chunk->nb_locals * sizeof(Upvalue));
    if (chunk->upvalues_list == NULL) {
        fprintf(stderr, "Memory allocation failed for upvalues\n");
        exit(1);
    }
    for(size_t i = 0; i < chunk->nb_upvalues; i++){
        Upvalue up = {0};
        read_string(file, &up.name, big_endian);
        chunk->upvalues_list[i] = up;
        printf("Upvalue %zu: Name=%s\n", i+1, up.name);
    }

    printf("*** END OF CHUNK ***\n\n");
}

void free_lua_chunk(LuaChunk *chunk) {
    if (chunk == NULL) {
        return;
    }

    if (chunk->name) {
        free(chunk->name);
    }

    if (chunk->instructions) {
        free(chunk->instructions);
    }

    if (chunk->constants) {
        for (uint32_t i = 0; i < chunk->nb_constants; i++) {
            if (chunk->constants[i].type == LUA_TSTRING && chunk->constants[i].data.string) {
                free(chunk->constants[i].data.string);
            }
        }
        free(chunk->constants);
    }

    if (chunk->prototypes) {
        for (uint32_t i = 0; i < chunk->nb_protos; i++) {
            free_lua_chunk(chunk->prototypes[i]);
        }
        free(chunk->prototypes);
    }

    if (chunk->lines) {
        free(chunk->lines);
    }

    if (chunk->locals) {
        for (uint32_t i = 0; i < chunk->nb_locals; i++) {
            if (chunk->locals[i].name) {
                free(chunk->locals[i].name);
            }
        }
        free(chunk->locals);
    }

    if (chunk->upvalues_list) {
        for (uint32_t i = 0; i < chunk->nb_upvalues; i++) {
            if (chunk->upvalues_list[i].name) {
                free(chunk->upvalues_list[i].name);
            }
        }
        free(chunk->upvalues_list);
    }
}

void free_lua_table(LuaTable *table) {
    if (table == NULL) {
        return; 
    }
    if (table->pairs) {
        for (size_t i = 0; i < table->size; i++) {
            if (table->pairs[i].key.type == LUA_TSTRING && table->pairs[i].key.data.string) {
                free(table->pairs[i].key.data.string);
            }
             if (table->pairs[i].value.type == LUA_TSTRING && table->pairs[i].value.data.string) {
                free(table->pairs[i].value.data.string);
            }
        }
        free(table->pairs);
    }

    free(table); 
}