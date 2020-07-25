#pragma once
#include <iostream>

int NUM_MAX_PASSENGERS = 10;

class Passenger {
public:
	int sourceFloor;
	int destFloor;

	Passenger(int a, int b) {
		sourceFloor = a;
		destFloor = b;
	}
};