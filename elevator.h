#include <windows.h>
#include <chrono>
#include <thread>
#include <random>
#include <array>
#include <unordered_map>

#define MAX_FLOOR 7

enum class EState
{
	EMPTY,
	STOPPED,
	MOVING_UP,
	MOVING_DOWN
};

struct Passenger
{
	char name[25] {};
	float fWeight;
};

class Elevator
{
	int iCurrentFloor;
	int iLastChoosedFloor = -1;
	int iMaxFloor;
	float fDoorTimer = 0.f;
	bool bAreDoorsClosing = false;
	float fCurrentLoadWeight = 0.f;
	float fLoadWeightLimit;
	bool bIsDoorOpened = false;
	EState State = EState::EMPTY;

	std::array<bool, MAX_FLOOR> m_ChoosedFloors {};
	std::unordered_map<Passenger*, uint8_t> m_Passengers;

public:
	Elevator(int _iMaxFloor, float _fLoadWeightLimit);
	void Start();
	void Stop();
	void Run();
	void PrintState() const;

	void CallElevator(uint8_t iFromFloor);
	void ChooseFloor(uint8_t iFloor);
	void OnFloorChanged();
	void StopAndOutPassengers();

	void OpenDoors();
	void CloseDoors();
	bool EnterPassenger(Passenger* p);
	void OutPassenger(Passenger* p);
};