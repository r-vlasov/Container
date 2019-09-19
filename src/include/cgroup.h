#ifndef _CGROUP_H
#define _CGROUP_H

struct cgrp_control {
        char control[256];
        struct cgrp_setting {
                char name[256];
                char value[256];
        } **settings;
};
struct cgrp_setting add_to_tasks = {
        .name = "tasks",
        .value = "0"
};


#endif
