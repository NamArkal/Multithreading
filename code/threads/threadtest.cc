#include "kernel.h"
#include "main.h"
#include "process.h"

int counter = 2;
int tcount = 1;

void
SimplePrint(){
   //printf("*** thread with name %c is forked for process %s \n", kernel->currentProcess->currentThread->getName(), kernel->currentProcess->getName());
   std::cout << "Thread " << tcount++  << " is forked for process " << kernel->currentProcess->getName() << endl;
}

void
SimpleThread(int which)
{
	printf("---------------------------------- \n");
        printf("Process with process id %d and priority %d is forked \n", kernel->currentProcess->getPid(), kernel->currentProcess->getPriority());
        kernel->currentProcess->ForkProcessThreads(counter++,(VoidFunctionPtr) SimplePrint, (void *) 1);
        kernel->currentProcess->Yield();
	kernel->currentProcess->Terminate();
}

void
ThreadTest()
{
    std::cout << "In threadtest main\n";
    Process *p1 = new Process(4, "SimpleThread forked process1, P4");
    Process *p2 = new Process(1, "SimpleThread forked process2, P5");
    Process *p3 = new Process(5, "SimpleThread forked process3, P5");
    Process *p4 = new Process(2, "SimpleThread forked process4, P2");
    Process *p5 = new Process(3, "SimpleThread forked process5, P3");
    p1->Fork((VoidFunctionPtr) SimpleThread, (void *) 1);
    p2->Fork((VoidFunctionPtr) SimpleThread, (void *) 1);
    p3->Fork((VoidFunctionPtr) SimpleThread, (void *) 1);
    p4->Fork((VoidFunctionPtr) SimpleThread, (void *) 1);
    p5->Fork((VoidFunctionPtr) SimpleThread, (void *) 1);

    p2->CreateChildProcess();
    p4->CreateChildProcess();
    p4->CreateChildProcess();
    //p4->Join(); 
    kernel->currentProcess->Yield();
    IntStatus oldLevel;
    oldLevel = kernel->interrupt->SetLevel(IntOff);
    while(kernel->scheduler->processReadyList->NumInList() >0)
    {
	kernel->currentProcess->Yield();
    }
	std::cout<<"Back to main thread \n";
    kernel->interrupt->SetLevel(IntOn);
}
