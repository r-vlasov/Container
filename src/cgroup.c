#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>
#include "include/container.h"
#include "include/cgroup.h"

#define MAX_SIZE        64
#define DEFAULT_MEMORY "4000000"
#define DEFAULT_CPUS "0"
#define DEFAULT_PIDS "16"



struct cgrp_setting add_to_tasks = {
        .name = "tasks",
        .value = "0"
};


struct cgrp_control *cgrps[] = {
        & (struct cgrp_control) {
                .control = "memory",
                .settings = (struct cgrp_setting *[]) {
                        & (struct cgrp_setting) {
                                .name = "memory.limit_in_bytes",
                                .value = DEFAULT_MEMORY
                        },
                        &add_to_tasks,
                        NULL
                }
        },
        & (struct cgrp_control) {
                .control = "cpuset",
                .settings = (struct cgrp_setting *[]) {
                        & (struct cgrp_setting) {
                                .name = "cpuset.cpus",
                                .value = DEFAULT_CPUS
                        },
                        &add_to_tasks,
                        NULL
                }
        },
        & (struct cgrp_control) {
                .control = "pids",
                .settings = (struct cgrp_setting *[]) {
                        & (struct cgrp_setting) {
                                .name = "pids.max",
                                .value = DEFAULT_PIDS
                        },
                        &add_to_tasks,
                        NULL
                }
        },
        NULL
};


/* cat ..\*.mems return a empty line. So we can't assign a new task to the cgroup 
   as it will not have any memory to work with.
*/
static int assigned_memnodes(char* pdir, struct cgrp_control **cgrp) { 
        if (strcmp((*cgrp)->control, "cpuset\0"))
                return 0;

        char path[MAX_SIZE] = {0};
        if (snprintf(path, sizeof(path), "%s/%s.mems", pdir, (*cgrp)->control) == -1) {
                return -1;
        }
        
     
        int fd; // file descr. special for /cgroup/*.mems
        if ((fd = open(path, O_WRONLY)) == -1) {
                fprintf(stderr, "opening %s failed: %m\n", path);
                return -1;
        }
        if (write(fd, "0", 3) == -1) {
                fprintf(stderr, "writing to %s failed: %m\n", path);
                close(fd);
                return -1; 
        }
        close(fd);
        return 0;
}


static void cgroup_root_mount(isolproc_info* config) {
        if (!config->nspace.mnt)
                return;

        if (mount("cgroup_root", "./sys/fs/cgroup", "tmpfs", MS_MGC_VAL, NULL)) {
                fprintf(stderr, "Failed to mount cgroup_root, stop\n");
                exit(-1);
        }
}

static void cgr_subdir_mount(char *d) {
        char dir[MAX_SIZE] = {0};
        fprintf(stderr, "mounting %s\n", d);
        if (snprintf(dir, sizeof(dir), "./sys/fs/cgroup/%s", d) == -1) {
                fprintf(stderr, "Failed to snprintf %s\n", d);
                return;
        }
        if (mkdir(dir, 0555)) {
                fprintf(stderr, "Failed to make directory %s, stop\n", dir);
                exit(-1);
        }
        if (mount(d, dir, "cgroup", 0, d)) {
                fprintf(stderr, "Failed to mount %s, stop\n", d);
                exit(-1);
        }
}

static void cgrp_fill_values(cgroup_info* info, int i) {
       double* s = (double*) info;
       if (s[i]) {
               strcpy(((*(cgrps[i]->settings))->value), s + i);
       }
}

