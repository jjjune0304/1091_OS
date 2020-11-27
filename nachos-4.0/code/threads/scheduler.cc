// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
#include "scheduler.h"
#include "main.h"

int SleepThreadCompare(SleepThread* a, SleepThread* b)
{
    if(a->getSleepTime() == b->getSleepTime())
        return 0;
    return a->getSleepTime() > b->getSleepTime() ? 1 : -1;
}

int SJFCompare(Thread *a, Thread *b) {
    if(a->getBurstTime() == b->getBurstTime())
        return 0;
    return a->getBurstTime() > b->getBurstTime() ? 1 : -1;
}

int FCFSCompare(Thread *a, Thread *b) {
    return 1;
}

//----------------------------------------------------------------------
// Scheduler::Scheduler
//          Initialize the list of ready but not running threads.
//          Initially, no ready threads.
//----------------------------------------------------------------------

Scheduler::Scheduler(SchedulerType type)
{
    schedulerType = type;
    switch(schedulerType) {
    case RR:
        cout << "RR scheduling \n";
        readyList = new List<Thread *>;
        break;
    case SJF:
        cout << "SJF scheduling \n";
        readyList = new SortedList<Thread *>(SJFCompare);
        break;
    case FCFS:
        cout << "FCFS scheduling \n";
        readyList = new SortedList<Thread *>(FCFSCompare);
        break;
    }
    sleepList = new SortedList<SleepThread *> (SleepThreadCompare);
    toBeDestroyed = NULL;
} 

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------

Scheduler::Scheduler()
{
    Scheduler(RR);
} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{ 
    delete sleepList;
    delete readyList;
}

//----------------------------------------------------------------------
// Scheduler::PutToSleep
//          Put a thread to sleep.
//----------------------------------------------------------------------

void Scheduler::PutToSleep(int sleepTime)
{
    IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
    Thread* t = kernel->currentThread;

    // set burst time
    int actualBurst = kernel->stats->userTicks - t->getStartTime();
    t->setBurstTime(0.5*t->getBurstTime() + 0.5*actualBurst);
    t->setStartTime(kernel->stats->userTicks);

    SleepThread* st= new SleepThread(t,sleepTime);
    sleepList->Insert(st); // put a thread into sleep list
    t->Sleep(FALSE);
    kernel->interrupt->SetLevel(IntOn);
} 

//----------------------------------------------------------------------
// Scheduler::IsSleepListEmpty()
//          Return true if sleep list is empty.
//----------------------------------------------------------------------

bool Scheduler::IsSleepListEmpty()
{
    return sleepList->IsEmpty();
}

//----------------------------------------------------------------------
// Scheduler::AlarmTicks()
//          Check if there exists any thread that its sleeping time is due,
//          and put the woken threads into the ready queue and remove them from sleep list.
//          Return true if there exists any woken thread.
//----------------------------------------------------------------------

bool Scheduler::AlarmTicks()
{
    bool woken = false;
    ListIterator<SleepThread*> iter(sleepList);
    for (; !iter.IsDone(); iter.Next()) {
         iter.Item()->decreaseTime();
    }
    while(!sleepList->IsEmpty()){
        SleepThread* st = sleepList->Front();
        if(st->getSleepTime() == 0){
            ReadyToRun(st->getSleeper());
            sleepList->RemoveFront();
            delete st;
            woken = true;
        }
        else { break; }
    }
    return woken;
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
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    DEBUG(dbgThread, "Putting thread on ready list: " << thread->getName());
    thread->setStatus(READY);
    readyList->Append(thread);
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
    Thread *oldThread = kernel->currentThread;
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (finishing) {	// mark that we need to delete current thread
         ASSERT(toBeDestroyed == NULL);
	 toBeDestroyed = oldThread;
    }
    
#ifdef USER_PROGRAM			// ignore until running user programs 
    if (oldThread->space != NULL) {	// if this thread is a user program,
        oldThread->SaveUserState(); 	// save the user's CPU registers
	oldThread->space->SaveState();
    }
#endif
    
    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow

    kernel->currentThread = nextThread;  // switch to the next thread
    nextThread->setStatus(RUNNING);      // nextThread is now running
    kernel->currentThread->setStartTime(kernel->stats->userTicks); // Add
    
    DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    SWITCH(oldThread, nextThread);

    // we're back, running oldThread
      
    // interrupts are off when we return from switch!
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    DEBUG(dbgThread, "Now in thread: " << oldThread->getName());

    CheckToBeDestroyed();		// check if thread we were running
					// before this one has finished
					// and needs to be cleaned up
    
#ifdef USER_PROGRAM
    if (oldThread->space != NULL) {	    // if there is an address space
        oldThread->RestoreUserState();     // to restore, do it.
	oldThread->space->RestoreState();
    }
#endif
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
    cout << "Ready list contents:\n";
    readyList->Apply(ThreadPrint);
}