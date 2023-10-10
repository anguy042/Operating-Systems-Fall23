#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "elevator.h"

int nextPersonId = 1;
Lock *personIdLock = new Lock("PersonIdLock");

ELEVATOR *e;

void ELEVATOR::start()
{
    // Do the following steps A and B forever .... (not needed for this, all persons created at start)
    while (1)
    {

        // A. wait until hailed
        for (int j = 0; j < 1000000; j++)
        {
            currentThread->Yield();
        }
        // B. While there are active persons, loop doing the following
        while (anyPersonWaiting > 0)
        {
            //assuming there must be atleast 1 person in the elevator

            //      0. Acquire elevatorLock
            elevatorLock->Acquire();

            //      1. Signal persons inside elevator to get off (leaving->broadcast(elevatorLock))
            leaving[currentFloor]->Broadcast(elevatorLock);
            //at the currentFloor a signal will be broadcasted
            printf("Elevator arrives on floor %d and broadcast.\n", currentFloor);

            //      2. persons atFloor to get in , one at time, checking ocupancyLimit each time

            //      2.5 release elevatorLock
            elevatorLock->Release();

            //      3. Spin for some time
            for (int j = 0; j < 1000000; j++)
            {
                currentThread->Yield();
            }
            //      4. go to next floor (update corrent floor)
            currentFloor = (currentFloor % (maxFloor+1)) + 1;

            //  print("Elevator arrives on floor %d.\n", )
            printf("Elevator leaving floor %d.\n", currentFloor);
        }

    }
}

void ElevatorThread(int numFloors)
{
    printf("Elevator function invoked! Elevator has %d floors.\n", numFloors);

    e = new ELEVATOR(numFloors);

    e->start();
}

ELEVATOR::ELEVATOR(int numFloors)
{
    maxFloor = numFloors;
    currentFloor = 1;
    entering = new Condition *[numFloors];
    // initialize entering

    for (int i = 0; i < numFloors; i++)
    {
        entering[i] = new Condition("Entering " + i);
    }

    personsWaiting = new int[numFloors];
    elevatorLock = new Lock("ElevatorLock");

    // initalize leaving
    leaving = new Condition *[numFloors];

}

void Elevator(int numFloors)
{
    //Every time the elevator reaches a new floor, it needs to do two things.
    //First, it needs to tell all the threads what floor it is at so all the
    //floors can get off if they need to.
    //Secondly, all the waiting floor can get on if they want to.

    //Create Elevator Thread
    Thread *t = new Thread("Elevator");
    t->Fork(ElevatorThread, numFloors);
}

void ELEVATOR::hailElevator(Person *p)
{
    // 1. increment waitng persons atFloor
    personsWaiting[p->atFloor]++;
    anyPersonWaiting++;

    // 2. hail Elevator
    // 2.5 Acquire elevatorLock;
    elevatorLock->Acquire();

    // 3. wait for the elevator to arrive atFloor [entering[p->atFloor]->wait(elevatorLock)]
    while (currentFloor != p->atFloor)
    {
        entering[p->atFloor]->Wait(elevatorLock);
    }
    //while were on the currentFloor it will note what floor the person is entering at

    // 5. get into elevator
    printf("Person %d got into the elevator. \n", p->id);

    // 6. decrement persons waitng atFloor [personsWaiting[atFloor]++]
    personsWaiting[p->atFloor]--;
    anyPersonWaiting--;
    //subtracting the # of people waiting

    // 7. increment persons inside elevator [ocupancy++]
    occupancy++;
    //adding occupants

    // 8. wait for elevator to reach the floor [leaving[p->toFloor]->wait(elevatorLock)]
    while (currentFloor != p->toFloor)
    {
        leaving[p->toFloor]->Wait(elevatorLock);
    }

    // 9. get out of elevator
    printf("Person %d got out of the elevator. \n", p->id);

    // 10. decrement persons inside elevator
    occupancy--;
    //subtracting occupants

    // 11. Release elevatorLock;
    elevatorLock->Release();
}

//Fork() can only pass one integer as the second argument.
void PersonThread(int person)
{

    Person *p = (Person *)person;

    printf("Person %d wants to go from floor %d to %d\n", p->id, p->atFloor, p->toFloor);

    e->hailElevator(p);
}

int getNextPersonID()
{
    int personId = nextPersonId;
    personIdLock->Acquire();
    nextPersonId = nextPersonId + 1;
    personIdLock->Release();
    return personId;
}

void ArrivingGoingFromTo(int atFloor, int toFloor)
{

    //Create Person struct:
    Person *p = new Person;
    p->id = getNextPersonID();
    p->atFloor = atFloor;
    p->toFloor = toFloor;

    //Create Person thread
    Thread *t = new Thread("Person " + p->id);
    t->Fork(PersonThread, (int)p);
}