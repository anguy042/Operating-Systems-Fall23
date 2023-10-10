#ifndef ELEVATOR_H
#define ELEVATOR_H

#include "copyright.h"
#include "synch.h"


void Elevator(int numFloors);
void ArrivingGoingFromTo(int atFloor, int toFloor);

typedef struct Person {
    int id;
    int atFloor;
    int toFloor;
} Person;

class ELEVATOR {

public:
    ELEVATOR(int numFloors);
    ~ELEVATOR();
    void hailElevator(Person *p);
    void start();

private:
    int currentFloor;
    int maxFloor;
    Condition **entering;
    Condition **leaving;
    int *personsWaiting;
    int anyPersonWaiting;
    int occupancy;
    int maxOccupancy;
    Lock *elevatorLock;
};


#endif