static int cgroups(isolproc_info *config) {
        fprintf(stderr, "=> setting cgroups...\n");
        int counter = 0; // for cgrp_fill_values;

        for (struct cgrp_control **cgrp = cgrps; *cgrp; ++cgrp, ++counter) {
                char dir[MAX_SIZE] = {0};
                fprintf(stderr, "\t%s...", (*cgrp)->control);  
                cgr_subdir_mount((*cgrp)->control);     // mounting subdirectory(cpuset, memory, ...)
                if (snprintf(dir, sizeof(dir), "/sys/fs/cgroup/%s/%s",
                             (*cgrp)->control, config->hostname) == -1) {
                        return -1;
                }
                if (mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR)) {
                        fprintf(stderr, "mkdir %s failed: %m\n", dir);
                        return -1;
                }
                cgrp_fill_values(&(config->cgrp), counter);               // if we uses cpu, memory, pids cgroup settings
                
                // specially for add new tasks   
                if (assigned_memnodes(dir, cgrp)) {
                        fprintf(stderr, "Failed to assign memory nodes: %m, stop\n");
                        return -1;
                }

                for (struct cgrp_setting **setting = (*cgrp)->settings; *setting; setting++) {
                        char path[MAX_SIZE] = {0};
                        int fd = 0;
                        if (snprintf(path, sizeof(path), "%s/%s", dir,
                                     (*setting)->name) == -1) {
                                fprintf(stderr, "snprintf failed: %m\n");
                                return -1;
                        }
                        if ((fd = open(path, O_WRONLY)) == -1) {
                                fprintf(stderr, "opening %s failed: %m\n", path);
                                return -1;
                        }
                        if (write(fd, (*setting)->value, strlen((*setting)->value)) == -1) {
                                fprintf(stderr, "writing to %s failed: %m\n", path);
                                close(fd);
                                return -1;
                        }
                        close(fd);
                } // for
        } // for
        fprintf(stderr, "done.\n");
        return 0;
}

/* When our process sh(2) was ended his children attaching to pid=1 */
/* So we need to close all process at this container */
/* Because we can't clear cgroup directory:)))) */
static int isnumber(char* str) {
        int i;
        for (i = 0; str[i] >= '0' && str[i] <= '9'; i++) { }
        if (!str[i])
                return 1;
        return 0;
}

static int select_proc(isolproc_info* info) {
        if (info->nspace.pid == 0)
                return 0;
        
        DIR *dir;
        struct dirent *ent;
        int procnum;
        if ((dir = opendir ("/proc")) != NULL) {
                while ((ent = readdir(dir)) != NULL) {
                        if (isnumber(ent->d_name)) {        
                                procnum = atoi(ent->d_name);
                                if (procnum != 1) {
                                        char buf[64] = {0};
                                        snprintf(buf, sizeof(buf), "kill -9 %s", ent->d_name);
                                        system(buf);
                                }
                        }
                        continue;
                } // while
                return 0;
        }
        else {
                fprintf(stderr, "Can't open directory /proc, stop\n");
                return 1;
        }
}

int free_cgroup(isolproc_info* config) {
        if (select_proc(config)) {
                fprintf(stderr, "Selecting /proc error, stop\n");
                exit(-1);
        }
        fprintf(stderr, "freeing cgroup resources...");
        for (struct cgrp_control **cgrp = cgrps; *cgrp; cgrp++) {
                char dir[64] = {0};
                char task[64] = {0};
                int task_fd = 0;
                if (snprintf(dir, sizeof(dir), "/sys/fs/cgroup/%s/%s", (*cgrp)->control,
                        config->hostname) == -1 ||
                    snprintf(task, sizeof(task), "/sys/fs/cgroup/%s/tasks", 
                        (*cgrp)->control) == -1) {
                        fprintf(stderr, "snprintf failed: %m\n");
                        return -1;
                }
                if ((task_fd = open(task, O_WRONLY)) == -1) {
                        fprintf(stderr, "opening %s failed: %m\n", task);
                        return -1;
                }
                if (write(task_fd, "0", 2) == -1) {
                        fprintf(stderr, "writing to %s failed: %m\n", task);
                        close(task_fd);
                        return -1;
                } 
                close(task_fd);
                if (rmdir(dir)) {
                        fprintf(stderr, "rmdir %s failed: %m", dir);
                        return -1;
                }
        } // for
        fprintf(stderr, "done.\n");
        return 0;
}

int cgroup_namespace(isolproc_info* config) {
        
        cgroup_root_mount(config);
        if (cgroups(config))
        {
                fprintf(stderr, "Can't set cgroup namespace!");
                exit(-1);
        }
        
        return 0;
}
