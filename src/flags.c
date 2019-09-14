#include "include/container.h"

#define _GNU_SOURCE
#include <sched.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>



static int set_flag(isolproc_info* info, char* arg) {
	namespace_info* bitmask = &(info->nspace);
	if (arg[0] == '-') {
		if (strlen(arg) == 2) {
			switch (arg[1]){
				case 'm': 	bitmask->mnt = 1;
					 	return 1;
				case 'p': 	bitmask->pid = 1;
						return 1;
				case 'n': 	bitmask->net = 1;
						return 1;
				case 'i': 	bitmask->ipc = 1;
						return 1;
				case 'u': 	bitmask->usr = 1;
						return 1;
				case 'U':	bitmask->uts = 1;
						return 1;
				default:
					fprintf(stderr, "Bad flag %s, stop\n", arg);
					exit(-1);
			}
		}
		return 0;
	}
	return 0;
}

int set_cloneflags(namespace_info* bitmask){

	int fl = 0;
	if (bitmask->mnt == 1)
		fl = CLONE_NEWNS;
	if (bitmask->pid == 1)
		fl |= CLONE_NEWPID;
	if (bitmask->usr == 1)
		fl |= CLONE_NEWUSER;
	if (bitmask->uts == 1)
		fl |= CLONE_NEWUTS;

	return fl;
}
	

isolproc_info* initial_info(int argc, char** argv) {
	isolproc_info* a = (isolproc_info*)malloc(sizeof(isolproc_info));
	memset(a, 0, sizeof(isolproc_info));

	int i = 1;
	while (set_flag(a, argv[i])){
		++i;
	}
	
	a->argc = argc - i;
	a->argv = argv + i;
	return a;
}
