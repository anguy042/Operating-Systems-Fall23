#include "copyright.h"
#include "system.h"
#include "synch.h"

int nextPersonId = 1;
Lock *personIdLock = new Lock("PersonIdLock");

//Elevator *e = new Elevator(numFloors);


void ElevatorThread(int numFloors){
    printf("Elevator function invoked! Elevator has %d floors.\n", numFloors);


    // Do the following steps A and B forever .... (not needed for this, all persons created at start)

    // A. wait until hailed

    // B. While there are active persons, loop doing the following
    //      1. Signal persons inside elevator to get off
    //      2. persons atFloor to get in 
    //      3. go to next floor 
    //print("Elevator arrives on floor %d.\n", )


}

void Elevator(int numFloors){
    //Every time the elevator reaches a new floor, it needs to do two things.
    //First, it needs to tell all the threads what floor it is at so all the
    //floors can get off if they need to.
    //Secondly, all the waiting floor can get on if they want to.

    //Create Elevator Thread
    Thread *t = new Thread("Elevator");
    t->Fork(ElevatorThread, numFloors);


    


}

//Fork() can only pass one integer as the second argument. 
void PersonThread(int person){

    Person *p = (Person *)person;

    printf("Person %d wants to go from floor %d to %d\n", p->id, p->atFloor, p->toFloor);

    // 1. increment waitng persons atFloor
    // 2. hail Elevator
    // 3. wait for the elevator to arrive atFloor
    // 4. Check if elevator occupancy limit is reached -- wait if it is
    // 5. get into elevator
    print("Person %d got into the elevator. \n", p->id);
    // 6. decrement persons waitng atFloor 
    // 7. increment persons inside elevator
    // 8. wait for elevator to reach the floor
    // 9. get out of elevator
    print("Person %d got out of the elevator. \n", p->id);
    // 10. decrement persons inside elevator
}

int getNextPersonID(){
    int personId = nextPersonId;
    personIdLock->Acquire();
    nextPersonId = nextPersonId + 1;
    personIdLock->Release();
    return personId;
}

void ArrivingGoingFromTo(int atFloor, int toFloor){

   
    //Create Person struct:
    Person *p = new Person;
    p->id = getNextPersonID();
    p->atFloor = atFloor;
    p->toFloor = toFloor;

    //Create Person thread
    Thread *t = new Thread("Person " + p->id);
    t->Fork(PersonThread, (int)p);

}