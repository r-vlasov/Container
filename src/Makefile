OBJFILES = container.o namespace.o cgroup.o flags.o

GCC = gcc
OBJPATH = ../obj/$@


%.o: %.c
	$(GCC) -c $< -o  $(OBJPATH)


run: $(OBJFILES)


