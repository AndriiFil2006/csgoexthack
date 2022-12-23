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
}proc;

struct gameOffsets
{
	DWORD jump = 0x52BBC9C;
	DWORD flags = 0x104;
	DWORD entList = 0x4DFFF04;
	DWORD isSpotted = 0x93D;
	DWORD localPlayer = 0xDEA964;
	DWORD flashDur = 0x10470;
}offsets;



void writeCheatstoConsole(bool bhop, bool radar, bool antiflash)
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
}



int main()
{
    bool bBhop = false, bRadarHack = false, bAntiFlash = false;
	int flashDur = 0;
	writeCheatstoConsole(bBhop, bRadarHack, bAntiFlash);

	proc.processID = MemClass.getProcess("csgo.exe");
	proc.moduleBase = MemClass.getModule(proc.processID, "client.dll");

	double sleepTime = 0;

	while (true)
	{
		do
		{
			proc.localPlayer = MemClass.readMem<DWORD>((proc.moduleBase + offsets.localPlayer));
		} while (proc.localPlayer == NULL);

		if (GetAsyncKeyState(VK_NUMPAD1) & 1)
		{
			bBhop = !bBhop;
			writeCheatstoConsole(bBhop, bRadarHack, bAntiFlash);
		}
		if (GetAsyncKeyState(VK_NUMPAD2) & 1)
		{
			bRadarHack = !bRadarHack;
			writeCheatstoConsole(bBhop, bRadarHack, bAntiFlash);
		}
		if (GetAsyncKeyState(VK_NUMPAD3) & 1)
		{
			bAntiFlash = !bAntiFlash;
			writeCheatstoConsole(bBhop, bRadarHack, bAntiFlash);
		}

		if (bBhop)
		{
			proc.flag = MemClass.readMem<BYTE>(proc.localPlayer + offsets.flags);

			if (GetAsyncKeyState(VK_SPACE) && proc.flag & (1 << 0))
			{
				proc.jump = proc.moduleBase + offsets.jump;
				MemClass.writeMem<DWORD>(proc.jump, 6);
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

		Sleep(sleepTime);
	}

	return 0;
}
