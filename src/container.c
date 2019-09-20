#define _GNU_SOURCE // clone
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/mount.h>
#include <unistd.h>

#include "include/container.h"
#include "include/flags.h"
#include "include/namespace.h"


#define STACK_SIZE 4096*4

static char child_stack[STACK_SIZE];



static isolproc_info* parse_args(int argc, char** argv) {
	isolproc_info* a = (isolproc_info*)malloc(sizeof(isolproc_info));
	a->argc = argc;
	a->argv = argv;
	return a;
}

static void open_pipe(isolproc_info* info) {
	if (pipe(info->pipefd) == -1) {
		fprintf(stderr, "Can't create pipe to control setting, stop\n");
		exit(-1);
	}
};

static void sinch_pipe_in(isolproc_info* info) {
	if (close(info->pipefd[0]) == -1) {
		fprintf(stderr, "Can't close pipe, stop\n");
		exit(-1);
	}
	if (write(info->pipefd[1], "go.", 3) != 3) {
		fprintf(stderr, "Can't write in pipe, stop\n");
		exit(-1);
	}
	if (close(info->pipefd[1]))
	{
		fprintf(stderr, "Can't close pipe, stop\n");
		exit(-1);
	}
}

static int sinch_pipe_out(isolproc_info* info) {
	if (close(info->pipefd[1]))
	{
		fprintf(stderr, "Can't close pipe, stop\n");
		exit(-1);
	}
	char buf[3];
	int res = read(info->pipefd[0], buf, 3);

	if (close(info->pipefd[0]))
	{
		fprintf(stderr, "Can't close pipe, stop\n");
		exit(-1);
	}

	return (res == 3) ? 1 : 0;
}
	


static int isolate_process(void *arguments) {
	isolproc_info* info = (isolproc_info*) arguments;

	if(!sinch_pipe_out(info)) {
		fprintf(stderr, "Can't sinchronize with pipe, stop\n");
		exit(-1);
	}

	// set uts_namespace
	if (info->nspace.uts) {
		if (!get_new_hostname(info->hostname, 15)) {
			if(sethostname(info->hostname, 15)) {
				fprintf(stderr,"can't sethostname(), stop\n");
				exit(-1);
			}
        	}
	}
		
	// set a new mount namespace
	if (info->nspace.mnt) {	
		char mdir[] = "rootfs\0";
		strcpy(info->root, mdir);
		mount_namespace(info);
	}


	pid_t root_pid = fork();
	if (root_pid == 0) {	

		// set a new pid namespace
		if (info->nspace.pid) {
			pid_namespace(info);
		}

                cgroup_namespace(info); 
                
                if (execvp(info->argv[0], info->argv) == -1){
			printf("Exec error\n");
			free(info);
			exit(-1);
		}
	}
	else {
		wait(NULL);
                free_cgroup(info);

	}
}

int main(int argc, char** argv)
{
	isolproc_info* _info = initial_info(argc, argv);
	open_pipe(_info);

	int _clone_flags = SIGCHLD | CLONE_NEWCGROUP | set_cloneflags(&(_info->nspace));

    	pid_t isolproc_id = clone(&isolate_process, (void *) child_stack+STACK_SIZE, _clone_flags, (void*)_info);
    	if(isolproc_id == -1) {
        	printf("Clone error, stop\n");
        	return 0;
    	} else {
		_info->pid = isolproc_id;		
		// control by pipe
		sinch_pipe_in(_info);
		int status;
        	if(waitpid(isolproc_id, &status, 0) < 0) {
			if (WIFEXITED(status))
			{
				return 0;
			}
        		fprintf(stderr, "waiting\n");
        	}
        	return 0;
    	}
}
