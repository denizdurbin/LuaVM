#include "instructions.h"
#include "vm.h"
#include <stdlib.h>
#include <string.h>


const char* lua_opcode_names[40] = {
    /* 0 */ "MOVE",     /* 1 */ "LOADK",    /* 2 */ "LOADBOOL", /* 3 */ "LOADNIL",  /* 4 */ "GETUPVAL",
    /* 5 */ "GETGLOBAL", /* 6 */ "GETTABLE", /* 7 */ "SETGLOBAL", /* 8 */ "SETUPVAL", /* 9 */ "SETTABLE",
    /* 10 */ "NEWTABLE",  /* 11 */ "SELF",     /* 12 */ "ADD",      /* 13 */ "SUB",      /* 14 */ "MUL",
    /* 15 */ "DIV",      /* 16 */ "MOD",      /* 17 */ "POW",      /* 18 */ "UNM",      /* 19 */ "NOT",
    /* 20 */ "LEN",      /* 21 */ "CONCAT",   /* 22 */ "JMP",      /* 23 */ "EQ",       /* 24 */ "LT",
    /* 25 */ "LE",       /* 26 */ "TEST",     /* 27 */ "TESTSET",  /* 28 */ "CALL",     /* 29 */ "TAILCALL",
    /* 30 */ "RETURN",   /* 31 */ "FORLOOP",  /* 32 */ "FORPREP",  /* 33 */ "TFORLOOP", /* 34 */ "SETLIST",
    /* 35 */ "CLOSE",    /* 36 */ "CLOSURE",  /* 37 */ "VARARG",   /* 38 */ "VARARG",   /* 39 */ "INVALID"
};

const OpcodeType lua_opcode_types[40] = {
    /* 0 */ OP_ABC,  /* 1 */ OP_ABx,  /* 2 */ OP_ABC,  /* 3 */ OP_ABC,  /* 4 */ OP_ABC,
    /* 5 */ OP_ABx,  /* 6 */ OP_ABC,  /* 7 */ OP_ABx,  /* 8 */ OP_ABC,  /* 9 */ OP_ABC,
    /* 10 */ OP_ABC, /* 11 */ OP_ABC, /* 12 */ OP_ABC, /* 13 */ OP_ABC, /* 14 */ OP_ABC,
    /* 15 */ OP_ABC, /* 16 */ OP_ABC, /* 17 */ OP_ABC, /* 18 */ OP_ABC, /* 19 */ OP_ABC,
    /* 20 */ OP_ABC, /* 21 */ OP_ABC, /* 22 */ OP_AsBx,  /* 23 */ OP_ABC, /* 24 */ OP_ABC,
    /* 25 */ OP_ABC, /* 26 */ OP_ABC, /* 27 */ OP_ABC, /* 28 */ OP_ABC, /* 29 */ OP_ABC,
    /* 30 */ OP_ABC, /* 31 */ OP_AsBx, /* 32 */ OP_AsBx, /* 33 */ OP_ABC, /* 34 */ OP_ABC,
    /* 35 */ OP_ABC, /* 36 */ OP_ABx,  /* 37 */ OP_ABC, /* 38 */ OP_ABC, /* 39 */ OP_ABC
};

void lua_print(VM *vm, uint32_t num_args, uint32_t num_return, uint32_t register_index) {
    uint32_t reg_index = register_index;
    for (uint32_t i = 0; i < num_args; ++i) {
        reg_index++;
        Constant *arg = &vm->registers[reg_index];
        if (arg->type == LUA_TBOOLEAN) {
            printf("%s ", arg->data.boolean ? "true" : "false");
        } else if (arg->type == LUA_TNUMBER) {
            printf("%f ", arg->data.number);
        } else if (arg->type == LUA_TSTRING) {
            printf("%s ", arg->data.string);
        } else if (arg->type == LUA_TNIL) {
            printf("nil ");
        } else if(arg->type == LUA_TTABLE) {
            printf("table: %p ", arg->data.table);
        }
    }
    printf("\n");
    vm->return_count = num_return;
}

void register_builtin(VM *vm, const char *name, LuaCFunction fn) {
    for (int i = 0; i < GLOBAL_ENV_SIZE; i++) {
        if (vm->global_names[i] == NULL) {
            vm->global_env[i].type = LUA_TFUNCTION;
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
