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

mutex m1;
condition_variable cv1, cv2;

time_t seed = time(0);

vector<int> buffer { 0, 0, 0, 0 };

vector<int> fidelity { 0, 0, 0, 0 };

int TOTAL_PROD = 0;

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

chrono::system_clock::time_point prog_start = chrono::system_clock::now();


bool pushBuffer(vector<int> loadOrder) {
	return (loadOrder[0] + buffer[0] <= 6 && loadOrder[0] != 0) || 
		(loadOrder[1] + buffer[1] <= 5 && loadOrder[1] != 0) ||
		(loadOrder[2] + buffer[2] <= 4 && loadOrder[2] != 0) ||
		(loadOrder[3] + buffer[3] <= 3 && loadOrder[3] != 0);
}

//bool pushBufferFull(vector<int> loadOrder) {
//	return loadOrder[0] + buffer[0] <= 6 && loadOrder[1] + buffer[1] <= 5 && loadOrder[2] + buffer[2] <= 4 && loadOrder[3] + buffer[3] <= 3;
//}

bool pullBuffer(vector<int> pickupOrder) {
	return (pickupOrder[0] <= buffer[0] && pickupOrder[0] != 0) || 
		(pickupOrder[1] <= buffer[1] && pickupOrder[1] != 0) || 
		(pickupOrder[2] <= buffer[2] && pickupOrder[2] != 0) || 
		(pickupOrder[3] <= buffer[3] && pickupOrder[3] != 0);
}

//bool pullBufferFull(vector<int> pickupOrder) {
//	return pickupOrder[0] <= buffer[0] && pickupOrder[1] <= buffer[1] && pickupOrder[2] <= buffer[2] && pickupOrder[3] <= buffer[3];
//}

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

