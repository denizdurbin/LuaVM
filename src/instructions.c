#include "instructions.h"
#include <stdlib.h>


const char* lua_opcode_names[40] = {
    "MOVE", "LOADK", "LOADBOOL", "LOADNIL", "GETUPVAL",
    "GETGLOBAL", "GETTABLE", "SETGLOBAL", "SETUPVAL", "SETTABLE",
    "NEWTABLE", "SELF", "ADD", "SUB", "MUL",
    "DIV", "MOD", "POW", "UNM", "NOT",
    "LEN", "CONCAT", "JMP", "EQ", "LT",
    "LE", "TEST", "TESTSET", "CALL", "TAILCALL",
    "RETURN", "FORLOOP", "FORPREP", "TFORLOOP", "SETLIST",
    "CLOSE", "CLOSURE", "VARARG"
};

const OpcodeType lua_opcode_types[40] = {
    OP_ABC, OP_ABx, OP_ABC, OP_ABC, OP_ABC,
    OP_ABx, OP_ABC, OP_ABx, OP_ABC, OP_ABC,
    OP_ABC, OP_ABC, OP_ABC, OP_ABC, OP_ABC,
    OP_ABC, OP_ABC, OP_ABC, OP_ABC, OP_ABC,
    OP_ABC, OP_ABC, OP_ABC, OP_ABC, OP_ABC,
    OP_ABC, OP_ABC, OP_ABC, OP_ABC, OP_ABC,
    OP_ABC, OP_ABC, OP_ABC, OP_ABC, OP_ABC
};

void lua_print(VM *vm) {
    Instruction *instr = &vm->chunk->instructions[vm->pc - 1];  
    uint32_t num_args = instr->B;

    for (uint32_t i = 0; i < num_args; ++i) {
        uint32_t reg_index = instr->A + i + 1;  
        Constant *arg = &vm->registers[reg_index];

        if (arg->type == 1) {  // boolean
            printf("%s ", arg->data.boolean ? "true" : "false");
        } else if (arg->type == 3) {  // number
            printf("%f ", arg->data.number);
        } else if (arg->type == 4) {  // string
            printf("%s ", arg->data.string);
        }
    }

    printf("\n");
}

void register_builtin(VM *vm, const char *name, void (*fn)()) {
    for (int i = 0; i < GLOBAL_ENV_SIZE; i++) {
        if (vm->global_names[i] == NULL) {
            vm->global_env[i].type = 2;  
            vm->global_env[i].data.function = fn;  
            vm->global_names[i] = malloc(strlen(name) + 1);
            if (vm->global_names[i] != NULL) {
                strcpy(vm->global_names[i], name);
            } else {
                fprintf(stderr, "Memory allocation failed for function name: %s\n", name);
                return;
            }
            
            return;
        }
    }
}
