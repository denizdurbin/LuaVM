#include "../include/vm.h"
#include "../include/instructions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// OPCODES NE PAS IMPLEMENTE; SETTABLE, GETTABLE
//print stuff that doesnt exist
void init_vm(VM *vm, LuaChunk *chunk, LuaHeader *header) {

    vm->stack_top = -1;
    vm->chunk = chunk;
    vm->pc = 0;
    vm->header = *header; 
    for (int i = 0; i < REGISTER_COUNT; i++) {
        vm->registers[i].type = 0;
        vm->registers[i].data.string = NULL;
    }
    for (int i = 0; i < GLOBAL_ENV_SIZE; i++) {
        vm->global_env[i].type = 0;
        vm->global_env[i].data.string = NULL;
        vm->global_names[i] = NULL;
    }
    register_builtin(vm, "print", lua_print);
}

void execute_instruction(VM *vm, Instruction *instr) {
    switch (instr->opcode) {
        case 0: // MOVE
            {
                uint32_t a = instr->A;
                uint32_t b = instr->B;
                vm->registers[a] = vm->registers[b];
                printf("MOVE: Register[%d] = %f, Type=%d\n", a, vm->registers[a].data.number, vm->registers[a].type);
            }
            break;
        case 1: // LOADK
            {
                uint32_t a = instr->A;
                uint32_t Bx = instr->Bx;
                switch(vm->chunk->constants[Bx].type){
                    case 3 : // NUM
                        { 
                            vm->registers[a].type = 3;
                            vm->registers[a].data.number = vm->chunk->constants[Bx].data.number;
                            
                        }
                        break;
                    case 4 : // STRING
                        { 
                            vm->registers[a].type = 4;
                            vm->registers[a].data.string = malloc(strlen(vm->chunk->constants[Bx].data.string) + 1);
                            char* i = strcpy(vm->registers[a].data.string, vm->chunk->constants[Bx].data.string);
                            if(i== NULL){
                                fprintf(stderr, "Error copying string\n");
                                exit(1);
                            }
                        }
                        break;
                    default :
                        fprintf(stderr, "Unknown constant");
                        exit(1); 
                }
            }
            break;
        case 2 : // LOADBOOL
            {
                uint32_t a = instr->A;
                uint32_t b = instr->B;
                uint32_t c = instr->C;
                vm->registers[a].type = 1;
                vm->registers[a].data.boolean = b;
                if (c) {
                    vm->pc++;
                }
            }
            break;
        case 3 : // LOADNIL
            {
                uint32_t a = instr->A;
                uint32_t b = instr->B;
                for (uint32_t i = a; i <= b; i++) {
                    vm->registers[i].data.string = NULL;
                    vm->registers[i].type = 0;
                }
            }
            break;
        case 4 : // GETUPVAL
            {

            }
            break;

        case 5 : // GETGLOBAL
                {
                    uint32_t a = instr->A;
                    uint32_t Bx = instr->Bx;
                    const char *name = vm->chunk->constants[Bx].data.string;    
                    for (int i = 0; i < GLOBAL_ENV_SIZE; i++) {
                        if (vm->global_names[i] && strcmp(vm->global_names[i], name) == 0) {
                            vm->registers[a].data = vm->global_env[i].data;
                            vm->registers[a].type = vm->global_env[i].type;
                            //printf("Got global %s of type %d\n", name, vm->registers[a].type);
                            break;
                        }
                    }
                }
                break;  
        
        case 7 : // SETGLOBAL
                {
                uint32_t a = instr->A;
                uint32_t Bx = instr->Bx;
                const char *name = vm->chunk->constants[Bx].data.string;
                for (int i = 0; i < GLOBAL_ENV_SIZE; i++) {
                    if (vm->global_names[i] == NULL) {
                        vm->global_env[i] = vm->registers[a];
                        vm->global_names[i] = malloc(strlen(name) + 1);
                        //printf("Setting global %s\n", name);
                        if (vm->global_names[i] != NULL) {
                            strcpy(vm->global_names[i], name);
                        } else {
                            fprintf(stderr, "Memory allocation failed for global name: %s\n", name);
                            exit(1);
                        }
                        break;
                    }
                }
                }
                break;
        case 8 : // SETUPVAL
                {
                }
                break;  
        case 12: // ADD
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    uint32_t c = instr->C; 
                    double op1 = (b >= 256) ? vm->chunk->constants[b - 256].data.number : vm->registers[b].data.number;
                    double op2 = (c >= 256) ? vm->chunk->constants[c - 256].data.number : vm->registers[c].data.number;
                    vm->registers[a].data.number = op1 + op2;
                    vm->registers[a].type = 3;
                }
                break;
        case 13: // SUB
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    uint32_t c = instr->C;
                    double op1 = (b >= 256) ? vm->chunk->constants[b - 256].data.number : vm->registers[b].data.number;
                    double op2 = (c >= 256) ? vm->chunk->constants[c - 256].data.number : vm->registers[c].data.number;
                    vm->registers[a].data.number = op1 - op2;
                    vm->registers[a].type = 3;
                   
                }
                break;
        case 14: // MUL
        {
            uint32_t a = instr->A;
            uint32_t b = instr->B;
            uint32_t c = instr->C;
            double op1 = (b >= 256) ? vm->chunk->constants[b - 256].data.number : vm->registers[b].data.number;
            double op2 = (c >= 256) ? vm->chunk->constants[c - 256].data.number : vm->registers[c].data.number;
            vm->registers[a].data.number = op1 * op2;
            vm->registers[a].type = 3;
           
        }
                break;
        case 15: // DIV
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    uint32_t c = instr->C;

                    double op1 = (b >= 256) ? vm->chunk->constants[b - 256].data.number : vm->registers[b].data.number;
                    double op2 = (c >= 256) ? vm->chunk->constants[c - 256].data.number : vm->registers[c].data.number;
                    if(op2 == 0){
                        if(op1 == 0){
                            vm->registers[a].data.number = NAN;
                        }
                        else if(op1 > 0){
                            vm->registers[a].data.number = INFINITY;
                        }
                        else{
                            vm->registers[a].data.number = -INFINITY;
                        }
                    }
                    else{
                        vm->registers[a].data.number = op1 / op2;
                    }
                    vm->registers[a].type = 3;

                }
                break;
        case 16: // MOD
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    uint32_t c = instr->C;

                    double op1 = (b >= 256) ? vm->chunk->constants[b - 256].data.number : vm->registers[b].data.number;
                    double op2 = (c >= 256) ? vm->chunk->constants[c - 256].data.number : vm->registers[c].data.number;
                    vm->registers[a].data.number = fmod(op1, op2);
                    vm->registers[a].type = 3;
                }
                break;
        case 17: // POW
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    uint32_t c = instr->C;
                    double op1 = (b >= 256) ? vm->chunk->constants[b - 256].data.number : vm->registers[b].data.number;
                    double op2 = (c >= 256) ? vm->chunk->constants[c - 256].data.number : vm->registers[c].data.number;
                    vm->registers[a].data.number = pow(op1, op2);
                    vm->registers[a].type = 3;
                }
                break;
        case 18: // UNM
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    double op1 = (b >= 256) ? vm->chunk->constants[b - 256].data.number : vm->registers[b].data.number;
                    vm->registers[a].data.number = -op1;
                    vm->registers[a].type = 3;
                }
                break;
        case 19: // NOT
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    vm->registers[a].data.boolean = !vm->registers[b].data.boolean;
                    vm->registers[a].type = 1;
                }
                break;
        case 20: // LEN
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    if (vm->registers[b].type == 4) {
                        vm->registers[a].data.number = strlen(vm->registers[b].data.string);
                        vm->registers[a].type = 3;
                    } else {
                        fprintf(stderr, "LEN: Invalid type for register %d\n", b);
                        exit(1);
                    }
                }
                break;
        case 21 : //CONCAT  
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    uint32_t c = instr->C;
                    if (b == c) {
                        fprintf(stderr, "CONCAT: B and C cannot be the same register\n");
                        exit(1);
                    } 
                    size_t total_len = 1;
                    for (uint32_t i = b; i <= c; i++) {
                        if (vm->registers[i].type != 4) {
                            fprintf(stderr, "CONCAT: Register %d is not a string\n", i);
                            exit(1);
                        }
                        total_len += strlen(vm->registers[i].data.string);
                    }

                    char *concat_str = malloc(total_len);
                    if (!concat_str) {
                        fprintf(stderr, "Memory allocation failed for CONCAT\n");
                        exit(1);
                    }
                    concat_str[0] = '\0'; 
                    for (uint32_t i = b; i <= c; i++) {
                        strcat(concat_str, vm->registers[i].data.string);
                    }
                    vm->registers[a].data.string = concat_str;
                    vm->registers[a].type = 4;
                }
                break;
        case 22: // JMP
                {
                    uint32_t sBx = instr->sBx;
                    vm->pc += sBx;
                }
                break;
        case 28: // CALL
            {
                uint32_t a = instr->A;
                uint32_t b = instr->B;
                uint32_t c = instr->C;
                Constant funcConstant = vm->registers[a];
                if (funcConstant.type == 2 && funcConstant.data.function != NULL) {
                    vm->return_count = c;  
                    void (*func)() = funcConstant.data.function;
                    func(vm, b - 1, c - 1, a);  
                } else {
                    fprintf(stderr, "CALL: Invalid function or NULL function pointer at register %d\n", a);
                    exit(1);
                }
            }
            break;
        case 29: //TAILCALL
            {
                uint32_t a = instr->A;
                uint32_t b = instr->B;

                Constant funcConstant = vm->registers[a];
                if (funcConstant.type == 2 && funcConstant.data.function != NULL) {
                    void (*func)() = funcConstant.data.function;
                    func(vm, b - 1, 0);  
                    return;
                } else {
                    fprintf(stderr, "TAILCALL: Invalid function or NULL function pointer at register %d\n", a);
                    exit(1);
                }
            }
            break;
        case 30: // RETURN
            {
                uint32_t a = instr->A;
                uint32_t b = instr->B;
                for (uint32_t i = 0; i < b - 1; i++) {
                    vm->registers[vm->return_count + i] = vm->registers[a + i];
                }
                vm->return_count += b - 1;
            }
            break;
        case 36: // CLOSURE
            {
            }
            break;
        default:
            fprintf(stderr, "Unknown opcode: %d\n", instr->opcode);
            exit(1);
    }
}

void run(VM *vm) {
    while (vm->pc < vm->chunk->nb_instructions) {
        Instruction *instr = &vm->chunk->instructions[vm->pc++];
        execute_instruction(vm, instr);
    
    }
}
