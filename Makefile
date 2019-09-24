OBJ_DIR = ./obj
OBJ_FILES := $(wildcard $(OBJ_DIR)/*.o)
GCC = gcc
SRC = ./src
EXECUTABLE = ./bin/container.exe

run:	$(OBJ_FILES)
	$(MAKE) run -C $(SRC)
	$(GCC) $^ -O -o $(EXECUTABLE)
