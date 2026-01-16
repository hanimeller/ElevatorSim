#include <thread>
#include <iostream>
#include <windows.h>
#include "elevator.h"
#include <conio.h>
#include <chrono>

Elevator* elv = nullptr;

void InputReader()
{
	while (true)
	{
		if (GetAsyncKeyState(VK_RETURN))
			elv->EnterPassenger(new Passenger());
		else
		{
			for (int key = '0'; key <= '6'; key++)
			{
				if (GetAsyncKeyState(key))
				{
					uint8_t iFloorNumber = key - '0';
					elv->CallElevator(iFloorNumber);
				}

				while (GetAsyncKeyState(key))
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
}

void StartElevator()
{
	elv->Start();
}

int main()
{
	std::cout << "Max Kat Sayisi: 6\n"
	<< "Asansoru cagir: 1~6\nYolcu bindir: ENTER";

	elv = new Elevator(6, 300.f);
	std::thread elvThread(StartElevator);

	InputReader();

	elvThread.join();
	return 0;
}