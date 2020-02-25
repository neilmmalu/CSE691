#include <iostream>
#include <thread>
#include <mutex>
#include <stdlib.h>
#include <condition_variable>
#include <vector>
#include <chrono>

using namespace std;

mutex m1;
condition_variable cv1, cv2;

vector<int> buffer = {0, 0, 0, 0};

vector<int>* generateLoadOrder(){
    vector<int>* load = new vector<int>();
    int total = 0;
    srand(std::chrono::system_clock::now());
    int index = 0;
    int comp = 4;
    while(1){
        load[i] = rand % comp;
        total += load[i];
        comp -= load[i];
        i++;
        if(i == 3){
            load[i] = 4 - total;
            break;
        }
    }

    this_thread::sleep_for(chrono::microseconds(load[0]*50) + chrono::microseconds(load[1]*70) + chrono::microseconds(load[2]*90) + chrono::microseconds(load[3]*110));

    return load;
}

vector<int>* generatePickupOrder(){
    vector<int>* pickup = new vector<int>();
    int total = 0;
    srand(std::chrono::system_clock::now());
    int index = 0;
    int comp = 5;
    while(1){
        pickup[i] = rand % comp;
        total += pickup[i];
        comp -= pickup[i];
        i++;
        if(i == 4){
            pickup[i] = 5 - total;
            break;
        }
    }

    //assign 0 to a random index and increase a random index by that value
    int zeroIndex = rand % 3;
    int replaceVal = pickup[zeroIndex];
    pickup[zeroIndex] = 0;
    int replaceIndex = rand % 3;
    pickup[replaceIndex] += replaceVal;

    this_thread::sleep_for(chrono::microseconds(pickup[0]*80) + chrono::microseconds(pickup[1]*100) + chrono::microseconds(pickup[2]*120) + chrono::microseconds(pickup[3]*140));

    return pickup;
}

void PartWorker(int i){
    while(1){
        vector<int>* loadOrder = generateLoadOrder();
        unique_lock<mutex> lock(m1);

    }
}

void ProductWorker(int i){
    while(1){
        vector<int>* pickupOrder = generatePickupOrder();
        unique_lock<mutex> lock(m1);
    }
}



int main(){

    const int m = 20, n = 16; //m: number of Part Workers
    //n: number of Product Workers
    //m>n

    thread partW[m];
    thread prodW[n];
    for (int i = 0; i < n; i++){
        partW[i] = thread(PartWorker, i);
        prodW[i] = thread(ProductWorker, i);
    }

    for (int i = n; i < m; i++) {
        partW[i] = thread(PartWorker, i);
    }


    /* Join the threads to the main threads */
    for (int i = 0; i < n; i++) {
        partW[i].join();
        prodW[i].join();
    }

    for (int i = n; i < m; i++) {
        partW[i].join();
    }

    cout << "Finish!" << endl;
    return 0;
}