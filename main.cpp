/*
Saurav Chhapawala

*/

#include "shipment.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <memory.h>
#include "semaphore.h"
#include <random>
#include <ctime>
#include <cstdlib>
#include <sys/wait.h>
using namespace std;

/*
merchandise
0 - Customer ID
1 - overgrips
2 - racket strings
3 - tennis shoes
4 - tennis balls
5 - tennis accessories
*/
int* merch;
long childPID;
int MAX_ORDERS = 100;
int orderIndex;
enum {PUT_ITEM, TAKE_ITEM, GRIPS, STRINGS, SHOES, BALLS, ACCESSORIES};

void printMerch(int a[]) {
    cout << "INVENTORY" << endl;
    cout << merch[0] << " overgrips" << endl;
    cout << merch[1] << " racket strings" << endl;
    cout << merch[2] << " tennis shoes" << endl;
    cout << merch[3] << " tennis balls" << endl;
    cout << merch[4] << " tennis accessories" << endl;
}

//index will have elements that correspond to the customer id and order elements
int findOpenOrderIndex(int* shmBUF) {
    //todo
}

int findMinIndex(int customerID) {
    //todo
}

int findMaxIndex(int customerID) {
    //todo
}

void fufilOrder(SEMAPHORE &sem, int* shmBUF) {
    for (int i = 0; i < 10 * MAX_ORDERS; i++){
        //Wait for an order
        sem.P(PUT_ITEM);

        //Index to get 
        //see which order is ready to go
        int minIndex = findOpenOrderIndex(shmBUF);
        //Look through to see if we already have this order
        int customerID = *(shmBUF + minIndex);
        //Went through, reset to 0
        //resets customer id to 0 so order is good
        *(shmBUF + minIndex) = 0;
        merch[0] = 0;
        /*
        0 - Customer ID
        1 - overgrips
        2 - racket strings
        3 - tennis shoes
        4 - tennis balls
        5 - tennis accessories
        */
       //make the purchase and decrement inventory
        if (*(shmBUF + minIndex + 1) == 1) {
            *(shmBUF + 1) -= 1;
            merch[1] -= 1;
        }
        if (*(shmBUF + minIndex + 1) == 1) {
            *(shmBUF + 2) -= 1;
            merch[2] -= 1;
        }
        if (*(shmBUF + minIndex + 1) == 1) {
            *(shmBUF + 3) -= 1;
            merch[3] -= 1;
        }
        if (*(shmBUF + minIndex + 1) == 1) {
            *(shmBUF + 4) -= 1;
            merch[4] -= 1;
        }
        if (*(shmBUF + minIndex + 1) == 1) {
            *(shmBUF + 5) -= 1;
            merch[5] -= 1;
        }

        //Print what the customer ordered
        cout << "ORDER" << endl;
        cout << "Customer ID: " << customerID << endl;
        cout << "Over Grips: " << *(shmBUF + minIndex + 1) << endl;
        cout << "Racket Strings: " << *(shmBUF + minIndex + 2) << endl;
        cout << "Tennis Shoes: " << *(shmBUF + minIndex + 3) << endl;
        cout << "Tennis Balls: " << *(shmBUF + minIndex + 4) << endl;
        cout << "Tennis Accessories: " << *(shmBUF + minIndex + 5) << endl;

        sem.V(TAKE_ITEM);
    }
}

void customerProcess(SEMAPHORE &sem, int* shmBUF, int customerID) {
    //Get the index
    int min = findMinIndex(customerID);
    int max = findMaxIndex(customerID);

    //Customers order, follows same numbering scheme
    int orderData[6];
    orderData[0] = customerID;

    //Every order has a grip
    orderData[1] = 1;

    for (int i = 0; i < MAX_ORDERS; i++) {
        //randomly generate order
        for (int i = 2; i < 6; i++) {
            orderData[i] = rand() % 2;
        }

        //If we recieved an order for it wait and get it
        //this is always one
        sem.P(GRIPS);

        if (orderData[2] == 1)
            sem.P(STRINGS);
        if (orderData[3] == 1)
            sem.P(SHOES);
        if (orderData[4] == 1)
            sem.P(BALLS);
        if (orderData[5] == 1)
            sem.P(ACCESSORIES);
    }
    sem.V(GRIPS);
    sem.V(STRINGS);
    sem.V(SHOES);
    sem.V(BALLS);
    sem.V(ACCESSORIES);
}

void supplyProcess(SEMAPHORE &sem, int* shmBUF) {
    sem.P(PUT_ITEM);

    //Get the items from provided method
    int* itemsGotten = shipment_arrival(merch);

    //get index
    int minIndex = findOpenOrderIndex(shmBUF);
    //customer id is first in scheme
    int customerID = *(shmBUF + minIndex);
    //set to 11 so we know it is restocker
    *(shmBUF + minIndex) = 11;
    merch[0] = customerID;

    //Add items gotten into inventory
    *(shmBUF + 1) += itemsGotten[0];
    merch[1] += itemsGotten[0];
    *(shmBUF + 2) += itemsGotten[1];
    merch[2] += itemsGotten[1];
    *(shmBUF + 3) += itemsGotten[2];
    merch[3] += itemsGotten[2];
    *(shmBUF + 4) += itemsGotten[3];
    merch[4] += itemsGotten[3];
    *(shmBUF + 5) += itemsGotten[4];
    merch[5] += itemsGotten[4];

    sem.V(TAKE_ITEM);
}

int main() {
    //initialize random seed
    srand (time(NULL));

    int shmid;
	int *shmBUF;
	SEMAPHORE sem(2);

    merch  = new int[6];
    printMerch(merch);
    
    int customerID = 1;
    
    //spawn child
    //set to 1 to start it off
    childPID = 1;
    if (childPID > 0) { //parent
        for (int i = 0; i < 11; i++) {
            customerID++;
            childPID = fork();
            //create 11 children
            if (childPID == 0 && customerID == 11) { //child
                //11th process, Resupply
                cout << "Supplier added more to inventory" << endl;
                supplyProcess(sem, shmBUF);
                break;
            }
            else if (childPID == 0) { //child
                //Customer
                cout << "Made customer and sent order" << endl;
                customerProcess(sem, shmBUF, customerID);
                break;
            }
            else {
                cout << "fork() error" << endl;
            }
            printMerch(merch);
        }
        fufilOrder(sem, shmBUF);
        cout << "Fufiled order" << endl;
    }
}