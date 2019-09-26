# Custom Linux Container
Custom Linux container written in C with the help of namespace and cgroup mechanisms

## Instalation

Use the GNU utility make in your directory

'''bash
make run
'''

## Usage

Run the program 'container.exe' as a root

## Launch options

'''bash
        -U      -       runs program in new UTS namespace
        -m      -       runs program in new mount namespace
        -p      -       runs program in new PID namespace
        
        cpu:[]  -       selects the CPUs where the isolate programm will run
        mem:[]  -       selects the number of memory used by isolate program
        pid:[]  -       selects the largest number of processes running by isolate program

For example:
        sudo ./container.exe -U -m -p cpu:0-3 pid:8 mem:120M sh
'''
