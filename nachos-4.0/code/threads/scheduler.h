// scheduler.h 
//	Data structures for the thread dispatcher and scheduler.
//	Primarily, the list of threads that are ready to run.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "copyright.h"
#include "list.h"
#include "thread.h"

// The following class defines a sleeping thread.
class SleepThread {
  public:
    	SleepThread(Thread* t, int x):
    		sleeper(t), when(x) { };
    	~SleepThread() { }
	bool operator ==(const SleepThread &b) const { return ((this->sleeper == b.sleeper) && (this->when == b.when)); }
	Thread* getSleeper() { return sleeper; }
	int getSleepTime() { return when; }
	void decreaseTime() { --when; }
 
  private:
	Thread* sleeper;
	int when;
};

// The following class defines the scheduler/dispatcher abstraction -- 
// the data structures and operations needed to keep track of which 
// thread is running, and which threads are ready but not running.

enum SchedulerType {
        RR,     // Round Robin
        SJF,
        Priority,
        FCFS
};

class Scheduler {
  public:
	Scheduler(SchedulerType type);	// Initialize list of ready threads and sleep list according to scheduler type
	Scheduler();		// Initialize list of ready threads 
	~Scheduler();				// De-allocate ready list

	void PutToSleep(int sleepTime);  // Put a thread to sleep
	bool IsSleepListEmpty();  // Check if a sleep list is empty
	bool AlarmTicks();  // Periodically check if there exists any thread that its sleeping time is due
	void ReadyToRun(Thread* thread);	
    					// Thread can be dispatched.
	Thread* FindNextToRun();	// Dequeue first thread on the ready 
					// list, if any, and return thread.
	void Run(Thread* nextThread, bool finishing);
	    				// Cause nextThread to start running
	void CheckToBeDestroyed();	// Check if thread that had been
    					// running needs to be deleted
	void Print();			// Print contents of ready list

	SchedulerType getSchedulerType() {return schedulerType;}
    	void setSchedulerType(SchedulerType t) {schedulerType = t;}
    
    // SelfTest for scheduler is implemented in class Thread
    
  private:
	SchedulerType schedulerType;
	SortedList<SleepThread *> *sleepList;  // ordered list of threads that are sleeping,
	List<Thread *> *readyList;	// queue of threads that are ready to run,
					// but not running
	Thread *toBeDestroyed;		// finishing thread to be destroyed
    					// by the next thread that runs
};

#endif // SCHEDULER_H
