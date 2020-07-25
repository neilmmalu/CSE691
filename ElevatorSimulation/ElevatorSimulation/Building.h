#pragma once
#include <iostream>
#include <map>
#include <list>
#include <vector>
#include <mutex>
#include <stdlib.h>
#include <time.h>
#include "Passenger.h"

using namespace std;

class Building {
public:
	map<int, list<Passenger *>> building;
	map<int, list<Passenger*>> waiting;
	int numFloors;
	mutex buildingLock;
	int totalPassengersWaiting;
	int totalPassengersTransported;

	Building(int f) {
		numFloors = f;
		totalPassengersWaiting = 0;
		srand(time(NULL));
		spawnPassengers(1);
	}

	void spawnPassengers(int floor) {

		int numPassengers = 0;
		if (floor == 1) {
			numPassengers = NUM_MAX_PASSENGERS;
		}
		else {
			numPassengers = rand() % 10 + 1;
		}
		unique_lock<mutex> ul(buildingLock);
		for (int i = 1; i <= numPassengers; ++i) {
			building[floor].push_back(new Passenger(floor, rand() % 40 + 1));
			totalPassengersWaiting++;
		}

	}

	list<Passenger* > unloadPassengers(int floor) {
		unique_lock<mutex> ul(buildingLock);
		list<Passenger*> passengers = building[floor];
		building[floor].clear();
		return passengers;
	}

	void loadPassengers(list<Passenger*>& passengers, int floor) {
		unique_lock<mutex> ul(buildingLock);

		for (auto* p : passengers) {
			delete p;
			totalPassengersTransported++;
			totalPassengersWaiting--;
			this_thread::sleep_for(1000ms);
		}
		return;


		if (floor == 1) {
			for (auto* p : passengers) delete p;
			return;
		}

		for (auto* p : passengers) waiting[floor].push_back(p);
	}

	void reloadPassengers(int floor) {
		unique_lock<mutex> ul(buildingLock);
		for (auto* p : waiting[floor]) building[floor].push_back(p);
	}
};