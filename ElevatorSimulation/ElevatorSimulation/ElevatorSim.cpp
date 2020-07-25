#include <iostream>
#include "Screen.h"
#include "Building.h"
#include "Controller.h"
#include "Elevator.h"

using namespace std;

int main() {
	
	Building* b = new Building(40);

	Screen* s = new Screen(b);

	Controller* c = new Controller(b, s);

	CloseHandle(s->hConsole);
	return 0;
}