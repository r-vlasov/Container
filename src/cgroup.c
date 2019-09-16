#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include "include/container.h"
#define PIDS "20\0"
 // cgroup let us limit computer resources like mem, cputime, devices, pids and etc
struct cgrp_control {
        char control[256];
        struct cgrp_setting {
                char name[256];
                char value[256];
        } **settings;
};


struct cgrp_setting add_to_task = {
        .name = "tasks",
        .value = "0"
};

struct cgrp_control *cgrps[2];
#include <stdio.h>

int cgroup_namespace(isolproc_info* info) {
        strcpy(cgrps[0]->control, "pid\0");
        strcpy(cgrps[0]->settings[0]->name, "pids.max\0");
        strcpy(cgrps[0]->settings[0]->value, PIDS);

        fprintf(stderr, "setting cgroups...\n");
        for (struct cgrp_control **cgrp = cgrps; *cgrp != NULL; cgrp++) {
                fprintf(stderr, "hell05555\n");
             //   char dir[30];
                fprintf(stderr, "%s...", (*cgrp)->control);
             //   if (snprintf(dir, sizeof(dir), "/sys/fs/cgroup/%s/%s",(*cgrp)->control, info->hostname) == -1) {
             //           fprintf(stderr, "sddd\n");
             //           return -1;
             //   }

        /*        if (mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR)) {
                        fprintf(stderr, "mkdir %s failed: %m\n", dir);
                        return -1;
                }

                for (struct cgrp_setting **setting = (*cgrp)->settings; *setting; setting++) {
                        char path[64] = {0};
                        int fd = 0;
                        if (snprintf(path, sizeof(path), "%s/%s", dir, (*setting)->name) == -1) {
                                fprintf(stderr, "snprintf failed\n");
                                return -1;
                        }

                }
*/        }
        fprintf(stderr, "hello1\n");
        fprintf(stderr, "done\n");
        return 0;
}
