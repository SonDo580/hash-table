CC = gcc
CFLAGS = -g -W -Wall
SRC = build/main.c build/hash_table.c
OBJ = build/main.o build/hash_table.o
TARGET = build/main

all: build_dir $(TARGET)

build_dir: 
	mkdir -p build

$(TARGET):$(OBJ)
	$(CC) $(OBJ) -o $(TARGET)

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean: 
	rm -rf build