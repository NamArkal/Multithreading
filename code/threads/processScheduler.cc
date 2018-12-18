#include "../lib/debug.h"
#include "processScheduler.h"
#include "main.h"

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------

int compare(Process *p1, Process *p2){
    if(p1->getPriority() <  p2->getPriority())
        return 1;
    else if(p1->getPriority() > p2->getPriority())
        return -1;
    else return 0;
}

ProcessScheduler::ProcessScheduler()
{    
    processReadyList = new SortedList<Process *>(compare);
    processToBeDestroyed = NULL;
} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

ProcessScheduler::~ProcessScheduler()
{ 
    delete processReadyList;
} 

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void
ProcessScheduler::ReadyToRun (Process *process)
{
//    std::cout<<"In process ready to run \n";
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    DEBUG(dbgThread, "Putting process on ready list: " << process->getName());
    std::cout << "Putting process on ready list: " << process->getName() << endl;
    process->setStatus(P_READY);
    processReadyList->Insert(process);
    //processReadyList->Append(process);
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Process *
ProcessScheduler::FindNextProcessToRun ()
{
//    std::cout << "In process find next to run\n";
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (processReadyList->IsEmpty()) {
	return NULL;
    } else {
    	return processReadyList->RemoveFront();
    }
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable kernel->currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//	"finishing" is set if the current thread is to be deleted
//		once we're no longer running on its stack
//		(when the next thread starts running)
//----------------------------------------------------------------------

void
ProcessScheduler::Run (Process *nextProcess, bool finishing)
{
    //std::cout << "In process run\n";
    Process *oldProcess = kernel->currentProcess;
    Thread * oldthead = kernel->currentProcess->currentThread;
    
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (finishing) {	// mark that we need to delete current thread
         ASSERT(processToBeDestroyed == NULL);
	 processToBeDestroyed = oldProcess;
    }
    if(oldthead->space!= NULL){
	oldthead->SaveUserState();
	oldthead->space->SaveState();
    }
    oldthead->CheckOverflow();
    
   // kernel->currentProcess = nextProcess;  // switch to the next thread
//    nextProcess->setStatus(P_RUNNING);      // nextThread is now running
    
    Thread *nextThread = nextProcess->currentThread;
    kernel->currentProcess = nextProcess;
    nextProcess->setStatus(P_RUNNING);
    nextThread->setStatus(RUNNING);
   // DEBUG(dbgThread, "Switching from: " << oldProcess->getName() << " to: " << nextProcess->getName());
   // kernel->currentProcess->currentThread = nextThread;  
   // nextProcess->currentThread->setStatus(RUNNING);    
    SWITCH(oldthead, nextThread);

    ASSERT(kernel->interrupt->getLevel() == IntOff);

   // DEBUG(dbgThread, "Now in thread: " << oldProcess->getName());

    //CheckIfParentUnblocked();
    
    CheckToBeDestroyed();	// check if thread we were running
    if (oldthead->space != NULL) {	    // if there is an address space
        oldthead->RestoreUserState();     // to restore, do it.
	oldthead->space->RestoreState();
    }
}

void
ProcessScheduler::CheckIfParentUnblocked() {
    bool allDone = true;
    int parentId = kernel->currentProcess->parentId;
    Process* toBeRun;
    ListIterator<Process*> *li = new ListIterator<Process*>(kernel->liPr);
    List<Process*> *lst;
    while(!li->IsDone()){
        if(li->Item()->getPid()==parentId){
            toBeRun = li->Item();
            *lst = toBeRun->getCProcess();
        }
        li->Next();
    }

    ListIterator<Process*> *liCh= new ListIterator<Process*>(lst);
    while (!liCh->IsDone()) {
        if(!liCh->Item()->isFinished)
            allDone = false;
        liCh->Next();
    }

    if(allDone)
        ReadyToRun(toBeRun);
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void
ProcessScheduler::CheckToBeDestroyed()
{
    if (processToBeDestroyed != NULL) {
        delete processToBeDestroyed;
	processToBeDestroyed = NULL;
    }
}
 
//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void
ProcessScheduler::Print()
{
    cout << "Process ready list contents:\n";
    processReadyList->Apply(ProcessPrint);
}

