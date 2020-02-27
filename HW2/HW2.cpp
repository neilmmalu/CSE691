#include <iostream>
#include <thread>
#include <mutex>
#include <stdlib.h>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <ctime>

using namespace std;

#define PART_TIMEOUT 3000;
#define PRODUCT_TIMEOUT 6000;

mutex m1;
condition_variable cv1, cv2;

time_t seed = time(0);

vector<int> buffer { 0, 0, 0, 0 };

vector<int> fidelity { 0, 0, 0, 0 };

bool pushBuffer(vector<int> loadOrder) {
	return loadOrder[0] + buffer[0] <= 6 || loadOrder[1] + buffer[1] <= 5 || loadOrder[2] + buffer[2] <= 4 || loadOrder[3] + buffer[3] <= 3;
}

bool pushBufferFull(vector<int> loadOrder) {
	return loadOrder[0] + buffer[0] <= 6 && loadOrder[1] + buffer[1] <= 5 && loadOrder[2] + buffer[2] <= 4 && loadOrder[3] + buffer[3] <= 3;
}

bool pullBuffer(vector<int> pickupOrder) {
	return pickupOrder[0] <= buffer[0] || pickupOrder[1] <= buffer[1] || pickupOrder[2] <= buffer[2] || pickupOrder[3] <= buffer[3];
}

bool pullBufferFull(vector<int> pickupOrder) {
	return pickupOrder[0] <= buffer[0] && pickupOrder[1] <= buffer[1] && pickupOrder[2] <= buffer[2] && pickupOrder[3] <= buffer[3];
}

vector<int> generateLoadOrder(){
	vector<int> load{0, 0, 0, 0};
    int total = 0;
	srand(seed++);
    int i = 0;
    int comp = 4;
    while(1){
        load[i] = rand() % comp;
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

vector<int> generatePickupOrder(){
	vector<int> pickup{1, 1, 1, 1};
    int total = 0;
	srand(seed++);
    int i = 0;
    int comp = 3;
    while(1){
        pickup[i] = rand() % comp;
        total += pickup[i];
        comp -= pickup[i];
        i++;
        if(i == 3){
            pickup[i] = 5 - total;
            break;
        }
    }

    //assign 0 to a random index and increase a random index by that value
    int zeroIndex = rand() % 3;
    int replaceVal = pickup[zeroIndex];
    pickup[zeroIndex] = 0;
    int replaceIndex = rand() % 3;
    pickup[replaceIndex] += replaceVal;

    return pickup;
}

void PartWorker(int i){
    while(1){
		int timeOut = PART_TIMEOUT;
        vector<int> loadOrder = generateLoadOrder();
		
		//Generate time done
		
		int moveTime = loadOrder[0]*20 + loadOrder[1]*30 + loadOrder[2]*40 + loadOrder[3]*50;
		this_thread::sleep_for(chrono::microseconds(moveTime));
		//Move time done

		//compete for the lock
        unique_lock<mutex> lock(m1);

		chrono::system_clock::time_point start = chrono::system_clock::now();
		if (cv1.wait_until(lock, start + chrono::microseconds(timeOut), [loadOrder] { return pushBuffer(loadOrder); })) {
			//Will enter here if predicate is true
			//If predicate is false, will sleep
			//If can fit into buffer, will try to fit as much as possible

			if (loadOrder[0] + buffer[0] <= 6) {
				buffer[0] += loadOrder[0];
				loadOrder[0] = 0;
			}

			if (loadOrder[1] + buffer[1] <= 5) {
				buffer[1] += loadOrder[1];
				loadOrder[1] = 0;
			}

			if (loadOrder[2] + buffer[2] <= 4) {
				buffer[2] += loadOrder[2];
				loadOrder[2] = 0;
			}

			if (loadOrder[3] + buffer[3] <= 3) {
				buffer[3] += loadOrder[3];
				loadOrder[3] = 0;
			}

			//check if load order is empty
			if (loadOrder == fidelity) {
				//In this case don't put it back to sleep
				//Just print and go to next iteration

			}
			else {
				//put it back to sleep
			}

		}
		else {
			//Timeout condition
			//Check if buffer can fit entire remaining load
			//If not then discard the entire thing
			if (pushBufferFull(loadOrder)) {
				buffer[0] += loadOrder[0];
				loadOrder[0] = 0;
				buffer[1] += loadOrder[1];
				loadOrder[1] = 0;
				buffer[2] += loadOrder[2];
				loadOrder[2] = 0;
				buffer[3] += loadOrder[3];
				loadOrder[3] = 0;
			}
			else {
				//discard
				moveTime = loadOrder[0] * 20 + loadOrder[1] * 30 + loadOrder[2] * 40 + loadOrder[3] * 50;
				this_thread::sleep_for(chrono::microseconds(moveTime));
			}
			
			//PRINT

			cv1.notify_all();
			cv2.notify_one();
		}
		cv2.notify_one();
		
    }
}

void ProductWorker(int i){
    while(1){
		int timeOut = PRODUCT_TIMEOUT;
        vector<int> pickupOrder = generatePickupOrder();
		unique_lock<mutex> lock(m1);

		chrono::system_clock::time_point start = chrono::system_clock::now();
		if (cv2.wait_until(lock, start + chrono::microseconds(timeOut), [pickupOrder] { return pullBuffer(pickupOrder); })) {
			//Will enter here if predicate is true
			//If predicate is false, will sleep
			//If can pull from buffer, will try to pull as much as possible
			if (buffer[0] >= pickupOrder[0]) {
				buffer[0] -= pickupOrder[0];
				pickupOrder[0] = 0;
			}

			if (buffer[1] >= pickupOrder[1]) {
				buffer[1] -= pickupOrder[1];
				pickupOrder[1] = 0;
			}

			if (buffer[2] >= pickupOrder[2]) {
				buffer[2] -= pickupOrder[2];
				pickupOrder[2] = 0;
			}

			if (buffer[3] >= pickupOrder[3]) {
				buffer[3] -= pickupOrder[3];
				pickupOrder[3] = 0;
			}

			//Check if pickupOrder is complete
			if (pickupOrder == fidelity) {
				//In this case don't put it back to sleep
				//Assemble
				int assembleTime = pickupOrder[0] * 80 + pickupOrder[1] * 100 + pickupOrder[2] * 120 + pickupOrder[3] * 140;
				this_thread::sleep_for(chrono::microseconds(assembleTime));

				//Just print and go to next iteration
			}
			else {
				//put it back to sleep
			}
		}
		else {
			//Timeout condition
			//Check if entire order can be picked up. If yes then assemble
			//If not then discard the picked out items

			if (pullBufferFull(pickupOrder)) {
				buffer[0] -= pickupOrder[0];
				pickupOrder[0] = 0;
				buffer[1] -= pickupOrder[1];
				pickupOrder[1] = 0;
				buffer[2] -= pickupOrder[2];
				pickupOrder[2] = 0;
				buffer[3] -= pickupOrder[3];
				pickupOrder[3] = 0;
			}
			else {
				int moveTime = pickupOrder[0] * 20 + pickupOrder[1] * 30 + pickupOrder[2] * 40 + pickupOrder[3] * 50;
				this_thread::sleep_for(chrono::microseconds(moveTime));
			}

			//PRINT STUFF

			cv1.notify_one();
			cv2.notify_all();
		}
		
		cv1.notify_one();
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


    /* Join the threads to the main threads*/ 
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