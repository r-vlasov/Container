#ifndef _CONTAINER_H
#define _CONTAINER_H


#include "container.h"

typedef struct namespaces{
	int mnt ;
	int pid ;
	int net ;
	int ipc ;
	int uts ;
	int usr ;
	int cgroup ;
} namespace_info;

typedef struct cgroup{
        char mem[8] ;
        char cpus[8]  ;
        char pids[8] ;
} cgroup_info;

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



int cgroup_namespace(isolproc_info *config);
#endif
