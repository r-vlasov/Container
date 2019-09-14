OBJ_DIR = ./obj
OBJ_FILES := $(wildcard $(OBJ_DIR)/*.o)
GCC = gcc
SRC = ./src
EXECUTABLE = ./test/container.exe

run:	$(OBJ_FILES)
	$(MAKE) run -C $(SRC)
	$(GCC) $^ -o $(EXECUTABLE)
