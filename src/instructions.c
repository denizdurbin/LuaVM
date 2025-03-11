#include "instructions.h"


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