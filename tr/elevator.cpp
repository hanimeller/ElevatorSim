#include "elevator.h"
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <algorithm>

int rand(int min, int max)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<> distr(min, max);
	return distr(gen);
}

Elevator::Elevator(int _iMaxFloor, float _fLoadWeightLimit)
	: iCurrentFloor(0)
	, iMaxFloor(_iMaxFloor)
	, fLoadWeightLimit(_fLoadWeightLimit)
	, State(EState::EMPTY)
	, bIsDoorOpened(false)
	, bAreDoorsClosing(false)
{
	for(int i=0; i < MAX_FLOOR; ++i)
		m_ChoosedFloors[i] = false;
	
	std::cout << "\nAsansor aktif.";
}

void Elevator::Run()
{
	if (bIsDoorOpened || bAreDoorsClosing)
		return;

	int targetFloor = -1;
	for (int i = 0; i <= iMaxFloor; ++i)
	{
		if (m_ChoosedFloors[i])
		{
			targetFloor = i;
			break; 
		}
	}

	if (targetFloor != -1)
	{
		if (targetFloor > iCurrentFloor) 
			State = EState::MOVING_UP;
		else if (targetFloor < iCurrentFloor) 
			State = EState::MOVING_DOWN;
		else
			State = EState::STOPPED; // Zaten hedef katta
	}
	else
	{
		State = EState::EMPTY;
	}
}

void Elevator::Start()
{
	std::thread t1([this]()
	{
		auto lastTime = std::chrono::high_resolution_clock::now();
		float moveTimer = 0.f;

		while (true)
		{
			auto currentTime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> deltaTime = currentTime - lastTime;
			lastTime = currentTime;
			float dt = deltaTime.count();

			// Sadece kapi kapaliyken ve hareket halindeyken kat degistir
			if (!bIsDoorOpened && !bAreDoorsClosing && 
			   (State == EState::MOVING_UP || State == EState::MOVING_DOWN))
			{
				moveTimer += dt;
				if (moveTimer >= 1.0f) // Her 1 saniyede 1 kat
				{
					if (State == EState::MOVING_UP)
						iCurrentFloor = std::min(iCurrentFloor + 1, iMaxFloor);
					else
						iCurrentFloor = std::max(iCurrentFloor - 1, 0);

					moveTimer = 0.f;
					OnFloorChanged();
				}
			}

			if (bIsDoorOpened && !bAreDoorsClosing)
			{
				fDoorTimer += dt;
				if (fDoorTimer >= 3.0f)
				{
					CloseDoors();
					PrintState();
					fDoorTimer = 0.0f;
				}
			}

			Run(); // Her dongude durumu guncelle
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	});
	t1.detach();
}

void Elevator::OnFloorChanged()
{
	PrintState();

	if (m_ChoosedFloors[iCurrentFloor])
	{
		m_ChoosedFloors[iCurrentFloor] = false; // Hedefe ulasti
		StopAndOutPassengers();
	}
}

void Elevator::StopAndOutPassengers()
{
	State = EState::STOPPED;
	PrintState();
	OpenDoors();

	auto it = m_Passengers.begin();
	while (it != m_Passengers.end())
	{
		if (it->second == iCurrentFloor)
		{
			std::cout << it->first->name << " hedef katina ulasti ve iniyor.";
			fCurrentLoadWeight = std::max(0.f, fCurrentLoadWeight - it->first->fWeight);
			it = m_Passengers.erase(it);
		} 
		else 
		{
			++it;
		}
	}
}

void Elevator::OpenDoors()
{
	if (bIsDoorOpened)
		return;

	std::cout << "\nKapilar aciliyor...";
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout << "\nKapilar acildi.";

	bIsDoorOpened = true;
	fDoorTimer = 0.f;
}

void Elevator::CloseDoors()
{
	if (!bIsDoorOpened)
		return;

	std::cout << "\nKapilar kapaniyor...";
	bAreDoorsClosing = true;
	std::this_thread::sleep_for(std::chrono::seconds(1));

	bAreDoorsClosing = false;
	bIsDoorOpened = false;
	std::cout << "\nKapilar kapandi.";
}

void Elevator::ChooseFloor(uint8_t iFloor)
{
	if (iFloor > iMaxFloor)
		return;

	if (iFloor == iCurrentFloor && !bIsDoorOpened)
	{
		OpenDoors();
		PrintState();
	}
	else
	{
		std::cout << "\n" << (int)iFloor << ". Kat siraya eklendi";
		m_ChoosedFloors[iFloor] = true;
	}
}

void Elevator::CallElevator(uint8_t iFromFloor)
{
	std::cout << "\n" << (int)iFromFloor << ". kattan cagrildi.";
	ChooseFloor(iFromFloor);
}

bool Elevator::EnterPassenger(Passenger* p)
{
	if (!bIsDoorOpened || bAreDoorsClosing)
		return false;

	sprintf(p->name, "Yolcu-%03d", rand(0, 999));
	int iChoosedFloor = rand(0, iMaxFloor);
	p->fWeight = (float)rand(5, 95);

	if (fCurrentLoadWeight + p->fWeight > fLoadWeightLimit)
	{
		std::cout << "\nAgirlik siniri asildigi icin "
		<< p->name << " geri indi.";

		fDoorTimer = 0.f; // Her inen icin kapi suresini sifirla
		delete p;
		return false;
	}
	fCurrentLoadWeight += p->fWeight;

	// Mevcut kati secerse binmis sayilmaz, kapi acik kalir
	if(iChoosedFloor == iCurrentFloor)
		iChoosedFloor = (iCurrentFloor + 1) % iMaxFloor;

	m_Passengers.emplace(p, iChoosedFloor);
	std::cout << "\n" << p->name << " bindi. " << iChoosedFloor << ". Kati secti.";
	
	ChooseFloor(iChoosedFloor);
	fDoorTimer = 0.f; // Her binen icin kapi suresini sifirla
	return true;
}

void Elevator::PrintState() const
{
	switch (State)
	{
		case EState::EMPTY:
			std::cout << "\033[38;2;217;217;217m\nAsansor bosta.\033[0m";
			break;
		case EState::STOPPED:
			std::cout << "\033[38;2;255;0;0m\nAsansor durdu.\033[0m";
			break;
		case EState::MOVING_UP:
			std::cout << "\033[38;2;61;252;3m\nAsansor yukari cikiyor.\033[0m";
			break;
		case EState::MOVING_DOWN:
			std::cout << "\033[38;2;255;221;0m\nAsansor asagi iniyor.\033[0m";
			break;
	}
	std::cout << " --> Kat: ["
	<< iCurrentFloor
	<< "] "
	<< " Agirlik: "
	<< fCurrentLoadWeight;
}