void PartWorker(int i){
	int iteration = 0;
    while(iteration < NUM_ITER){
		int timeOut = PART_TIMEOUT;
        vector<int> loadOrder = generateLoadOrder();
		
		//Generate time done
		
		int moveTime = loadOrder[0]*20 + loadOrder[1]*30 + loadOrder[2]*40 + loadOrder[3]*50;
		this_thread::sleep_for(chrono::microseconds(moveTime));
		//Move time done

		//compete for the lock
        unique_lock<mutex> lock(m1);

		int waits = 0;

		chrono::system_clock::time_point start = chrono::system_clock::now();
	wait:
		if (cv1.wait_until(lock, start + chrono::microseconds(timeOut), [loadOrder] { return pushBuffer(loadOrder); })) {
			//Will enter here if predicate is true
			//If predicate is false, will sleep
			//If can fit into buffer, will try to fit as much as possible
			chrono::system_clock::time_point curr_time = chrono::system_clock::now();
			cout << "Current Time: " << chrono::duration_cast<chrono::microseconds>(curr_time - prog_start).count() << "us" << endl;
			cout << "Part Worker ID: " << i << endl;
			cout << "Iteration: " << iteration << endl;
			if (waits == 0) cout << "Status: New Load Order" << endl;
			else cout << "Status: Wakeup-Notified" << endl;
			cout << "Accumulated Wait Time: " << chrono::duration_cast<chrono::microseconds>(curr_time - start).count() << "us" << endl;
			cout << "Buffer State: (" << buffer[0] << ", " << buffer[1] << ", " << buffer[2] << ", " << buffer[3] << ")" << endl;
			cout << "Load Order: (" << loadOrder[0] << ", " << loadOrder[1] << ", " << loadOrder[2] << ", " << loadOrder[3] << ")" << endl;

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

			cout << "Updated Buffer State: (" << buffer[0] << ", " << buffer[1] << ", " << buffer[2] << ", " << buffer[3] << ")" << endl;
			cout << "Updated Load Order: (" << loadOrder[0] << ", " << loadOrder[1] << ", " << loadOrder[2] << ", " << loadOrder[3] << ")" << endl;
			cout << endl;
			waits++;

			if(loadOrder != fidelity){
				//put it back to sleep
				//Not enough space. Alert prod
				cv2.notify_one();
				goto wait;
			}
			//If loadOrder is empty, just break and move on to next iteration

		}
		else {
			//Timeout condition
			//Check if buffer can fit entire remaining load
			//If not then discard the entire thing

			chrono::system_clock::time_point curr_time = chrono::system_clock::now();
			cout << "Current Time: " << chrono::duration_cast<chrono::microseconds>(curr_time - prog_start).count() << "us" << endl;
			cout << "Part Worker ID: " << i << endl;
			cout << "Iteration: " << iteration << endl;
			cout << "Status: Wakeup-Timeout" << endl;
			cout << "Accumulated Wait Time: " << chrono::duration_cast<chrono::microseconds>(curr_time - start).count() << "us" << endl;
			//cout << "Accumulated Wait Time: " << PART_TIMEOUT << "us" << endl;
			cout << "Buffer State: (" << buffer[0] << ", " << buffer[1] << ", " << buffer[2] << ", " << buffer[3] << ")" << endl;
			cout << "Load Order: (" << loadOrder[0] << ", " << loadOrder[1] << ", " << loadOrder[2] << ", " << loadOrder[3] << ")" << endl;


			if (pushBuffer(loadOrder)) {
				/*buffer[0] += loadOrder[0];
				loadOrder[0] = 0;
				buffer[1] += loadOrder[1];
				loadOrder[1] = 0;
				buffer[2] += loadOrder[2];
				loadOrder[2] = 0;
				buffer[3] += loadOrder[3];
				loadOrder[3] = 0;*/

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
			}
			else {
				//discard
				moveTime = loadOrder[0] * 20 + loadOrder[1] * 30 + loadOrder[2] * 40 + loadOrder[3] * 50;
				this_thread::sleep_for(chrono::microseconds(moveTime));
			}

			cout << "Updated Buffer State: (" << buffer[0] << ", " << buffer[1] << ", " << buffer[2] << ", " << buffer[3] << ")" << endl;
			cout << "Updated Load Order: (" << loadOrder[0] << ", " << loadOrder[1] << ", " << loadOrder[2] << ", " << loadOrder[3] << ")" << endl;
			cout << endl;
			
			cv2.notify_one();
		}
		cv1.notify_one();
		iteration++;
    }
	cv2.notify_one();
}

