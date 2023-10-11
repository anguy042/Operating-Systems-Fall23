// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(const char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	    scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(const char* debugName) {
    name = debugName;
    free = true;
    queue = new List;
}
Lock::~Lock() {
    delete queue;
}
void Lock::Acquire() {

    // Disable interrupts
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

    // check if lock is free
    // else lock is not free -- add self to queue 
    while (!free) { 			// lock not available
        queue->Append((void *)currentThread);	// so go to sleep
        currentThread->Sleep();
    } 

    // if yes, make the lock not free anymore
    free = false;

    // enable interrupts
    (void) interrupt->SetLevel(oldLevel);
}
void Lock::Release() {

    // disable interrupts
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

    // check if thread has lock ... isHeldByCurrentThread
    // if not, do nothing
    while (!isHeldByCurrentThread()) { 			// lock not available
        queue->Append((void *)currentThread);	// so go to sleep
        currentThread->Sleep();
    } 
     
    // if yes, release the lock and wakeup 1 of the waiting threads in queue
    free = true;

    Thread *thread;
    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	    scheduler->ReadyToRun(thread);

    // enable interrupts
    (void) interrupt->SetLevel(oldLevel);
}

bool Lock::isHeldByCurrentThread() {
    return !free;
}

Condition::Condition(const char* debugName) { 
    name = debugName; // init
    queue = new List;
}
Condition::~Condition() { 
    delete queue;
}
void Condition::Wait(Lock* conditionLock) { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    // check if calling thread holds the lock
    while (!conditionLock->isHeldByCurrentThread()) { 			// lock not available
        queue->Append((void *)currentThread);	// so go to sleep
        currentThread->Sleep();
    } 
    // release the lock
    conditionLock->Release();

    // put self in the queue of waiting threads
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();

    // re-acquire the lock
    conditionLock->Acquire();
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}
void Condition::Signal(Lock* conditionLock) { 

    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    // check if calling thread holds the lock
    while (!conditionLock->isHeldByCurrentThread()) { 			// lock not available
        queue->Append((void *)currentThread);	// so go to sleep
        currentThread->Sleep();
    } 

    // dequeue one of the threads in the queue
    Thread *thread;
    thread = (Thread *)queue->Remove();

    // if thread exists, wake it up.
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	    scheduler->ReadyToRun(thread);
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}
void Condition::Broadcast(Lock* conditionLock) { 

    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    while (!conditionLock->isHeldByCurrentThread()) { 			// lock not available
        queue->Append((void *)currentThread);	// so go to sleep
        currentThread->Sleep();
    } 
    // Dequeue all threads in the queue one-by-one
    Thread *thread;
    thread = (Thread *)queue->Remove();
    while (thread){
        // Wakeup each thread
        scheduler->ReadyToRun(thread);
        thread = (Thread *)queue->Remove();
    }

    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}
