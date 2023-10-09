#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "elevator.h"


Lock *personIdLock = new Lock("PersonIdLock");

//We will implement the elevator problem using global variables
//and semaphores to protect the global variables.

//Global Variables and Semaphores:
int currElevatorFloor = 1;
Semaphore *mutexFloor = new Semaphore("Elevator floor mutext semaphore", 1);

int occupancy = 0;
Semaphore *mutexOccupancy = new Semaphore("Semaphore for mutex on elevator capacity", 1);

int nextPersonId = 1;
Semaphore *mutexPersonID = new Semaphore("Semaphore for person ID", 1);

ELEVATOR *e;

void ELEVATOR::start(int numFloors)
{
    printf("Starting Elevator!\n");
    //This will keep track of if the elevator is going up or down.
    //Since the elevator presumably starts on floor 1 and can only  go up,
    //we will initialize it to true. 
    bool up = true; 
    // Do the following steps A and B forever .... (not needed for this, all persons created at start)
    while (1)
    {
        //printf("Entering eternal while loop!\n");
        //I will start by having the elevator always running and then see if we can implement 
        //a way to call it.
        if(up){
            //printf("Elevator is going up!\n");
            //Elevator goes up one floor:
            //      3. Spin for some time
            for (int j = 0; j < 1000000; j++)
            {
                currentThread->Yield();
            }

            //Entry section----------
            //Call P() before incrementing floor number:
            mutexFloor->P();
            //printf("Semaphore signaled! Incrementing floor number!\n");
            currElevatorFloor = currElevatorFloor + 1;
            
            printf("Elevator arrives on floor %d\n", currElevatorFloor);
            
            //before calling V(), check if it is time to go down.
            if(currElevatorFloor == numFloors){
                //printf("Time to go down!\n");
                up = false;
            }

            //Exit section------------
            mutexFloor->V();
            //printf("Semaphore signaled!\n");

            //Yield so people threads can run:
            currentThread->Yield();          

        }else{
            //If elevator is going down, a similar procedure:
    
            //      3. Spin for some time
            for (int j = 0; j < 1000000; j++)
            {
                currentThread->Yield();
            }

            //Entry section----------
            //Call P() before decrementing floor number:
            mutexFloor->P();
            currElevatorFloor = currElevatorFloor - 1;
            
            printf("Elevator arrives on floor %d\n", currElevatorFloor);
            
            //before calling V(), check if it is time to go up.
            if(currElevatorFloor == 1){
                up = true;
            }

            //Exit section------------
            mutexFloor->V();

            //Yield so people threads can run:
            currentThread->Yield();          

        }

    }//while(1)
    
}//Start()

void ElevatorThread(int numFloors)
{
    printf("Elevator function invoked! Elevator has %d floors.\n", numFloors);
    
    
    e = new ELEVATOR(numFloors);
   

    e->start(numFloors);
}

ELEVATOR::ELEVATOR(int numFloors)
{
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
    printf("Starting person thread!\n");

    Person *p = (Person *)person;

    printf("Person %d wants to go from floor %d to %d\n", p->id, p->atFloor, p->toFloor);

    bool waiting = true;
    bool isOnElevator = false;
    bool arrived = false;

    while(waiting){
        //Continuously check what floor the elevator is on.
        if(currElevatorFloor == p->atFloor){
            //Check if elevator is at capacity or not:
            if(occupancy <= 5){
                //get on the elevator.
                printf("Person %d got into the elevator.\n", p->id);

                //increment occupancy:
                mutexOccupancy->P();
                occupancy = occupancy + 1;
                mutexOccupancy->V();

                waiting = false;
                isOnElevator = true;

            }
           
            
        }

        currentThread->Yield();
    }//on elevator

    while(isOnElevator){
        //Continuously check what floor the elevator is on.
        if(currElevatorFloor == p->toFloor){
            //no need to check occupancy before getting off:
             printf("Person %d got out of the elevator.\n", p->id);

                //decrement occupancy:
                mutexOccupancy->P();
                occupancy = occupancy - 1;
                mutexOccupancy->V();

                isOnElevator = false;
                arrived = true; //I didn't use this but I like having it.
        }

        currentThread->Yield();

    }


}

int getNextPersonID()
{
    int personId = nextPersonId;
    
    mutexPersonID->P();
    nextPersonId = nextPersonId + 1;
    mutexPersonID->V();
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