#include "../include/vm.h"
#include "../include/instructions.h"
#include <stdio.h>
#include <stdlib.h>

void init_vm(VM *vm, LuaChunk *chunk, LuaHeader *header) {
    vm->stack_top = -1;
    vm->chunk = chunk;
    vm->pc = 0;
    vm->header = *header; 
    for (int i = 0; i < REGISTER_COUNT; i++) {
        vm->registers[i] = 0.0;
    }
    for (int i = 0; i < GLOBAL_ENV_SIZE; i++) {
        vm->global_env[i] = 0.0;
        vm->global_names[i] = NULL;
    }
    vm->global_env[0] = (double)(uintptr_t)&printf;
    vm->global_names[0] = "print";
}

void push(VM *vm, double value) {
    if (vm->stack_top >= STACK_SIZE - 1) {
        fprintf(stderr, "Stack overflow\n");
        exit(1);
    }
    vm->stack[++vm->stack_top] = value;
}

double pop(VM *vm) {
    if (vm->stack_top < 0) {
        fprintf(stderr, "Stack underflow\n");
        exit(1);
    }
    return vm->stack[vm->stack_top--];
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
                vm->registers[a] = vm->chunk->constants[Bx].data.number;
            }
            break;
        case 3 : // LOADNIL
            {
                uint32_t a = instr->A;
                uint32_t b = instr->B;
                for (uint32_t i = a; i <= b; i++) {
                    vm->registers[i] = 0.0;
                }
            }
        case 5: // GETGLOBAL
        {
            uint32_t a = instr->A;
            uint32_t Bx = instr->Bx;
            const char *name = vm->chunk->constants[Bx].data.string;
            for (int i = 0; i < GLOBAL_ENV_SIZE; i++) {
                if (vm->global_names[i] && strcmp(vm->global_names[i], name) == 0) {
                    vm->registers[a] = vm->global_env[i];
                    break;
                }
            }
        }
        break;  
            break;
        case 12: // ADD
            {
                uint32_t a = instr->A;
                uint32_t b = instr->B;
                uint32_t c = instr->C;
                vm->registers[a] = vm->registers[b] + vm->registers[c];
            }
            break;
        case 13: // SUB
            {
                uint32_t a = instr->A;
                uint32_t b = instr->B;
                uint32_t c = instr->C;
                vm->registers[a] = vm->registers[b] - vm->registers[c];
            }
            break;
        case 14: // MUL
            {
                uint32_t a = instr->A;
                uint32_t b = instr->B;
                uint32_t c = instr->C;
                vm->registers[a] = vm->registers[b] * vm->registers[c];
            }
            break;
        case 15: // DIV
            {
                uint32_t a = instr->A;
                uint32_t b = instr->B;
                uint32_t c = instr->C;
                vm->registers[a] = vm->registers[b] / vm->registers[c];
            }
            break;
        case 28: //CALL
        {
            uint32_t a = instr->A;
            uint32_t b = instr->B;
            uint32_t c = instr->C;
            
            void (*func)(const char *, ...) = (void (*)(const char *, ...))(uintptr_t)vm->registers[a];
            
            if (func != NULL) {
                if (b > 1) {
                    func("%f\n", vm->registers[a + 1]);
                }     
                if (c > 1) {
                    vm->registers[a] = 1.0; 
                }
            } else {
                fprintf(stderr, "CALL: Function pointer is NULL\n");
                exit(1);
            }
        }
        break;
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
                    double result = vm->registers[a + i];
                    printf("Returning %f\n", result);  
                }
            } else if (b == 0) {
                // Return values from R(A) to the top of the stack
                for (uint32_t i = a; i <= vm->stack_top; i++) {
                    double result = vm->registers[i];
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
