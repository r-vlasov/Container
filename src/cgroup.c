#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <fcntl.h>
#include "include/container.h"
#include "include/cgroup.h"


#define MEMORY "1073741824"
#define SHARES "256"
#define PIDS "5"
#define WEIGHT "10"
#define FD_COUNT 64




struct cgrp_control *cgrps[] = {
        & (struct cgrp_control) {
                .control = "memory",
                .settings = (struct cgrp_setting *[]) {
                        & (struct cgrp_setting) {
                                .name = "memory.limit_in_bytes",
                                .value = MEMORY
                        },
                        & (struct cgrp_setting) {
                                .name = "memory.kmem.limit_in_bytes", 
                                .value = MEMORY
                        },
                        &add_to_tasks,
                        NULL
                }
        },
        & (struct cgrp_control) {
                .control = "cpu",
                .settings = (struct cgrp_setting *[]) {
                        & (struct cgrp_setting) {
                                .name = "cpu.shares",
                                .value = SHARES
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
                                .value = PIDS
                        },
                        &add_to_tasks,
                        NULL
                }
        },
        & (struct cgrp_control) {
                .control = "blkio",
                .settings = (struct cgrp_setting *[]) {
                        & (struct cgrp_setting) {
                                .name = "blkio.weight",
                                .value = PIDS
                        },
                        &add_to_tasks,
                        NULL
                }
        },
        NULL
};

static int cgroups(isolproc_info *config) {
        fprintf(stderr, "=> setting cgroups...");
        for (struct cgrp_control **cgrp = cgrps; *cgrp; cgrp++) {
                char dir[64] = {0};
                fprintf(stderr, "%s...", (*cgrp)->control);
                if (snprintf(dir, sizeof(dir), "/sys/fs/cgroup/%s/%s",
                             (*cgrp)->control, config->hostname) == -1) {
                        return -1;
                }
                if (mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR)) {
                        fprintf(stderr, "mkdir %s failed: %m\n", dir);
                        return -1;
                }
                for (struct cgrp_setting **setting = (*cgrp)->settings; *setting; setting++) {
                        char path[64] = {0};
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
                }
        }
        fprintf(stderr, "done.\n");
}

#include <sys/errno.h>
int cgroup_namespace(isolproc_info* config) {
    /*    if (mount("cgroup_root", "./sys/fs/cgroup", "tempfs", 0, NULL)) {
                perror("");
                fprintf(stderr, "Failed to mount cgroup_root, stop\n");
                exit(-1);
        }

        if (mkdir("./sys/fs/cgroup/cpuset", 0555)) {
                fprintf(stderr, "Failed to make directory ./sys/fs/cgroup/cpuset, stop\n");
                exit(-1);
        }

        if (mount("cpuset", "./sys/fs/cgroup/cpuset", "cgroup", MS_MGC_VAL, "cpuset")) {
                fprintf(stderr, "Failed to mount cgroup/cpuset, stop\n");
                exit(-1);
        }

        cgroups(config);
      */  return 0;
}


