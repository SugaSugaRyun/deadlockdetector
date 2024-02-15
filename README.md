# Deadlock detector  

#### What is 'Deadlock'?  
>A deadlock occurs when each of a set of threads is blocked, waiting for another thread in the set to satisfy certain condition (wait shared resource, signal or event)

---  


## Two techniques of deadlock detecting
### 1. Cyclic deadlock monitoring algorithm 
- Monitor lock acquires and releases in runtime  
- Lock graph  
> • Create a node nX when a thread acquires lock X  
• Create an edge (nX, nY) when a thread acquires lock Y while holding lock X   
• Remove nX , (nX,*) and (*, nX) when a thread releases X  
→ Report deadlock when the graph has any cycle
  
<br>  

---   

### 2. Deadlock Prediction Technique  
- Potential cyclic deadlock detection algorithm [Harrow, SPIN 00]
>• Create a node nX when a thread acquires lock X  
• Create an edge (nX, nY) when a thread acquires lock Y while holding lock X   
~~• Remove nX , (nX,*) and (*, nX) when a thread releases X~~  
→Report potential deadlocks if the resulted graph at the end of an execution has a cycle
<br>

Main difference between two strategy is deleteing released lock  

## Goal of this project  
> Develop a program to detect deadlocks.  
This program will make output which contains mutex address, thread id what call pthread_mutex_lock() and line number of pthread_mutex_lock() function call involved to make deadlock.  

## How to build
    $make clean
    $make

## How to run
You have to execute checking program first, and run target program with following command.

    $./ddchck
    $LD_PRELOAD="./ddmon.so" ./target

If result not produced well, then remove FIFO(named pipe).  
Name of FIFO is ".ddtrace"  

    $rm .ddtrace
