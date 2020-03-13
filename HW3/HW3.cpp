//HW3
//Name: Neil Malu
//SU NetID: nemalu SUID: 635692900

/*
In this implementation, once timeout occurs, check if its possible to COMPLETELY add/remove any parts
If the entire order cannot be fulfilled, then discard the load/pickup order
*/

#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <stdlib.h>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <ctime>

using namespace std;

#define PART_TIMEOUT 3000
#define PRODUCT_TIMEOUT 6000
#define NUM_ITER 5

time_t seed = time(0);
chrono::system_clock::time_point prog_start = chrono::system_clock::now();

vector<vector<int>> partPossibilities = {
    {0,0,0,4},
    {0,0,4,0},{0,0,1,3},{0,0,3,1},{0,0,2,2},
    {0,4,0,0},{0,3,0,1},{0,3,1,0},{0,2,0,2},{0,2,2,0},{0,2,1,1},{0,1,0,3},{0,1,3,0},{0,1,1,2},{0,1,2,1},
    {1,0,0,3},{1,0,3,0},{1,0,1,2},{1,0,2,1},{1,1,0,2},{1,1,2,0},{1,1,1,1},{1,2,0,1},{1,2,1,0},{1,3,0,0},
    {2,0,0,2},{2,0,2,0},{2,0,1,1},{2,1,0,1},{2,1,1,0},{2,2,0,0},
    {3,0,0,1},{3,0,1,0},{3,1,0,0},
    {4,0,0,0}
};

vector<vector<int>> prodPossibilities = {
    {0,1,1,3},{0,1,3,1},{0,2,1,2},{0,2,2,1},{0,3,1,1},
    {1,0,1,3},{1,0,3,1},{1,0,2,2},{1,1,0,3},{1,1,3,0},{1,3,0,1},{1,3,1,0},
    {2,0,1,2},{2,0,2,1},{2,1,0,2},{2,1,2,0},{2,2,0,1},{2,2,1,0},
    {3,0,1,1},{3,1,0,1},{3,1,1,0}
};

vector<int> generateLoadOrder(){
    srand(seed++);
    vector<int> load = partPossibilities[rand() % (partPossibilities.size() - 1)];

    int generateTime = load[0]*50 + load[1]*70 + load[2]*90 + load[3]*110;
    this_thread::sleep_for(chrono::microseconds(generateTime));

    return load;
}

vector<int> generatePickupOrder(){
    srand(seed++);
    vector<int> pickup = prodPossibilities[rand() % (prodPossibilities.size() - 1)];

    return pickup;
}


class Factory{
public:
    mutex m1;
    condition_variable cv1, cv2;
    vector<int> buffer { 0, 0, 0, 0 };

    vector<int> capacity { 6, 5, 4, 3 };

    vector<int> fidelity { 0, 0, 0, 0 };

    int TOTAL_PROD = 0;
    int PART_SLEEPERS = 0;
    int PROD_SLEEPERS = 0;

    bool pushBuffer(vector<int> loadOrder) {
        return (loadOrder[0] + buffer[0] <= 6 && loadOrder[0] != 0) || 
            (loadOrder[1] + buffer[1] <= 5 && loadOrder[1] != 0) ||
            (loadOrder[2] + buffer[2] <= 4 && loadOrder[2] != 0) ||
            (loadOrder[3] + buffer[3] <= 3 && loadOrder[3] != 0);
    }

    bool pushBufferFull(vector<int> loadOrder) {
        return loadOrder[0] + buffer[0] <= 6 && loadOrder[1] + buffer[1] <= 5 && loadOrder[2] + buffer[2] <= 4 && loadOrder[3] + buffer[3] <= 3;
    }

    bool pullBuffer(vector<int> pickupOrder) {
        return (pickupOrder[0] <= buffer[0] && pickupOrder[0] != 0) || 
            (pickupOrder[1] <= buffer[1] && pickupOrder[1] != 0) || 
            (pickupOrder[2] <= buffer[2] && pickupOrder[2] != 0) || 
            (pickupOrder[3] <= buffer[3] && pickupOrder[3] != 0);
    }

    bool pullBufferFull(vector<int> pickupOrder) {
        return pickupOrder[0] <= buffer[0] && pickupOrder[1] <= buffer[1] && pickupOrder[2] <= buffer[2] && pickupOrder[3] <= buffer[3];
    }

    

};

int main(){

    const int m = 30, n = 25; //m: number of Part Workers
    //n: number of Product Workers
    //m>n
    ofstream out("log.txt");
    auto* outbuff = cout.rdbuf();
    cout.rdbuf(out.rdbuf());
    
    thread partW[m];
    thread prodW[n];
    for (int i = 0; i < n; i++){
        partW[i] = thread(PartWorker, i);
        prodW[i] = thread(ProductWorker, i);
    }

    for (int i = n; i < m; i++) {
        partW[i] = thread(PartWorker, i);
    }


    /* Join the threads to the main threads*/ 
    for (int i = 0; i < n; i++) {
        partW[i].join();
        prodW[i].join();
    }

    for (int i = n; i < m; i++) {
        partW[i].join();
    }
    
    cout << "PRODUCTS COMPLETED: " << TOTAL_PROD << endl;
    cout << "Finish!" << endl;
    return 0;
}