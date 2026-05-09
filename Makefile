CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
LDLIBS = -lm
SRC = src/main.c src/parser.c src/vm.c src/instructions.c
OBJ = $(SRC:.c=.o)
OUT = lua_vm

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJ) $(LDLIBS)

debug: CFLAGS += -g
debug: clean $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(OUT)