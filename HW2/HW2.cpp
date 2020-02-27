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

    int generateTime = load[0]*50 + load[1]*70 + load[2]*90 + load[3]*110;

    this_thread::sleep_for(chrono::microseconds(generateTime));

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

    int produceTime = pickup[0]*80 + pickup[1]*100 + pickup[2]*120 + pickup[3]*140;

    this_thread::sleep_for(chrono::microseconds(produceTime));

    return pickup;
}

void PartWorker(int i){
    while(1){
        vector<int>* loadOrder = generateLoadOrder();
        unique_lock<mutex> lock(m1);

        int moveTime = 0;
        this_thread::sleep_for(chrono::microseconds(moveTime));

        if(cv1.wait_for(lock, chrono::microseconds(600))){
            if(loadOrder[0] + buffer[0] <= 6){
                buffer[0] += loadOrder[0];
                moveTime += loadOrder[0]*20;
                loadOrder[0] = 0;
            }

            if(loadOrder[1] + buffer[1] <= 5){
                buffer[1] += loadOrder[1];
                moveTime += loadOrder[1]*30;
                loadOrder[1] = 0;
            }

            if(loadOrder[2] + buffer[2] <= 4){
                buffer[2] += loadOrder[2];
                moveTime += loadOrder[2]*40;
                loadOrder[2] = 0;
            }

            if(loadOrder[3] + buffer[3] <= 3){
                buffer[3] += loadOrder[3];
                moveTime += loadOrder[3]*50;
                loadOrder[3] = 0;
            }

            this_thread::sleep_for(chrono::microseconds(moveTime));

        }
        else{
            delete loadOrder;
            this_thread::sleep_for(chrono::microseconds(moveTime));
        }

        cv2.notify_one();

    }
}

void ProductWorker(int i){
    while(1){
        vector<int>* pickupOrder = generatePickupOrder();
        unique_lock<mutex> lock(m1);

        int moveTime = 0;
        
        //wait for the lock until timeout
        if(cv2.wait_for(lock, chrono::microseconds(1000))){
            if(buffer[0] > pickupOrder[0]){
                buffer[0] -= pickupOrder[0];
                moveTime += pickupOrder[0]*20;
                pickupOrder[0] = 0;
            }

            if(buffer[1] > pickupOrder[1]){
                buffer[1] -= pickupOrder[1];
                moveTime += pickupOrder[1]*30;
                pickupOrder[1] = 0;
            }

            if(buffer[2] > pickupOrder[2]){
                buffer[2] -= pickupOrder[2];
                moveTime += pickupOrder[2]*40;
                pickupOrder[2] = 0;
            }

            if(buffer[3] > pickupOrder[3]){
                buffer[3] -= pickupOrder[3];
                moveTime += pickupOrder[3]*50;
                pickupOrder[3] = 0;
            }

            this_thread::sleep_for(chrono::microseconds(moveTime));

        }
        else{
            delete loadOrder;
            this_thread::sleep_for(chrono::microseconds(moveTime));
        }

        cv2.notify_one();

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