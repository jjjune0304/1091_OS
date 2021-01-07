# 109-1 Operating Sysytem (Farn Wang)

* We use Nachos to simulate an operating system in this project.

Build nachos with the following commands:
```cmd=
cd nachos-4.0/code/
make 
```

## HW1: Multiprogramming
* An executing user program is a process. 
* Nachos is modified to support multiple processes.

Run multiple programs with the following commands:
```cmd=
cd nachos-4.0/code/usrprog
./nachos -e ../test/test1 -e ../test/test2
```

## HW2: CPU Scheduling
* Nachos is modified to support multiple CPU scheduling algorithms.
* The supported CPU scheduling are Round Robin, Shortest Job First and First Come First Service.

Specify the CPU scheduling algorithm (RR, SJF, FCFS) with the following commands:
```cmd=
cd nachos-4.0/code/usrprog
./nachos SJF -e ../test/test1 -e ../test/test2 -e ../test/test3
```


## HW3: Memory Management
* Nachos is modified to support virtual memory.

Run test cases with the following commands:
```cmd=
cd nachos-4.0/code/usrprog
./nachos -e ../test/matmult -e ../test/sort
```
* test/matmult.c : should return 7220
* test/sort.c : A[0] of sorted array should be 1
