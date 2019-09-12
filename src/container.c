#define _GNU_SOURCE // clone
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#define STACK_SIZE 4096
static char child_stack[STACK_SIZE];


// struct of args which we push in isolate process
struct p_args{
	int argc;
	char** argv;
};

static struct p_args* parse_args(int argc, char** argv) {
	struct p_args* a = (struct p_args*)malloc(sizeof(struct p_args));
	a->argc = argc;
	a->argv = argv;

	return a;
}


static int isolate_process(void *arguments) {
	struct p_args* args = (struct p_args*) arguments;
	pid_t root_pid = fork();
	if (root_pid == 0) {
		execle("/bin/ls", "/bin/ls", "-l", "/proc/1/ns", NULL);
		if (execvp(args->argv[1], args->argv + 1) == -1){
			printf("Exec error\n");
			free(args);
			exit(-1);
		}
	} else {
		int status;
		waitpid(root_pid, &status, 0);
		if (!WIFEXITED(status)){
			waitpid(root_pid, &status, 0);
		}
	}
	free(args);
    	return 0;
}

int main(int argc, char** argv)
{
	struct p_args* isolproc_args = parse_args(argc, argv);
	
	int _clone_flags = SIGCHLD 	| 	CLONE_NEWPID 	|
			   CLONE_NEWIPC	|	CLONE_NEWCGROUP |
			   CLONE_NEWNET	|	CLONE_NEWNS 	| 
			   CLONE_NEWUTS | 	CLONE_NEWUSER;

    	pid_t isolproc_id = clone(&isolate_process, (void *) child_stack+STACK_SIZE, _clone_flags, (void*)isolproc_args);

	
//	set_userns(isolproc_id);
//	set_netns(isolproc_id);


    	if(isolproc_id == -1) {
        	printf("Clone error\n");
        	return 0;
    	} else {
		int status;
        	if(waitpid(isolproc_id, &status, 0) < 0) {
			if (WIFEXITED(status))
			{
				return 0;
			}
        		printf("waiting\n");
        	}
		execle("/bin/ls", "/bin/ls", "-l", "/proc/1/ns", NULL);
        	return 0;
    	}
}
