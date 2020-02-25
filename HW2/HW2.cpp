#include <iostream>
#include <thread>
#include <mutex>
#include <stdlib.h>
#include <vector>
#include <chrono>

using namespace std;

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

    return load;
}

vector<int>* generatePickupOrder(){
    vector<int>* load = new vector<int>();
    int total = 0;
    srand(std::chrono::system_clock::now());
    int index = 0;
    int comp = 5;
    bool checkZero = false;
    while(1){
        load[i] = rand % comp;
        if(load[i] == 0) checkZero = true;
        total += load[i];
        comp -= load[i];
        i++;
        if(i == 4){
            load[i] = 5 - total;
            break;
        }
    }

    return load;
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