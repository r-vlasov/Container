#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <syscall.h>
#include <string.h>

#include "include/container.h"


static void write_file(char* path, char* line){
	int fd = open(path, O_WRONLY);
	if (fd < 0){
		fprintf(stderr, "Can't open mapping file, stop\n");
	}
	write(fd, line, strlen(line));
	close(fd);
}


int get_new_hostname(char* buf, unsigned len){
	printf("Enter your new hostname(maxlen = 14): ");   
	scanf("%s", buf);
	return 0;
}



// mapping uid links UID from usernamespace1 to usernamespace2
int user_namespace(isolproc_info* _info){
	char path[100];
	char line[100];
	int  uid = 1000;

	sprintf(path, "/proc/%d/uid_map", _info->pid);
	sprintf(line, "0 %d 1\n", uid);
	write_file(path, line);

	sprintf(path, "/proc/%d/setgroups", _info->pid);
	sprintf(line, "0 %d 1\n", uid);	
	write_file(path, line);

	sprintf(path, "/proc/%d/gid_map", _info->pid);
	sprintf(line, "0 %d 1\n", uid);
	write_file(path, line);
	return 0;
}


const char put_old[] = ".put_old";


// mounting the new root (CLONE_NEWNS allows child to start in a new mount namespace)
int mount_namespace(isolproc_info* _info){


	const char* mnt = _info->root;
        
        fprintf(stderr, "=> remounting everything with MS_PRIVATE...");
        if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL)) {
                fprintf(stderr, "failed! %m\n");
                exit(-1);
        }
        fprintf(stderr, "remounted.\n");

	if (mount(_info->root, mnt, NULL, MS_BIND, "")) {
		fprintf(stderr, "Failed to mount %s at %s: %m, stop\n", _info->root, mnt);
		exit(-1);
	}
	if (chdir(mnt)) {
		fprintf(stderr, "Failed to chdir to rootfs mounted at %s : %m, stop\n", mnt);
		exit(-1);
	}

	if (mkdir(put_old, 0777) && errno != EEXIST) {
		fprintf(stderr, "Failed to mkdir put_old %s: %m, stop\n", put_old);
		exit(-1);
	}

	if (syscall(SYS_pivot_root, ".", put_old)) {
		fprintf(stderr, "Failed to mkdir put_old %s: %m, stop\n", put_old);
		exit(-1);
	}

	if (chdir("/")){
		fprintf(stderr, "Can't change directory to root, stop\n");
		exit(-1);
	}
	
	if (!_info->nspace.pid) {
		if (umount2(put_old, MNT_DETACH)) {
			fprintf(stderr, "Failed to umount put.old %s: %m, stop\n", put_old);
			exit(-1);
		}
	}
	return 0;
}


int pid_namespace(isolproc_info* _info) {

	if (mkdir("./proc", 0555) && errno != EEXIST) {
		fprintf(stderr, "Failed to make /proc directory, stop\n");
		exit(-1);
	}

             
	if (mount("proc", "./proc", "proc", 0, "")) {
		fprintf(stderr, "Failed to mount proc, stop\n");
		exit(-1);
	}
 
 	if (mount("sysfs", "./sys", "sysfs", 0, "")) {
                fprintf(stderr, "Failed to mount cgroup/cpu, stop\n");
		exit(-1);
	}


/*
        if (mkdir("/sys/fs/cgroup/cpu", 0555) && errno != EEXIST) {
		fprintf(stderr, "Failed to make /sys/fs/cgroup/cpu directory, stop\n");
		exit(-1);
	}
      if (mount("cgroup", "./sys/fs/cgroup", "cgroup", 0, "all")) {
                perror("");
                fprintf(stderr, "Failed to mount cgroup, stop\n");
		exit(-1);
	}
*/       

        if (umount2(put_old, MNT_DETACH)) {
                fprintf(stderr, "Failed to umount put.old, stop\n");
                exit(-1);
        }
	return 0;
}
