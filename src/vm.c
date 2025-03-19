#include "../include/vm.h"
#include "../include/instructions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
    
    vm->global_env[0].type = 2;
    vm->global_env[0].data.number = (double)(uintptr_t)&printf;
    vm->global_names[0] = "print";

}

void execute_instruction(VM *vm, Instruction *instr) {
    switch (instr->opcode) {
        case 0: // MOVE
            {
                uint32_t a = instr->A;
                uint32_t b = instr->B;
                vm->registers[a] = vm->registers[b];
            }
            break;
        case 1: // LOADK
            {
                uint32_t a = instr->A;
                uint32_t Bx = instr->Bx;
                switch(vm->chunk->constants[Bx].type){
                    case 0 : // NIL
                        { 
                            vm->registers[a].type = 0;
                            vm->registers[a].data.string = NULL;
                        }
                        break;
                    case 1 : // BOOL
                        { 
                            vm->registers[a].type = 1;
                            vm->registers[a].data.boolean = vm->chunk->constants[Bx].data.boolean;
                        }
                        break;
                    case 3 : // NUM
                        { 
                            vm->registers[a].type = 2;
                            vm->registers[a].data.number = vm->chunk->constants[Bx].data.number;
                            
                        }
                        break;
                    case 4 : // STRING
                        { 
                            vm->registers[a].type = 3;
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
        case 5 : // GETGLOBAL
            {
                uint32_t a = instr->A;
                uint32_t Bx = instr->Bx;
                const char *name = vm->chunk->constants[Bx].data.string;
                for (int i = 0; i < GLOBAL_ENV_SIZE; i++) {
                    if (vm->global_names[i] && strcmp(vm->global_names[i], name) == 0) {
                        vm->registers[a].data.string = vm->global_env[i].data.string;
                        vm->registers[a].type = 3;
                        break;
                    }
                }
            }
            break;  
        case 12: // ADD
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    uint32_t c = instr->C;
                    vm->registers[a].data.number = vm->registers[b].data.number + vm->registers[c].data.number;
                }
                break;
        case 13: // SUB
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    uint32_t c = instr->C;
                    vm->registers[a].data.number = vm->registers[b].data.number - vm->registers[c].data.number;
                }
                break;
        case 14: // MUL
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    uint32_t c = instr->C;
                    vm->registers[a].data.number = vm->registers[b].data.number * vm->registers[c].data.number;
                }
                break;
        case 15: // DIV
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    uint32_t c = instr->C;
                    if(vm->registers[c].data.number == 0){
                        if(vm->registers[b].data.number == 0){
                            vm->registers[a].data.number = NAN;
                        }
                        else if(vm->registers[b].data.number > 0){
                            vm->registers[a].data.number = INFINITY;
                        }
                        else{
                            vm->registers[a].data.number = -INFINITY;
                        }
                    }
                    vm->registers[a].data.number = vm->registers[b].data.number / vm->registers[c].data.number;
                }
                break;
        case 28: //CALL
            {
                uint32_t a = instr->A;
                uint32_t b = instr->B;
                uint32_t c = instr->C;
                
                void (*func)(const char *, ...) = (void (*)(const char *, ...))(uintptr_t)vm->registers[a].data.number;
                
                if (func != NULL) {
                    if (b > 1) {
                        func("%f\n", vm->registers[a + 1].data.number);
                    }     
                    if (c > 1) {
                        vm->registers[a].data.number = 1.0; 
                    }
                } else {
                    fprintf(stderr, "CALL: Function pointer is NULL\n");
                    exit(1);
                }
            }
            break;
        case 30: // RETURN
            {
                uint32_t a = instr->A;
                uint32_t b = instr->B;
                if (b == 1) {
                    // No return values
                } else if (b > 1) {
                    // Return (B-1) values starting from R(A)
                    for (uint32_t i = 0; i < b - 1; i++) {
                        double result = vm->registers[a + i].data.number;
                        printf("Returning %f\n", result);  
                    }
                } else if (b == 0) {
                    // Return values from R(A) to the top of the stack
                    for (uint32_t i = a; i <= vm->stack_top; i++) {
                        double result = vm->registers[i].data.number;
                        printf("Returning %f\n", result);  
                    }
                }
                // TODO; Implement returning values, close upvalues, etc.
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
