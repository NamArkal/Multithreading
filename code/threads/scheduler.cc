#include "../lib/debug.h"
#include "scheduler.h"
#include "main.h"

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------

Scheduler::Scheduler()
{ 
    readyList = new List<Thread *>;
    toBeDestroyed = NULL;
} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{ 
    delete readyList; 
} 

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void
Scheduler::ReadyToRun (Thread *thread)
{
//std::cout << "In thread ready to run\n";
ASSERT(kernel->interrupt->getLevel() == IntOff);
DEBUG(dbgThread, "Putting thread on ready list: " << thread->getName());

thread->setStatus(READY);
readyList->Append(thread);
//  Print();
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun ()
{
 //std::cout << "In thread FindNextToRun\n";
 ASSERT(kernel->interrupt->getLevel() == IntOff);

 if (readyList->IsEmpty()) {
    return NULL;
  } else {
    return readyList->RemoveFront();
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
Scheduler::Run (Thread *nextThread, bool finishing)
{
 //std::cout << "In thread run\n";
 Thread *oldThread = kernel->currentProcess->currentThread;

 ASSERT(kernel->interrupt->getLevel() == IntOff);

 if (finishing) {   // mark that we need to delete current thread
 ASSERT(toBeDestroyed == NULL);
toBeDestroyed = oldThread;
}

 if (oldThread->space != NULL) {    // if this thread is a user program,
  oldThread->SaveUserState();   // save the user's CPU registers
    oldThread->space->SaveState();
  }

oldThread->CheckOverflow();         // check if the old thread had an undetected stack overflow

 kernel->currentProcess->currentThread = nextThread;  // switch to the next thread
 nextThread->setStatus(RUNNING);      // nextThread is now running

DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());

SWITCH(oldThread, nextThread);

ASSERT(kernel->interrupt->getLevel() == IntOff);

DEBUG(dbgThread, "Now in thread: " << oldThread->getName());

CheckToBeDestroyed();       // check if thread we were running
 //before this one has finished
 //and needs to be cleaned up

if (oldThread->space != NULL) {     // if there is an address space
  oldThread->RestoreUserState();     // to restore, do it.
oldThread->space->RestoreState();
 }
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void
Scheduler::CheckToBeDestroyed()
{
  if (toBeDestroyed != NULL) {
      delete toBeDestroyed;
	toBeDestroyed = NULL;
   }
}
 
//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void
Scheduler::Print()
{
    cout << "Thread ready list contents:\n";
    readyList->Apply(ThreadPrint);
}