void ProductWorker(int i){
	int iteration = 0;
    while(iteration < NUM_ITER){
		int timeOut = PRODUCT_TIMEOUT;
        vector<int> pickupOrder = generatePickupOrder();
		unique_lock<mutex> lock(m1);

		int waits = 0;
		chrono::system_clock::time_point start = chrono::system_clock::now();
	wait:

		if (cv2.wait_until(lock, start + chrono::microseconds(timeOut), [pickupOrder] { return pullBuffer(pickupOrder); }))
        {
			//Will enter here if predicate is true
			//If predicate is false, will sleep
			//If can pull from buffer, will try to pull as much as possible
			chrono::system_clock::time_point curr_time = chrono::system_clock::now();
			cout << "Current Time: " << chrono::duration_cast<chrono::microseconds>(curr_time - prog_start).count() << "us" << endl;
			cout << "Product Worker ID: " << i << endl;
			cout << "Iteration: " << iteration << endl;
			if (waits == 0) cout << "Status: New Pickup Order" << endl;
			else cout << "Status: Wakeup-Notified" << endl;
			cout << "Accumulated Wait Time: " << chrono::duration_cast<chrono::microseconds>(curr_time - start).count() << "us" << endl;
			cout << "Buffer State: (" << buffer[0] << ", " << buffer[1] << ", " << buffer[2] << ", " << buffer[3] << ")" << endl;
			cout << "Pickup Order: (" << pickupOrder[0] << ", " << pickupOrder[1] << ", " << pickupOrder[2] << ", " << pickupOrder[3] << ")" << endl;
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

			cout << "Updated Buffer State: (" << buffer[0] << ", " << buffer[1] << ", " << buffer[2] << ", " << buffer[3] << ")" << endl;
			cout << "Updated Product Order: (" << pickupOrder[0] << ", " << pickupOrder[1] << ", " << pickupOrder[2] << ", " << pickupOrder[3] << ")" << endl;
			
			waits++;

			//Check if pickupOrder is complete
			if (pickupOrder == fidelity) {
				//In this case don't put it back to sleep
				//Assemble
				int assembleTime = pickupOrder[0] * 80 + pickupOrder[1] * 100 + pickupOrder[2] * 120 + pickupOrder[3] * 140;
				this_thread::sleep_for(chrono::microseconds(assembleTime));
				TOTAL_PROD++;
				cout << "Total Completed Products: " << TOTAL_PROD << endl;
				cout << endl;
				//Just print and go to next iteration
			}
			else {
				//put it back to sleep because not enough parts
				//alert parts
				cout << "Total Completed Products: " << TOTAL_PROD << endl;
				cout << endl;
				cv1.notify_all();
				goto wait;
			}
		}
		else {
			//Timeout condition
			//Check if entire order can be picked up. If yes then assemble
			//If not then discard the picked out items

			chrono::system_clock::time_point curr_time = chrono::system_clock::now();
			cout << "Current Time: " << chrono::duration_cast<chrono::microseconds>(curr_time - prog_start).count() << "us" << endl;
			cout << "Product Worker ID: " << i << endl;
			cout << "Iteration: " << iteration << endl;
			cout << "Status: Wakeup-Timeout" << endl;
			cout << "Accumulated Wait Time: " << chrono::duration_cast<chrono::microseconds>(curr_time - start).count() << "us" << endl;
			//cout << "Accumulated Wait Time: " << PRODUCT_TIMEOUT << "us" << endl;
			cout << "Buffer State: (" << buffer[0] << ", " << buffer[1] << ", " << buffer[2] << ", " << buffer[3] << ")" << endl;
			cout << "Pickup Order: (" << pickupOrder[0] << ", " << pickupOrder[1] << ", " << pickupOrder[2] << ", " << pickupOrder[3] << ")" << endl;


			if (pullBuffer(pickupOrder)) {
				/*buffer[0] -= pickupOrder[0];
				pickupOrder[0] = 0;
				buffer[1] -= pickupOrder[1];
				pickupOrder[1] = 0;
				buffer[2] -= pickupOrder[2];
				pickupOrder[2] = 0;
				buffer[3] -= pickupOrder[3];
				pickupOrder[3] = 0;*/

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

				if (pickupOrder == fidelity) {
					//In this case don't put it back to sleep
					//Assemble
					int assembleTime = pickupOrder[0] * 80 + pickupOrder[1] * 100 + pickupOrder[2] * 120 + pickupOrder[3] * 140;
					this_thread::sleep_for(chrono::microseconds(assembleTime));
					TOTAL_PROD++;

					//Just print and go to next iteration
				}
			}
			else {
				//Discard
				int moveTime = pickupOrder[0] * 20 + pickupOrder[1] * 30 + pickupOrder[2] * 40 + pickupOrder[3] * 50;
				this_thread::sleep_for(chrono::microseconds(moveTime));
			}

			cout << "Updated Buffer State: (" << buffer[0] << ", " << buffer[1] << ", " << buffer[2] << ", " << buffer[3] << ")" << endl;
			cout << "Updated Product Order: (" << pickupOrder[0] << ", " << pickupOrder[1] << ", " << pickupOrder[2] << ", " << pickupOrder[3] << ")" << endl;
			cout << "Total Completed Products: " << TOTAL_PROD << endl;
			cout << endl;
			cv1.notify_one();
		}
		
		cv2.notify_one();
		iteration++;
    }
	cv1.notify_one();
}



int main(){

    const int m = 20, n = 16; //m: number of Part Workers
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
	
	
    cout << "Finish!" << endl;
    return 0;
}