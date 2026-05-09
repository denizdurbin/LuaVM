# Lua 5.1 Virtual Machine in C

A from-scratch implementation of a Lua 5.1 bytecode interpreter, written in C. The project parses compiled `.luac` files produced by the standard Lua 5.1 compiler (`luac`) and executes them on a register-based virtual machine.

Developed as coursework for **Compilation Avancée** (Advanced Compilation), M2 STL Sorbonne Université, 2025.

**Author:** Deniz Ali Durbin

## Overview

Lua programs are normally executed by the reference Lua interpreter, which uses a register-based VM operating on a custom binary bytecode format. This project reimplements that VM end to end. The binary parser walks the bytecode header and chunk hierarchy (constants, debug tables, nested function prototypes), and the execution engine runs the resulting instruction stream against an in-memory register file and global environment.

The implementation targets the Lua 5.1 binary format specifically.

## Features

- Binary parser for the Lua 5.1 bytecode format: header, chunks, prototypes, constants, locals, line info, upvalue tables
- Endianness-aware integer and float reading driven by the header descriptor
- Register-based execution engine with 256 registers per VM
- Type system covering `nil`, boolean, number, string, table, native function, and closure values
- Tables implemented as resizable key/value arrays supporting heterogeneous keys
- Global environment with native function registration; `print` is provided as a built-in
- Full arithmetic and comparison instruction set, including the Lua "register or constant" (RK) operand encoding
- Numeric `for` loops via `FORPREP` and `FORLOOP`
- String operations (`CONCAT`, `LEN`)
- 35 of the 38 Lua 5.1 opcodes implemented

## Implemented Opcodes

`MOVE`, `LOADK`, `LOADBOOL`, `LOADNIL`, `GETUPVAL`, `GETGLOBAL`, `GETTABLE`, `SETGLOBAL`, `SETUPVAL`, `SETTABLE`, `NEWTABLE`, `SELF`, `ADD`, `SUB`, `MUL`, `DIV`, `MOD`, `POW`, `UNM`, `NOT`, `LEN`, `CONCAT`, `JMP`, `EQ`, `LT`, `LE`, `TEST`, `TESTSET`, `CALL`, `TAILCALL`, `RETURN`, `FORLOOP`, `FORPREP`, `SETLIST`, `CLOSURE`.

## Limitations

- TFORLOOP not implemented
- CLOSE not implemented
- VARARG not implemented
- CLOSURE upvalue capture partial
- SETLIST B=0 and C=0 cases not handled
- No garbage collector
- No metatables
- Standard library not provided (only `print`)
- Errors exit the process rather than propagate

## Architecture

```
src/main.c              entry point, file IO, VM lifecycle
src/parser.c            .luac binary format reader
src/vm.c                instruction dispatch and execution
src/instructions.c      opcode metadata and built-in functions
include/parser.h        header, chunk, constant, table, instruction types
include/vm.h            VM state (registers, globals, upvalues)
include/instructions.h  opcode names, formats, built-in registration
```

VM state holds 256 registers, 256 global slots indexed by name, 256 upvalue slots, the active chunk, and a program counter. Registers and constants share the same tagged-union `Constant` type, with type tags that mirror the Lua type identifiers.

## Build

```
make            # release build, produces ./lua_vm
make debug      # same with -g for debugging
make clean
```

Requirements: `gcc`, GNU `make`, and the standard math library.

## Usage

The VM consumes precompiled Lua 5.1 bytecode. Use the reference `luac` from a Lua 5.1 installation to produce it, then run `lua_vm` against the resulting file:

```
luac -o out.luac script.lua
./lua_vm out.luac
```

A set of Lua test programs covering arithmetic, comparison, loops, tables, `setlist`, `setglobal`, and method calls (`self`) is provided under `test/`. The matching precompiled bytecode files in the same directory can be run directly.

## Development Notes

The runtime structures and instruction semantics were written without reference to the upstream Lua VM source. The binary chunk layout was implemented from the Lua 5.1 reference manual and the published bytecode specification. Memory safety was verified during development with Valgrind.
