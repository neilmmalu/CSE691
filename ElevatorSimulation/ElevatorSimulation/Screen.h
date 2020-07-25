#pragma once
#include <Windows.h>
#include <thread>
#include "Building.h"
#include "Elevator.h"

using namespace std;

chrono::system_clock::time_point start = chrono::system_clock::now();

class Screen {
public:
	int nScreenWidth = 120;			// Console Screen Size X (columns)
	int nScreenHeight = 60;			// Console Screen Size Y (rows)

	int buildingWidth = 26;
	int buildingHeight = 40;

	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	unsigned char* building = new unsigned char[buildingWidth * buildingHeight]; // Create play field buffer

	DWORD dwBytesWritten = 0;
	Building* buildingScreen;
	mutex screenLock;

	Screen(Building* b) {
		buildingScreen = b;
		for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';

		_COORD coord;
		coord.X = nScreenWidth;
		coord.Y = nScreenHeight;

		_SMALL_RECT Rect;
		Rect.Top = 0;
		Rect.Left = 0;
		Rect.Bottom = nScreenHeight - 1;
		Rect.Right = nScreenWidth - 1;


		SetConsoleScreenBufferSize(hConsole, coord);            // Set Buffer Size 
		SetConsoleWindowInfo(hConsole, TRUE, &Rect);            // Set Window Size 
		SetConsoleActiveScreenBuffer(hConsole);

		int elNum = 1;
		for (int x = 0; x < buildingWidth; x++) {
			for (int y = -1; y <= buildingHeight; y++) {
				if (x == 0 && y >= 0 && y != buildingHeight)
					swprintf_s(&screen[(y + 2) * nScreenWidth + (x + 2)], 3, L"%02d", buildingHeight - y);

				if (x == 3 || ((y == -1 || y == buildingHeight) && x > 3 && x < buildingWidth - 4) || x == buildingWidth - 4)
					swprintf_s(&screen[(y + 2) * nScreenWidth + (x + 2)], 2, L"#");

				if (y >= 0 && y < buildingHeight && x == buildingWidth - 2)
					swprintf_s(&screen[(y + 2) * nScreenWidth + (x + 2)], 3, L"%02d", b->building[buildingHeight - y].size());

				//screen[(y + 2) * nScreenWidth + (x + 4)] = L" ABCDEFG=#"[building[y * buildingWidth + x]];
				if (y == buildingHeight - 1 && (x == 6 || x == 9 || x == 12 || x == 15 || x == 18)) {
					swprintf_s(&screen[(y + 2) * nScreenWidth + (x + 2)], 3, L"E%d", elNum++);
				}
			}
		}

		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
		//this_thread::sleep_for(10000ms);
	}

	void updatePassengerScreen() {
		unique_lock<mutex> ul(screenLock);
		int x = buildingWidth - 2;
		for (int y = -1; y <= buildingHeight; y++) {
			if (y >= 0 && y < buildingHeight && x == buildingWidth - 2)
				swprintf_s(&screen[(y + 2) * nScreenWidth + (x + 2)], 3, L"%02d", buildingScreen->building[buildingHeight - y].size());
		}

		x = buildingWidth + 50;
		int y = 0;
		swprintf_s(&screen[(y + 2) * nScreenWidth + (x + 2)], 29, L"Total Passengers waiting: %02d", buildingScreen->totalPassengersWaiting);
		y = 1;
		swprintf_s(&screen[(y + 2) * nScreenWidth + (x + 2)], 33, L"Total Passengers transported: %02d", buildingScreen->totalPassengersTransported);
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
		//this_thread::sleep_for(10000ms);
	}

	void updateElevatorFloor(Elevator* e) {
		unique_lock<mutex> ul(screenLock);
		int x = 6 + e->elevatorID*3;
		int y = buildingHeight - e->currFloor;
		swprintf_s(&screen[(y + 2) * nScreenWidth + (x + 2)], 3, L"  ");
		if (e->direction == 1) y--;
		else y++;
		swprintf_s(&screen[(y + 2) * nScreenWidth + (x + 2)], 3, L"E%d", e->elevatorID + 1);
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	void updateElevatorStatus(Elevator* e) {
		unique_lock<mutex> ul(screenLock);
		int x = buildingWidth + 5;
		int y = 7 * e->elevatorID;
		swprintf_s(&screen[(y + 2) * nScreenWidth + (x + 2) + 15], 3, L"E%d", e->elevatorID + 1);
		swprintf_s(&screen[(y + 3) * nScreenWidth + (x + 2) + 15], 15, L"Elevator ID: %d", e->elevatorID);
		swprintf_s(&screen[(y + 4) * nScreenWidth + (x + 2) + 15], 11, L"# Pass: %02d", e->passengers.size());
		swprintf_s(&screen[(y + 5) * nScreenWidth + (x + 2) + 15], 15, L"Curr Floor: %02d", e->currFloor);
		swprintf_s(&screen[(y + 6) * nScreenWidth + (x + 2) + 15], 14, L"Next stop: %02d", e->nextDestFloor);

		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

	void updateTime() {
		unique_lock<mutex> ul(screenLock);
		int x = buildingWidth + 50;
		int y = 3;
		chrono::system_clock::time_point end = chrono::system_clock::now();
		chrono::system_clock::duration elapse = end - start;
		auto int_ms = chrono::duration_cast<chrono::milliseconds>(elapse);
		swprintf_s(&screen[(y + 2) * nScreenWidth + (x + 2)], 31, L"Simulation time: %10d ms", int_ms.count());
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}

};



