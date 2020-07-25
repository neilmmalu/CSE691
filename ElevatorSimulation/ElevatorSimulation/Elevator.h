#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <list>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <set>
#include "Passenger.h"
#include "Screen.h"

using namespace std;

class Elevator {
public:
	int elevatorID;
	int currFloor;
	int direction;		//1 = UP, 0 = DOWN
	int capacity;
	int nextDestFloor;
	int currMaxFloor = 0;
	int currMinFloor = 40;
	bool pickingUp;
	bool droppingOff;
	bool isMoving;
	mutex eLock;
	condition_variable eCV;

	list<Passenger *> passengers;
	set<int> destinations;

	Elevator(int id) {
		currFloor = 1;
		direction = 1;
		capacity = 10;
		pickingUp = true;
		elevatorID = id;
		isMoving = false;
	}

	bool sortFuncAsc(Passenger* p1, Passenger* p2) {
		int d1 = p1->destFloor;
		int d2 = p2->destFloor;
		
		if (d1 > currFloor) {
			if (direction == 1) d1 -= currFloor;
			else d1 = (d1 - currMinFloor) + (currFloor - currMinFloor);
		}
		else {
			if (direction == 0) d1 = currFloor - d1;
			else d1 = (currMaxFloor - currFloor) + (currMaxFloor - d1);
		}

		if (d2 > currFloor) {
			if (direction == 1) d2 -= currFloor;
			else d2 = (d2 - currMinFloor) + (currFloor - currMinFloor);
		}
		else {
			if (direction == 0) d2 = currFloor - d2;
			else d2 = (currMaxFloor - currFloor) + (currMaxFloor - d2);
		}

		return d1 < d2;
	}

	list<Passenger*> pickUpPassengers(list<Passenger*>& queue) {
		while (!queue.empty() && passengers.size() <= capacity) {
			Passenger* p = queue.front();
			unique_lock<mutex> el(eLock);
			passengers.push_back(p);
			if (destinations.find(p->destFloor) == destinations.end()) destinations.insert(p->destFloor);
			if (p->destFloor < currMinFloor) currMinFloor = p->destFloor;
			if (p->destFloor > currMaxFloor) currMaxFloor = p->destFloor;
			queue.pop_front();
			el.unlock();
			this_thread::sleep_for(1000ms);
		}
		return queue;
	}

	list<Passenger* > dropPassengers() {
		list<Passenger*> pass;
		unique_lock<mutex> el(eLock);
		auto it = passengers.begin();
		while (it != passengers.end()) {
			if ((*it)->destFloor == currFloor) {
				pass.push_back(*it);
				passengers.erase(it);
				it = passengers.begin();
			}
			if(it != passengers.end()) ++it;
		}
		if(destinations.find(currFloor) != destinations.end()) destinations.erase(destinations.find(currFloor));
		return pass;
	}

	void moveUp() {
		++currFloor;
		//updateElevator(this);
	}

	void moveDown() {
		--currFloor;
	}
	/*
	void run(map<int, list<Passenger*>>& building) {
		pickUpPassengers(building);
		cout << "LMAO" << endl;
		while (true) {
			while(!setDestination()) {

			}

			if (building[nextDestFloor].size() != 0) pickingUp = true;
			else pickingUp = false;

			if (passengers.size() == 0) droppingOff = false;
			else droppingOff = true;

			setDirection();
			
			while (currFloor != nextDestFloor) {
				if (direction == 1) moveUp();
				else moveDown();
				this_thread::sleep_for(1000ms);
			}

			if (droppingOff) dropPassengers(building);

			if (pickingUp) pickUpPassengers(building);
		}

	}
	*/
	void setDestination() { 
		unique_lock<mutex> el(eLock);
		auto it1 = destinations.begin();
		auto it2 = destinations.begin();

		int minDiff = INT_MAX;
		while (it1 != destinations.end()) {
			int destFloor = *it1;
			int diff = 0;
			if (direction == 1) {
				if (currFloor < destFloor) diff = destFloor - currFloor;
				else diff = (currMaxFloor - destFloor) + (currMaxFloor - currFloor);
			}
			else {
				if (currFloor < destFloor) diff = (destFloor - currMinFloor) + (currFloor - currMinFloor);
				else diff = currFloor - destFloor;
			}

			if (diff < minDiff) {
				it2 = it1;
				minDiff = diff;
			}
			if(it1 !=destinations.end()) it1++;
		}


		if (it2 == destinations.end()) {
			nextDestFloor = -1;
			return;
		}

		nextDestFloor = *it2; 
		destinations.erase(it2);
	}

	void setDirection() { 
		unique_lock<mutex> el(eLock);
		direction = (nextDestFloor > currFloor) ? 1 : 0;
	}

};