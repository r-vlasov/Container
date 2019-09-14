#ifndef _NAMESPACE_H
#define _NAMESPACE_H

int get_new_hostname(char* buf, unsigned len);
int user_namespace(isolproc_info* _info);
int mount_namespace(isolproc_info* _info);
int pid_namespace(isolproc_info* _info);


#endif 
