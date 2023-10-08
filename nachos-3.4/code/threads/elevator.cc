#include "copyright.h"
#include "system.h"

int nextPersonId = 1;

typedef struct Person {
    int id;
    int atFloor;
    int toFloor;
} Person;


void ElevatorThread(int numFloors){
    printf("Elevator function invoked! Elevator has %d floors.\n", numFloors);
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

}

int getNextPersonID(){
    return nextPersonId++;
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