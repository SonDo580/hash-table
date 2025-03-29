CC = gcc
CFLAGS = -g -W -Wall
SRC = src/main.c src/hash_table.c src/prime.c
OBJ = build/main.o build/hash_table.o build/prime.o
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