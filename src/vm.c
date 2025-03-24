#include "../include/vm.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// OPCODES NE PAS IMPLEMENTE; TFORLOOP, CLOSURE, CLOSE, VARARG,
void init_vm(VM *vm, LuaChunk *chunk, LuaHeader *header) {

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
    
    for(int i = 0; i < UPVALUES_COUNT; i++){
        vm->upvalues[i].name = NULL;
        vm->upvalues[i].value = NULL;
    }
    register_builtin(vm, "print", lua_print);
}

Constant *get_table_value(LuaTable *table, Constant key) {
    for (size_t i = 0; i < table->size; i++) {
        if (constant_equals(table->pairs[i].key, key)) {
            return &table->pairs[i].value;
        }
    }
    return NULL; 
}

void set_table_value(LuaTable *table, Constant key, Constant value) {
    for (size_t i = 0; i < table->size; i++) {
        if (constant_equals(table->pairs[i].key, key)) {
            table->pairs[i].value = value;
            return;
        }
    }
    printf("Table Size: %zu, Capacity: %zu\n", table->size, table->capacity); // Debug print

    if (table->size == table->capacity) {
        size_t new_capacity = (table->capacity == 0) ? 4 : table->capacity * 2;
        printf("Resizing table to capacity: %zu\n", table->capacity * 2); // Debug print
        table->pairs = realloc(table->pairs, new_capacity * sizeof(KeyValuePair));
        if (!table->pairs) {
            fprintf(stderr, "Memory allocation failed during table resize\n");
            exit(1);
        }
        table->capacity = new_capacity;
    }
    table->pairs[table->size].key = key;
    table->pairs[table->size].value = value;
    table->size++;
}

