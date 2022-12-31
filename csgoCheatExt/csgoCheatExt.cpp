#include <iostream>
#include <Windows.h>
#include "MemMan.h"


MemMan MemClass;

struct runningProcess
{
	DWORD localPlayer;
	DWORD processID;
	uintptr_t moduleBase;
	BYTE flag;
	DWORD jump;
	int team;
	int tDelay;
	int currWeapon;
}proc;

struct gameOffsets
{
	DWORD jump = 0x52BBC9C;
	DWORD flags = 0x104;
	DWORD entList = 0x4DFFF04;
	DWORD isSpotted = 0x93D;
	DWORD localPlayer = 0xDEA964;
	DWORD flashDur = 0x10470;
	DWORD velocity = 0x114;
	DWORD FOV = 0x31F4;
	DWORD DefaultFOV = 0x333C;
	DWORD health = 0x100;
	DWORD forceAttack = 0x322DD10;
	DWORD crosshairId = 0x11838;
	DWORD teamNum = 0xF4;
	DWORD vecOrigin = 0x138;
	DWORD itemDefinitionIndex = 0x2FBA;
	DWORD activeWeapon = 0x2F08;
	DWORD bIsScoped = 0x9974;
}offsets;

struct Vector3
{
	float x, y, z;
};

bool isScoped()
{
	return (MemClass.readMem<bool>(proc.localPlayer + offsets.bIsScoped));
}

void setTBDelay(float distance)
{
	proc.tDelay = distance * 3.3;
	float delay = 0;

	switch (proc.currWeapon)
	{

		//m4a1-s
	case 60: delay = 3.0; break;

		//m4a4
	case 16: delay = 3.3; break;

		//mp5-sd
	case 23: delay = 3.0; break;

		//aug
	case 8: delay = 3.5; break;

		//famas
	case 10: delay = 3.3; break;

		//dual-berettas
	case 2: delay = 3.0; break;

		//five-seven
	case 3: delay = 3.0; break;

		//p90
	case 19: delay = 0.5; break;

		//mac10
	case 17: delay = 0.5; break;

		//ump
	case 24: delay = 2.5; break;

		//pp-bizon 
	case 26: delay = 0.5; break;

		//mp7
	case 33: delay = 2.5; break;

		//mp9
	case 34: delay = 1.5; break;

		//ak-47
	case 7: delay = 3.3; break;

		//scout
	case 40: delay = 0.2; break;

		//usp-s
	case 61: delay = 2.5; break;

		//glock-18
	case 4: delay = 1.5; break;

		//galil-ar
	case 13: delay = 3.0; break;

		//sg 553
	case 39: delay = 3.5; break;

		//awp
	case 9: delay = 0.2; break;

		//deagle
	case 1: delay = 6.5; break;

	default: delay = 0; break;
	}

	proc.tDelay = delay * distance;
}

float getDistance(DWORD ent)
{
	Vector3 playerLoc = MemClass.readMem<Vector3>(proc.localPlayer + offsets.vecOrigin);

	Vector3 enLoc = MemClass.readMem<Vector3>(ent + offsets.vecOrigin);

	return (sqrt(pow(playerLoc.x - enLoc.x, 2) + pow(playerLoc.y - enLoc.y, 2) + pow(playerLoc.z - enLoc.z, 2)) * 0.0254); //multipling by 0.0254 converts distance to meters
}

void shoot()
{
	Sleep(proc.tDelay);
	MemClass.writeMem(proc.moduleBase + offsets.forceAttack, 5);
	Sleep(1);
	MemClass.writeMem(proc.moduleBase + offsets.forceAttack, 4);
}

void getCurrWeapon()
{
	int weapon = MemClass.readMem<int>(proc.localPlayer + offsets.activeWeapon);
	int weaponEnt = MemClass.readMem<int>(proc.moduleBase + offsets.entList + (weapon & 0xFFF) * 0x10);

	if (weaponEnt)
	{
		proc.currWeapon = MemClass.readMem<int>(weaponEnt + offsets.itemDefinitionIndex);
		// std::cout << proc.currWeapon << std::endl;
	}
}

bool checktBot()
{
	int crosshair = MemClass.readMem<int>(proc.localPlayer + offsets.crosshairId);
	if (crosshair > 0 && crosshair < 64)
	{
		DWORD ent = MemClass.readMem<DWORD>(proc.moduleBase + offsets.entList + ((crosshair) * 0x10));
		int enTeam = MemClass.readMem<int>(ent + offsets.teamNum);
		int enHealth = MemClass.readMem<int>(ent + offsets.health);

		if (enTeam != proc.team && enHealth > 0)
		{
			getCurrWeapon();
			setTBDelay(getDistance(ent));

			if (proc.currWeapon == 40 || proc.currWeapon == 9)
			{
				return isScoped();
			}
			return true;
		}
		return false;
	}
	else
	{
		return false;
	}
}

bool isMoving(Vector3 vec)
{
	if ((int)vec.x != 0 || (int)vec.y != 0 || (int)vec.z != 0)
	{
		return true;
	}

	return false;
}

void handletBot()
{
	if (checktBot())
	{
		shoot();
	}
}


