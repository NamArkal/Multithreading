#ifndef PROCESSSCHEDULER_H
#define PROCESSSCHEDULER_H

#pragma once
#include "../lib/copyright.h"
#include "../lib/list.h"
//#include "thread.h"
#include "process.h"

// The following class defines the scheduler/dispatcher abstraction -- 
// the data structures and operations needed to keep track of which 
// thread is running, and which threads are ready but not running.
class Process;
//class Thread;
class ProcessScheduler {
  public:
    ProcessScheduler();                // Initialize list of ready threads 
    ~ProcessScheduler();               // De-allocate ready list

    //void ReadyToRun(Thread* thread);  
    void ReadyToRun(Process* process);
                                // Thread can be dispatched.
    //Thread* FindNextToRun();  // Dequeue first thread on the ready 
    Process* FindNextProcessToRun();
                                // list, if any, and return thread.
    //void Run(Thread* nextThread, bool finishing);
    void Run(Process* nextProcess, bool finishing);
                                // Cause nextThread to start running
    void CheckToBeDestroyed();// Check if thread that had been
                                // running needs to be deleted
    void Print();               // Print contents of ready list
    void CheckIfParentUnblocked();

    // SelfTest for scheduler is implemented in class Thread

    SortedList<Process *> *processReadyList;  // queue of threads that are ready to run,
  private:
    //List<Thread *> *readyList;
    //List<Process *> *processReadyList;
                                // but not running
    //Thread *toBeDestroyed;
    Process *processToBeDestroyed;      // finishing thread to be destroyed
                                // by the next thread that runs
};

#endif // PROCESSSCHEDULER_H
