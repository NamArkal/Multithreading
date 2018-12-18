//
// Created by Namrata A on 10/22/18.
//

#ifndef NACHOS_PROCESS_H
#define NACHOS_PROCESS_H

#pragma once
#include "thread.h"
#include "../lib/list.h"
#include "scheduler.h"
#include "kernel.h"

#include "../lib/copyright.h"
#include "../lib/utility.h"
#include "../lib/sysdep.h"

#include "../machine/machine.h"
#include "addrspace.h"


enum ProcessStatus { P_JUST_CREATED, P_RUNNING, P_READY, P_BLOCKED };
class Thread;
class Scheduler;
class Kernel;

class Process {
    int priority;
    //List<Thread*> *cThreads;
    List<Process*> *cProcess;
    ProcessStatus status;
    char* name;
    int pid;
    static int pidCounter;
    VoidFunctionPtr funcPtr;
    void *args;

public:
    Process(int priority, char* name);
    Process();
    virtual ~Process();
    void ForkProcessThreads(int n,VoidFunctionPtr func, void *arg);
    int getPriority() const { return priority; };
    List<Thread*> getCThreads() { return *cThreads; }
    List<Process*> getCProcess() { return *cProcess; }
    void Fork(VoidFunctionPtr func, void *arg);
    void Terminate();
    void setStatus(ProcessStatus st) { status = st; }
    char* getName() { return (name); }
    int getPid() const { return pid; };
    void Sleep(bool finishing);
    void Print() { cout << name; }
    void Yield();
    void CreateChildThread(VoidFunctionPtr func, void *arg);
    void CreateChildProcess();
    void Join();

    Scheduler *pScheduler;
    Thread *currentThread;
    int parentId;
    bool isFinished;
    List<Thread*> *cThreads;

    bool operator == (Process p) {
        if(this->getPid() == p.getPid())
            return true;
        else return false;
    }
};

extern void ProcessPrint(Process *p);	

#endif //NACHOS_PROCESS_H
