#include "include/container.h"

#define _GNU_SOURCE
#include <sched.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>



static int set_nspace(isolproc_info* info, char* arg) {
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

const static char *cgroup_params[] = {
	"mem:",
	"cpu:",
	"pid:"
};

const static int cgroup_params_number = 3;


/* 
 * compare_prefix(str1, str2) - is the function that return 0 when
 * str1 is prefix of str2 and any char without 0 when it isn't prefix
 */    
static int compare_prefix(const char* str1, const char* str2) {
        int i;
        for (i = 0; str1[i] == str2[i] && str1[i] != '\0'; ++i);
        if (str1[i] == '\0')
                return 0;
        return str1[i] - str2[i];
}

static int set_cgroup(isolproc_info* info, char* arg) {
        cgroup_info* cgrinfo = &(info->cgrp);
        
        for (int i = 0; i < cgroup_params_number; ++i) {
                if (!compare_prefix(cgroup_params[i], arg)) {
                        // there will be the hack
                        strcpy(cgrinfo->mem + i * 8, arg + 4);
                        return 1;   
                }
        }
                 
       /* if (arg[0] == 'c' && arg[1] == 'p' && arg[2] == 'u' && arg[3] == '-') {
                strcpy(cgrinfo->cpus, arg+4);
                return 1;
        }
        if (arg[0] == 'p' && arg[1] == 'i' && arg[2] == 'd' && arg[3] == '-') {
                strcpy(cgrinfo->pids, arg+4);
                return 1;
        }
        if (arg[0] == 'm' && arg[1] == 'e' && arg[2] == 'm' && arg[3] == '-') {
                strcpy(cgrinfo->mem, arg+4);
                return 1;
        }
 */       return 0;
        
}

int set_cloneflags(namespace_info* bitmask){

	int fl = 0;
	if (bitmask->mnt == 1)
		fl = CLONE_NEWNS;
	if (bitmask->pid == 1)
		fl |= CLONE_NEWPID;
	if (bitmask->uts == 1)
		fl |= CLONE_NEWUTS;

	return fl;
}
	

isolproc_info* initial_info(int argc, char** argv) {
	isolproc_info* a = (isolproc_info*)malloc(sizeof(isolproc_info));
	memset(a, 0, sizeof(isolproc_info));

	int i = 1;
	while (set_nspace(a, argv[i])){
		++i;
	}

        while (set_cgroup(a, argv[i])){
                ++i;
        }

	a->argc = argc - i;
	a->argv = argv + i;
	return a;
}