void writeCheatstoConsole(bool bhop, bool radar, bool antiflash, bool fov, bool bTrigger)
{
	system("CLS");

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	std::cout << "[NUM1] Bhop: ";

	if (bhop)
	{
		SetConsoleTextAttribute(hConsole, 10);

		std::cout << "ON" << std::endl;
	}
	else
	{
		SetConsoleTextAttribute(hConsole, 12);

		std::cout << "OFF" << std::endl;
	}

	SetConsoleTextAttribute(hConsole, 7);

	std::cout << "[NUM2] RadarHack: ";

	if (radar)
	{
		SetConsoleTextAttribute(hConsole, 10);

		std::cout << "ON" << std::endl;
	}
	else
	{
		SetConsoleTextAttribute(hConsole, 12);

		std::cout << "OFF" << std::endl;
	}

	SetConsoleTextAttribute(hConsole, 7);

	std::cout << "[NUM3] Anti Flash: ";

	if (antiflash)
	{
		SetConsoleTextAttribute(hConsole, 10);

		std::cout << "ON" << std::endl;
	}
	else
	{
		SetConsoleTextAttribute(hConsole, 12);

		std::cout << "OFF" << std::endl;
	}

	SetConsoleTextAttribute(hConsole, 7);

	std::cout << "[NUM4] FOV 130: ";

	if (fov)
	{
		SetConsoleTextAttribute(hConsole, 10);

		std::cout << "ON" << std::endl;
	}
	else
	{
		SetConsoleTextAttribute(hConsole, 12);

		std::cout << "OFF" << std::endl;
	}

	SetConsoleTextAttribute(hConsole, 7);

	std::cout << "[NUM5] TriggerBot: ";

	if (bTrigger)
	{
		SetConsoleTextAttribute(hConsole, 10);

		std::cout << "ON" << std::endl;
	}
	else
	{
		SetConsoleTextAttribute(hConsole, 12);

		std::cout << "OFF" << std::endl;
	}

	SetConsoleTextAttribute(hConsole, 7);
}



int main()
{
	bool bBhop = false, bRadarHack = false, bAntiFlash = false, bBigFov = false, bTrigger = false;;
	int flashDur = 0;
	writeCheatstoConsole(bBhop, bRadarHack, bAntiFlash, bBigFov, bTrigger);

	proc.processID = MemClass.getProcess("csgo.exe");
	proc.moduleBase = MemClass.getModule(proc.processID, "client.dll");

	double sleepTime = 0;

	while (true)
	{
		do
		{
			proc.localPlayer = MemClass.readMem<DWORD>((proc.moduleBase + offsets.localPlayer));
			proc.team = MemClass.readMem<int>(proc.localPlayer + offsets.teamNum);
		} while (proc.localPlayer == NULL);

		if (GetAsyncKeyState(VK_NUMPAD1) & 1)
		{
			bBhop = !bBhop;
			writeCheatstoConsole(bBhop, bRadarHack, bAntiFlash, bBigFov, bTrigger);
		}
		if (GetAsyncKeyState(VK_NUMPAD2) & 1)
		{
			bRadarHack = !bRadarHack;
			writeCheatstoConsole(bBhop, bRadarHack, bAntiFlash, bBigFov, bTrigger);
		}
		if (GetAsyncKeyState(VK_NUMPAD3) & 1)
		{
			bAntiFlash = !bAntiFlash;
			writeCheatstoConsole(bBhop, bRadarHack, bAntiFlash, bBigFov, bTrigger);
		}
		if (GetAsyncKeyState(VK_NUMPAD4) & 1)
		{
			//bBigFov = !bBigFov;
			writeCheatstoConsole(bBhop, bRadarHack, bAntiFlash, bBigFov, bTrigger);
		}
		if (GetAsyncKeyState(VK_NUMPAD5) & 1)
		{
			bTrigger = !bTrigger;
			writeCheatstoConsole(bBhop, bRadarHack, bAntiFlash, bBigFov, bTrigger);
		}


		if (bBhop)
		{
			proc.flag = MemClass.readMem<BYTE>(proc.localPlayer + offsets.flags);

			if (isMoving(MemClass.readMem<Vector3>(proc.localPlayer + offsets.velocity)))
			{
				if (GetAsyncKeyState(VK_SPACE) && proc.flag & (1 << 0))
				{
					proc.jump = proc.moduleBase + offsets.jump;
					MemClass.writeMem<DWORD>(proc.jump, 6);
				}
			}
		}
		
		if (bRadarHack)
		{
			for (int i = 0; i < 64; i++)
			{
				DWORD ent = MemClass.readMem<DWORD>(proc.moduleBase + offsets.entList + i * 0x10);
				if (ent != NULL)
				{
					MemClass.writeMem<bool>(ent + offsets.isSpotted, true);
				}
			}
		}

		if (bAntiFlash)
		{
			flashDur = MemClass.readMem<int>(proc.localPlayer + offsets.flashDur);
			if (flashDur != 0)
			{
				MemClass.writeMem(proc.localPlayer + offsets.flashDur, 0);
			}
		}
		/*
		if (bBigFov)
		{
			MemClass.writeMem(proc.localPlayer + offsets.FOV, 130);
		}
		else
		{
			MemClass.writeMem(proc.localPlayer + offsets.FOV, MemClass.readMem<int>(proc.localPlayer + offsets.DefaultFOV));
		}*/

		if (bTrigger)
		{
			handletBot();
		}

		Sleep(sleepTime);
	}

	return 0;
}
