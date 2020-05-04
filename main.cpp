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

int findOpenOrderIndex(int* shmBUF);
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

void addToMerch() {
    merch[0] += rand() % 20;
    merch[1] += rand() % 20;
    merch[2] += rand() % 20;
    merch[3] += rand() % 20;
    merch[4] += rand() % 20;
    /*
    cout << a[0] <<  endl;
    cout << a[1] <<  endl;
    cout << a[2] <<  endl;
    cout << a[3] <<  endl;
    cout << a[4] <<  endl;
    */
}

void placeOrder(int a, int b, int c, int d, int e) {
    merch[0] -= a;
    merch[1] -= b;
    merch[2] -= c;
    merch[3] -= d;
    merch[4] -= e;
}

void fufilOrder(SEMAPHORE &sem, int* shmBUF) {
    for (int i = 0; i < 10 * MAX_ORDERS; i++){
        sem.P(PUT_ITEM);

        int minIndex = findOpenOrderIndex(shmBUF);
        int customerID = *(shmBUF + minIndex);
        *(shmBUF + minIndex) = 0;
        merch[0] = 0;
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
    //int min = findMinIndex(customerID);
    //int max = findMaxIndex(customerID);

    int orderData[6];
    orderData[0] = customerID;
    orderData[1] = 1;

    for (int i = 0; i < MAX_ORDERS; i++) {
        for (int i = 2; i < 6; i++) {
            orderData[i] = rand() % 2;
        }

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
}

void supplyProcess(SEMAPHORE &sem, int* shmBUF) {
    sem.P(PUT_ITEM);

    int* itemsGotten = shipment_arrival(merch);

    int minIndex = findOpenOrderIndex(shmBUF);
    int customerID = *(shmBUF + minIndex);
    *(shmBUF + minIndex) = 0;
    merch[0] = customerID;

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

int findOpenOrderIndex(int* shmBUF) {
    //shmBUF
}

int findMinIndex(int customerID) {

}

int findMaxIndex(int customerID) {
    
}

int main() {
    //initialize random seed
    srand (time(NULL));

    int shmid;
	int *shmBUF;
	SEMAPHORE sem(2);

    merch  = new int[6];

    printMerch(merch);
    //addToMerch();
    //shipment_arrival(merch);
    printMerch(merch);
    
    int customerID = 1;
    int maxChildren = 11;
    //spawn child
    childPID = 1;
    if (childPID > 0) { //parent
        for (int i = 0; i < maxChildren; i++) {
            customerID++;
            childPID = fork();
            if (childPID == 0 && customerID == 11) { //child
                //Resupply
                cout << "Supplier added more to inventory" << endl;
                supplyProcess(sem, shmBUF);
                break;
            }
            else if (childPID == 0) { //child
                //Customer
                cout << "Made customer" << endl;
                customerProcess(sem, shmBUF, customerID);
                break;
            }
            else {
                cout << "fork() error" << endl;
            }
        }

        fufilOrder(sem, shmBUF);
    }

    
}