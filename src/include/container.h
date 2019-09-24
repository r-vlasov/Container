#ifndef _CONTAINER_H
#define _CONTAINER_H

#include "container.h"

typedef struct namespaces{
	unsigned int mnt : 1;
        unsigned int pid : 1;
	unsigned int net : 1;
	unsigned int ipc : 1;
        unsigned int uts : 1;
} namespace_info;

typedef struct cgroup{
        char mem[8] ;
        char cpus[8]  ;
        char pids[8] ;
} __attribute__((packed)) cgroup_info;

typedef struct isolproc_info{
	int 		argc;
	char** 		argv;

	char		root[7];
	char 		hostname[15];
	
	int 		pipefd[2];

	int 		pid;
	namespace_info  nspace;
        cgroup_info     cgrp;
} isolproc_info;


#endif
