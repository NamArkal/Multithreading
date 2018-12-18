//
// Created by Namrata A on 10/22/18.
//

#include "../lib/debug.h"
#include "../machine/interrupt.h"
#include "process.h"
#include "../lib/copyright.h"
#include "thread.h"
#include "switch.h"
#include "synch.h"
#include "../lib/sysdep.h"
#include "scheduler.h"

int Process::pidCounter=0;

void ProcessPrint(Process *p) { p->Print(); }

Process::Process(int priority, char* name) {
//    std::cout << "In process constructor\n";
    this->priority = priority;
    this->name = name;
    status = P_JUST_CREATED;
    pid = pidCounter++;
    cThreads = new List<Thread*>();
    cProcess = new List<Process*>();
    Thread *t = new Thread("main thread");
    // cThreads->Append(t);
    this->currentThread = t;
    //this->currentThread->setStatus(RUNNING); 
    this->pScheduler = new Scheduler();
}

void Process::Fork(VoidFunctionPtr func, void *arg) {
  //  std::cout << "The process thread has been forked\n";
    std::cout<<"\n----In Process Fork----\n";
    Interrupt *interrupt = kernel->interrupt;
//    ProcessScheduler *scheduler = kernel->scheduler;
    IntStatus oldLevel;
    //Thread *t = new Thread("forked thread");
    this->currentThread->Fork(this, (VoidFunctionPtr) func, (void *) arg);
    cThreads->Append(currentThread);
    // this->currentThread = t;
    oldLevel = interrupt->SetLevel(IntOff);
    pScheduler->FindNextToRun();
    //oldLevel = interrupt->SetLevel(IntOff);
    kernel->scheduler->ReadyToRun(this);
    //kernel->currentProcess->Yield();
    //this->pScheduler->ReadyToRun(this->currentThread);
    (void) interrupt->SetLevel(oldLevel);

    funcPtr = func;
    args = arg;
}

void Process::ForkProcessThreads(int n,VoidFunctionPtr func, void *arg) {
    Thread *t[n+2];
    for(int i=0;i<n;i++)
    {
        t[i] = new Thread("Child thread of process");
	t[i]->Fork(this, (VoidFunctionPtr) func, (void *) arg);
	
    }
     kernel->interrupt->SetLevel(IntOff);
     kernel->currentProcess->currentThread->Yield();
     kernel->interrupt->SetLevel(IntOn);
}




void Process::Terminate() {
   // std::cout << "In process terminate\n";
    std::cout <<"\n---- In Process Termination ----\n";
    (void) kernel->interrupt->SetLevel(IntOff);     
    //ASSERT(this == kernel->currentProcess);
    DEBUG(dbgThread, "Terminating Process: " << name);
    std::cout << "Termination Process: " << name << "\n";
    Sleep(TRUE);

    isFinished = true;
}

void
Process::Sleep (bool finishing)
{
    //std::cout << "In process sleep\n";
    Process *nextProcess;

    ASSERT(this == kernel->currentProcess);
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    
    status = P_BLOCKED;
    while ((nextProcess = kernel->scheduler->FindNextProcessToRun()) == NULL)
        kernel->interrupt->Idle();	// no one to run, wait for an interrupt

    // returns when it's time for us to run
    kernel->scheduler->Run(nextProcess, finishing);
    //this->currentThread->Sleep(finishing);
}

void
Process::Yield ()
{
    std::cout<<"\n---- Process Yield ----\n";
    Process *nextProcess;
    IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
    
    ASSERT(this == kernel->currentProcess);
    
    DEBUG(dbgThread, "Yielding process: " << name);
    std::cout << "Yielding process: "<< name << " Process priority: " << getPriority() << "\n";
    
    nextProcess = kernel->scheduler->FindNextProcessToRun();
    if (nextProcess != NULL) {
	    kernel->scheduler->ReadyToRun(this);
	    kernel->scheduler->Run(nextProcess, FALSE);
            //this->currentThread->Yield();
    }
    (void) kernel->interrupt->SetLevel(oldLevel);
}

void
Process::CreateChildThread (VoidFunctionPtr func, void *arg) {
    Thread *t = new Thread("Child Thread");
    t->Fork(this, func, arg);
    cThreads->Append(t);
}

void
Process::CreateChildProcess() {
    std::cout <<"\n---- Process ChildProcess ----\n";
    Process *p = new Process(this->priority, "Child process");
    p->parentId = this->pid;
    p->Fork(this->funcPtr, this->args);
    cProcess->Append(p);
    cout<< "The child process: "<< p->getName() << " and has the priority: " << getPriority() << "\n";
}

void
Process::Join() {
    kernel->liPr->Append(this);
    this->Sleep(FALSE);
}

Process::~Process() {
    delete cThreads;
    delete cProcess;
    delete pScheduler;
    delete currentThread;
}
