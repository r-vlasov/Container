OBJFILES = container.o namespace.o cgroup.o flags.o

GCC = gcc
OBJPATH = ../obj/$@


%.o: %.c
	$(GCC) -c $< -O -o  $(OBJPATH)


run: $(OBJFILES)