bool constant_equals(Constant a, Constant b){
    if(a.type != b.type){
        return false;
    }
    switch(a.type){
        case 0: //nil
            return true;
        case 1 : //bool
            return a.data.boolean == b.data.boolean;
        case 3: //number
            return a.data.number == b.data.number;
        case 4: //string
            return strcmp(a.data.string,b.data.string) == 0;
        default:
            return false;
    }
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
                            if(vm->registers[a].data.string == NULL){
                                fprintf(stderr, "Memory allocation failed for string\n");
                                exit(1);
                            }
                            strcpy(vm->registers[a].data.string, vm->chunk->constants[Bx].data.string);
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
                uint32_t a = instr->A;
                uint32_t b = instr->B;
                vm->registers[a] = *vm->upvalues[b].value;
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
        case 6 : // GETTABLE
                {
                uint32_t a = instr->A;
                uint32_t b = instr->B;
                uint32_t c = instr->C;
                LuaTable *table = vm->registers[b].data.table;
                Constant key = (c >= 256) ? vm->chunk->constants[c - 256] : vm->registers[c];
                Constant *value = get_table_value(table, key);
                if(value){

                    vm->registers[a] = *value;

                }
                else{
                    vm->registers[a].type = 0;
                    vm->registers[a].data.string = NULL;
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
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    *vm->upvalues[b].value = vm->registers[a];
                }
                break;
        case 9: //SETTABLE
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    uint32_t c = instr->C;
                    
                    LuaTable *table = vm->registers[a].data.table;
                    Constant key = (b >= 256) ? vm->chunk->constants[b - 256] : vm->registers[b];
                    Constant value = (c >= 256) ? vm->chunk->constants[c - 256] : vm->registers[c];
                    set_table_value(table, key, value);
                }
                break;
        case 10: // NEWTABLE
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    uint32_t c = instr->C;

                    LuaTable *table = malloc(sizeof(LuaTable));
                    if (table == NULL) {
                        fprintf(stderr, "Memory allocation failed for table\n");
                        exit(1);
                    }

                    table->size = 0;
                    table->capacity = b+c;
                    table->pairs = malloc(table->capacity * sizeof(KeyValuePair));
                    if (table->pairs == NULL) {
                        fprintf(stderr, "Memory allocation failed for table pairs\n");
                        exit(1);
                    }

                    vm->registers[a].type = 5;
                    vm->registers[a].data.table = table;
                }
                break;
        case 11: // SELF
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    uint32_t c = instr->C;
                    printf("SELF: %d %d %d\n", a, b, c);
                    printf("type b: %d\n", vm->registers[b].type);
                    LuaTable *table = vm->registers[b].data.table;
                    Constant key = (c >= 256) ? vm->chunk->constants[c - 256] : vm->registers[c];
                    Constant *value = get_table_value(table, key);

                    if(value){
                        vm->registers[a] = *value;
                    }
                    else{
                        vm->registers[a].type = 0;
                        vm->registers[a].data.string = NULL;
                    }
                    vm->registers[a + 1] = vm->registers[b];
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
                    vm->registers[a].data.string = malloc(total_len);
                    if (!vm->registers[a].data.string) {
                        fprintf(stderr, "Memory allocation failed for CONCAT\n");
                        exit(1);
                    }
                    strcpy(vm->registers[a].data.string, concat_str);
                    vm->registers[a].type = 4;
                    free(concat_str);
                }
                break;
        case 22: // JMP
                {
                    int32_t sBx = instr->sBx;
                    vm->pc += sBx;
                }
                break;
        case 23: // EQ
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    uint32_t c = instr->C;
    
                    double op1 = (b >= 256) ? vm->chunk->constants[b - 256].data.number : vm->registers[b].data.number;
                    double op2 = (c >= 256) ? vm->chunk->constants[c - 256].data.number : vm->registers[c].data.number;

                    if((op1 == op2) != a){
                        vm->pc++;
                    }
                }
                break;
        case 24: // LT
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    uint32_t c = instr->C;
                
                    double op1 = (b >= 256) ? vm->chunk->constants[b - 256].data.number : vm->registers[b].data.number;
                    double op2 = (c >= 256) ? vm->chunk->constants[c - 256].data.number : vm->registers[c].data.number;
                
                    bool comparison_result = (op1 < op2);
                
                    if (comparison_result == (bool)a) { // If comparison_result is equal to A, skip the jump.
                        //Do nothing, let pc continue.
                    } else {
                        vm->pc++; // If comparison_result is not equal to A, increment pc.
                    }
                }
                break;
        case 25: // LE
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    uint32_t c = instr->C;
    
                    double op1 = (b >= 256) ? vm->chunk->constants[b - 256].data.number : vm->registers[b].data.number;
                    double op2 = (c >= 256) ? vm->chunk->constants[c - 256].data.number : vm->registers[c].data.number;

                    if((op1 <= op2) != a){
                        vm->pc++;
                    }
                }
                break;
        case 26: // TEST
                {
                    uint32_t a = instr->A;
                    uint32_t c = instr->C;
                    
                    if(vm->registers[a].type != 1){
                        vm->pc++;
                    }
                    else if(vm->registers[a].data.boolean != c){
                        vm->pc++;
                    }
                }
                break;
        case 27: // TESTSET
                {
                    uint32_t a = instr->A;
                    uint32_t b = instr->B;
                    uint32_t c = instr->C;
                    if(vm->registers[b].type != 1){
                        vm->registers[a] = vm->registers[b];
                    }
                    else if(vm->registers[b].data.boolean != c){
                        vm->pc++;
                    }
                    else{
                        vm->registers[a] = vm->registers[b];
                    }
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
                    printf("RETURNING: ");
                    print_constant(&vm->registers[a + i]);
                    printf("\n");
                    vm->registers[vm->return_count + i] = vm->registers[a + i];
                }
                vm->return_count += b - 1;
                vm->pc = vm->chunk->nb_instructions;
            }
            break;
        case 31: // FORLOOP
         {

            uint32_t a = instr->A;
            int32_t sBx = instr->sBx;
            vm->registers[a].data.number = vm->registers[a].data.number + vm->registers[a + 2].data.number;
            if(vm->registers[a + 2].data.number > 0 ? vm->registers[a].data.number <= vm->registers[a + 1].data.number : vm->registers[a].data.number >= vm->registers[a + 1].data.number){
                vm->pc += sBx;
                vm->registers[a + 3] = vm->registers[a];
            }

         }
            break;
        case 32: // FORPREP
        {
            uint32_t a = instr->A;
            uint32_t sBx = instr->sBx;

            vm->registers[a].data.number = vm->registers[a].data.number - vm->registers[a + 2].data.number;
            vm->pc += sBx;
        }
            break;
        case 33: // TFORLOOP
        {

        }
            break;
        case 34: // SETLIST
        {
            uint32_t a = instr->A;
            uint32_t b = instr->B;
            uint32_t c = instr->C;
            if(b==0){ 
                // A IMPLEMENTER
            }
            if(c==0){
                // A IMPLEMENTER
            }
            uint32_t n = (c-1 ) * FPF;
            for (uint32_t i = 1; i <= b; i++) {
                set_table_value(vm->registers[a].data.table, vm->chunk->constants[n+i-1], vm->registers[a+i]);
            }
        }
            break;
        case 36 : //CLOSURE
        {
            uint32_t a = instr->A;
            uint32_t Bx = instr->Bx;
            LuaClosure *closure = malloc(sizeof(LuaClosure));
            if (closure == NULL) {
                fprintf(stderr, "Memory allocation failed for closure\n");
                exit(1);
            }
            closure->proto = vm->chunk->prototypes[Bx];
            closure->upvalues = malloc(closure->proto->nb_upvalues * sizeof(Constant*));
            if (closure->upvalues == NULL) {
                fprintf(stderr, "Memory allocation failed for closure upvalues\n");
                exit(1);
            }
            for (uint32_t i = 0; i < closure->proto->nb_upvalues; i++) {
                closure->upvalues[i].val = &vm->registers[a + i];
            }
            vm->registers[a].type = 6;
            vm->registers[a].data.closure = closure;
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

void print_constant(Constant *c) {
    if (c->type == 1) { // boolean
        printf("%s", c->data.boolean ? "true" : "false");
    } else if (c->type == 3) { // number
        printf("%f", c->data.number);
    } else if (c->type == 4) { // string
        printf("%s", c->data.string);
    } else if (c->type == 0) { // nil
        printf("nil");
    } else if (c->type == 5) { // table
        printf("%p", c->data.table);
    } else {
        printf("unknown type");
    }
}