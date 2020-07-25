#pragma once
#include <iostream>
#include <thread>
#include <mutex>
#include "Elevator.h"
#include "Building.h"
#include "Screen.h"

using namespace std;

class Controller {
public:
	int numElevators = 5;
	Screen* screen;
	Building* building;
	vector<Elevator *> elevators;
	mutex controllerLock;

	Controller(Building* b, Screen* s) {
		screen = s;
		building = b;

		Elevator e1(0);
		Elevator e2(1);
		Elevator e3(2);
		Elevator e4(3);
		Elevator e5(4);
		elevators.push_back(&e1);
		elevators.push_back(&e2);
		elevators.push_back(&e3);
		elevators.push_back(&e4);
		elevators.push_back(&e5);
		thread t1{ &Controller::run, this, ref(e1) };
		thread t2{ &Controller::run, this, ref(e2) };
		thread t3{ &Controller::run, this, ref(e3) };
		thread t4{ &Controller::run, this, ref(e4) };
		thread t5{ &Controller::run, this, ref(e5) };

		thread t6{ &Controller::checkBuildingStatus, this };
		thread t7{ &Controller::infiniteSpawn, this };
		thread t8{ &Controller::clearDaemonFloors, this };
		t1.join();
		t2.join();
		t3.join();
		t4.join();
		t5.join();
		t6.join();
		t7.join();
		t8.join();
	}
	
	int findClosestElevator(int destFloor) {
		
		int minDiff = INT_MAX;
		int eIndex;
		for (int i = 0; i < 5; ++i) {

			if (elevators[i]->passengers.size() + building->building[destFloor].size() > 10) continue;

			int diff = 0;
			int currFloor = elevators[i]->currFloor;
			int elevatorMinFloor = elevators[i]->currMinFloor;
			int elevatorMaxFloor = elevators[i]->currMaxFloor;
			if (destFloor > currFloor) {
				if (elevators[i]->direction == 1) diff = destFloor - currFloor;
				else diff = (destFloor - elevatorMinFloor) + (currFloor - elevatorMinFloor);
			}
			else {
				if (elevators[i]->direction == 0) diff = currFloor - destFloor;
				else diff = (elevatorMaxFloor - destFloor) + (elevatorMaxFloor - currFloor);
			}

			if (diff < minDiff) {
				minDiff = diff;
				eIndex = i;
			}
		}

		if (eIndex < 5 && elevators[eIndex]->isMoving && elevators[eIndex + 1]->currFloor == 1) eIndex++;

		return eIndex;
	}
	
	/*void assignElevator(int eIndex, int floor) {
		Elevator* e = elevators[eIndex];

		elevators[eIndex]->destinations.push_back(floor);
	}*/

	void checkBuildingStatus() {
		set<int> floorRequests;
		while (true) {
			this_thread::sleep_for(500ms);
			unique_lock<mutex> bl(controllerLock);
			
			for (int i = 1; i <= building->numFloors; ++i) {

				if (building->building[i].empty() && floorRequests.find(i) != floorRequests.end())
					floorRequests.erase(floorRequests.find(i));

				if (building->building[i].size() != 0 && floorRequests.find(i) == floorRequests.end()) {
					screen->updatePassengerScreen();
					int eIndex = findClosestElevator(i);
					elevators[eIndex]->destinations.insert(i);
					elevators[eIndex]->setDestination();
					elevators[eIndex]->setDirection();
					elevators[eIndex]->pickingUp = true;
					elevators[eIndex]->eCV.notify_one();
					floorRequests.insert(i);
				}
			}
			bl.unlock();
			screen->updateTime();
		}
	}

	void clearDaemonFloors() {
		while (true) {
			this_thread::sleep_for(30000ms);
			unique_lock<mutex> bl(controllerLock);

			for (int i = 1; i <= building->numFloors; ++i) {
				if (building->building[i].size() != 0) {
					bool check = false;
					for (int j = 0; j < 5; ++j) {
						if (elevators[j]->destinations.find(i) == elevators[j]->destinations.end())
							check = true;
							
					}
					if(check) elevators[4]->destinations.insert(i);
				}
			}
		}
	}

	void infiniteSpawn() {
		while (true) {
			srand(time(NULL));
			this_thread::sleep_for(10000ms);
			building->spawnPassengers(rand() % 40 + 1);
			screen->updatePassengerScreen();
		}
	}

	void run(Elevator& e) {
		
		//critical part begins
		unique_lock<mutex> cl(controllerLock);
		list<Passenger*> queue = building->unloadPassengers(e.currFloor);

		queue = e.pickUpPassengers(queue);
		e.droppingOff = true;
		//e.destinations.push_back(queue.front()->destFloor);
		if (!queue.empty()) building->loadPassengers(queue, e.currFloor);
		screen->updatePassengerScreen();
		screen->updateElevatorStatus(&e);
		cl.unlock();
		//critical part ends
		while (true) {

			if (e.passengers.size() > 8) {
				dropAllPassengers(e);
			}


			e.setDestination();
			cl.lock();
			while (e.nextDestFloor == -1) {

				if (e.passengers.size() > 0) dropAllPassengers(e);

				//add condition variable
				e.eCV.wait(cl);
			}
			cl.unlock();
			e.setDirection();
			screen->updateElevatorStatus(&e);
			if (building->building[e.nextDestFloor].empty()) e.pickingUp = false;
			else e.pickingUp = true;

			while (e.currFloor != e.nextDestFloor) {
				screen->updateElevatorFloor(&e);
				if (e.direction == 1) e.moveUp();
				else e.moveDown();
				screen->updateElevatorStatus(&e);
				e.isMoving = true;
				this_thread::sleep_for(500ms);
			}
			e.isMoving = false;
			cl.lock();

			list<Passenger*> queue = e.dropPassengers();
			if (!queue.empty()) building->loadPassengers(queue, e.currFloor);


			screen->updatePassengerScreen();
			screen->updateElevatorStatus(&e);
			if (e.pickingUp) {
				list<Passenger*> queue = building->unloadPassengers(e.currFloor);

				queue = e.pickUpPassengers(queue);

				if (!queue.empty()) building->loadPassengers(queue, e.currFloor);
			}
			cl.unlock();
			screen->updatePassengerScreen();
			screen->updateElevatorStatus(&e);
		}
		
	}

	void dropAllPassengers(Elevator& e) {
		while (e.passengers.empty()) {
			unique_lock<mutex> cl(controllerLock);
			e.nextDestFloor = e.passengers.front()->destFloor;
			while (e.currFloor != e.nextDestFloor) {
				screen->updateElevatorFloor(&e);
				if (e.direction == 1) e.moveUp();
				else e.moveDown();
				screen->updateElevatorStatus(&e);
				e.isMoving = true;
				this_thread::sleep_for(500ms);
			}
			e.isMoving = false;
			list<Passenger*> queue = e.dropPassengers();
			if (!queue.empty()) building->loadPassengers(queue, e.currFloor);
			screen->updatePassengerScreen();
			screen->updateElevatorStatus(&e);
		}
	}

	

};
