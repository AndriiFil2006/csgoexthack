#include <iostream>
#include <Windows.h>
#include "MemMan.h"
#include "csgo.hpp"


MemMan MemClass;

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

struct runningProcess
{
	DWORD localPlayer;
	DWORD processID;
	uintptr_t moduleBase;
	uintptr_t engineModule;
	BYTE flag;
	DWORD jump;
	int playerTeam;
	int tDelay;
	int currWeapon;
	uintptr_t glowObj;
}proc;

struct ClrRender
{
	BYTE red, green, blue;
};

struct GlowStruct
{
	BYTE base[4];
	float red, green, blue, alpha;
	BYTE buffer[16];
	bool renderWhenIncluded, renderWhenUnIncluded, fullBloom;
	BYTE buffer1[5];
	int glowStyle;
}Glow;

ClrRender clrTeam;
ClrRender clrEnemy;

GlowStruct SetGlowColor(GlowStruct glow, uintptr_t ent)
{
	float red, green;
	bool bDefusing = MemClass.readMem<bool>(ent + m_bIsDefusing);

	if (bDefusing)
	{
		glow.red = 2.0f;
		glow.alpha = 2.0f;
		glow.green = 2.0f;
		glow.blue = 2.0f;
	}
	else
	{
		int health = MemClass.readMem<int>(ent + m_iHealth);
		green = health * 0.02f;
		red = 2.0f - green;

		glow.red = red;
		glow.alpha = 2.0f;
		glow.green = green;
	}

	glow.renderWhenIncluded = true;
	glow.renderWhenUnIncluded = false;

	/* Works
	glow.red = 2.0f;
	glow.alpha = 1.5f;
	glow.renderWhenIncluded = true;
	glow.renderWhenUnIncluded = false;*/

	return glow;
}

void SetTeamGlow(uintptr_t ent, int glowIndex)
{
	GlowStruct tGlow;
	tGlow = MemClass.readMem<GlowStruct>(proc.glowObj + (glowIndex * 0x38) + 0x4);

	tGlow.blue = 2.0f;
	tGlow.alpha = 1.5f;
	tGlow.renderWhenIncluded = true;
	tGlow.renderWhenUnIncluded = false;

	MemClass.writeMem<GlowStruct>(proc.glowObj + (glowIndex * 0x38) + 0x4, tGlow);
}


void SetEnemyGlow(uintptr_t ent, int glowIndex)
{
	GlowStruct eGlow;
	eGlow = MemClass.readMem<GlowStruct>(proc.glowObj + (glowIndex * 0x38) + 0x4);
	eGlow = SetGlowColor(eGlow, ent);

	/*
	eGlow.red = 2.0f;
	eGlow.alpha = 1.5f;
	eGlow.renderWhenIncluded = true;
	eGlow.renderWhenUnIncluded = false;
	*/


	MemClass.writeMem<GlowStruct>(proc.glowObj + (glowIndex * 0x38) + 0x4, eGlow);
}

void HandleGlow()
{
	clrTeam.red = 0;
	clrTeam.blue = 0;
	clrTeam.green = 255;

	clrEnemy.red = 255;
	clrEnemy.blue = 0;
	clrEnemy.green = 0;

	proc.glowObj = MemClass.readMem<uintptr_t>(proc.moduleBase + dwGlowObjectManager);

	for (short int i = -1; i < 64; i++)
	{
		uintptr_t ent = MemClass.readMem<uintptr_t>(proc.moduleBase + dwEntityList + i * 0x10);
		if (ent != NULL)
		{
			int glowIndex = MemClass.readMem<int>(ent + m_iGlowIndex);
			int eTeam = MemClass.readMem<int>(ent + m_iTeamNum);

			if (eTeam == proc.playerTeam)
			{
				MemClass.writeMem<ClrRender>(ent + m_clrRender, clrTeam);

				SetTeamGlow(ent, glowIndex);
			}
			else
			{

				MemClass.writeMem<ClrRender>(ent + m_clrRender, clrEnemy);

				/*
				Vector3 enemyPos = MemClass.readMem<Vector3>(ent + m_vecOrigin);
				Vector3 lPos = MemClass.readMem<Vector3>(proc.localPlayer + m_vecOrigin);
				Vector3 lAngle = MemClass.readMem<Vector3>(proc.localPlayer + m_vecViewOffset);
				*/

				SetEnemyGlow(ent, glowIndex);
			}
		}
	}
}






struct Vector3
{
	float x, y, z;
};

bool isScoped()
{
	return (MemClass.readMem<bool>(proc.localPlayer + m_bIsScoped));
}

void setTBDelay(float distance)
{
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
	Vector3 playerLoc = MemClass.readMem<Vector3>(proc.localPlayer + m_vecOrigin);

	Vector3 enLoc = MemClass.readMem<Vector3>(ent + m_vecOrigin);

	return (sqrt(pow(playerLoc.x - enLoc.x, 2) + pow(playerLoc.y - enLoc.y, 2) + pow(playerLoc.z - enLoc.z, 2)) * 0.0254); //multipling by 0.0254 converts distance to meters
}

void shoot()
{
	Sleep(proc.tDelay);
	MemClass.writeMem(proc.moduleBase + dwForceAttack, 5);
	Sleep(1);
	MemClass.writeMem(proc.moduleBase + dwForceAttack, 4);
}

void getCurrWeapon()
{
	int weapon = MemClass.readMem<int>(proc.localPlayer + m_hActiveWeapon);
	int weaponEnt = MemClass.readMem<int>(proc.moduleBase + dwEntityList + (weapon & 0xFFF) * 0x10);

	if (weaponEnt)
	{
		proc.currWeapon = MemClass.readMem<int>(weaponEnt + m_iItemDefinitionIndex);
		// std::cout << proc.currWeapon << std::endl;
	}
}

bool checktBot()
{
	int crosshair = MemClass.readMem<int>(proc.localPlayer + m_iCrosshairId);
	if (crosshair > 0 && crosshair < 64)
	{
		DWORD ent = MemClass.readMem<DWORD>(proc.moduleBase + dwEntityList + ((crosshair - 1) * 0x10));
		int enTeam = MemClass.readMem<int>(ent + m_iTeamNum);
		int enHealth = MemClass.readMem<int>(ent + m_iHealth);

		// enTeam != proc.playerTeam &&

		if (enTeam != proc.playerTeam && enHealth > 0)
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

void handletriggerBot()
{
	if (checktBot())
	{
		shoot();
	}
}


void writeCheatstoConsole(bool bhop, bool radar, bool antiflash, bool bTrigger, bool bBrightness, bool bWallHack)
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

	std::cout << "[NUM4] TriggerBot: ";

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

	std::cout << "[NUM5] Bright models: ";

	if (bBrightness)
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

	std::cout << "[NUM6] WallHack: ";

	if (bWallHack)
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


void SetBrightness(float brightness)
{
	clrTeam.red = 0;
	clrTeam.blue = 0;
	clrTeam.green = 255;

	clrEnemy.red = 255;
	clrEnemy.blue = 0;
	clrEnemy.green = 0;

	//DWORD ptr = MemClass.readMem<int>(proc.engineModule + model_ambient_min);
	uintptr_t thisPtr = (uintptr_t)(proc.engineModule + model_ambient_min - 0x2C);

	uintptr_t somePtr = *(uintptr_t*)&brightness;
	uintptr_t xorptr = *(uintptr_t*)&brightness ^ thisPtr;
	int someInt = MemClass.readMem<int>(proc.engineModule + model_ambient_min);
	MemClass.writeMem<int>(proc.engineModule + model_ambient_min, xorptr);
}



int main()
{
	bool bBhop = false, bRadarHack = false, bAntiFlash = false, bBigFov = false, bTrigger = false, bBrightness = false, bWallHack = false;
	int flashDur = 0;
	writeCheatstoConsole(bBhop, bRadarHack, bAntiFlash, bTrigger, bBrightness, bWallHack);

	proc.processID = MemClass.getProcess(L"csgo.exe");
	proc.moduleBase = MemClass.getModule(proc.processID, L"client.dll");
	proc.engineModule = MemClass.getModule(proc.processID, L"engine.dll");

	double sleepTime = 0;

	while (true)
	{
		do
		{
			proc.localPlayer = MemClass.readMem<DWORD>((proc.moduleBase + dwLocalPlayer));
			proc.playerTeam = MemClass.readMem<int>(proc.localPlayer + m_iTeamNum);
		} while (proc.localPlayer == NULL);

		if (GetAsyncKeyState(VK_NUMPAD1) & 1)
		{
			bBhop = !bBhop;
			writeCheatstoConsole(bBhop, bRadarHack, bAntiFlash, bTrigger, bBrightness, bWallHack);
		}
		if (GetAsyncKeyState(VK_NUMPAD2) & 1)
		{
			bRadarHack = !bRadarHack;
			writeCheatstoConsole(bBhop, bRadarHack, bAntiFlash, bTrigger, bBrightness, bWallHack);
		}
		if (GetAsyncKeyState(VK_NUMPAD3) & 1)
		{
			bAntiFlash = !bAntiFlash;
			writeCheatstoConsole(bBhop, bRadarHack, bAntiFlash, bTrigger, bBrightness, bWallHack);
		}
		if (GetAsyncKeyState(VK_NUMPAD4) & 1)
		{
			bTrigger = !bTrigger;
			writeCheatstoConsole(bBhop, bRadarHack, bAntiFlash, bTrigger, bBrightness, bWallHack);
		}
		if (GetAsyncKeyState(VK_NUMPAD5) & 1)
		{
			bBrightness = !bBrightness;
			writeCheatstoConsole(bBhop, bRadarHack, bAntiFlash, bTrigger, bBrightness, bWallHack);
		}
		if (GetAsyncKeyState(VK_NUMPAD6) & 1)
		{
			bWallHack = !bWallHack;
			writeCheatstoConsole(bBhop, bRadarHack, bAntiFlash, bTrigger, bBrightness, bWallHack);
		}
		/*
		if (GetAsyncKeyState(VK_NUMPAD6) & 1)
		{
			bWallHack = !bWallHack;
			writeCheatstoConsole(bBhop, bRadarHack, bAntiFlash, bBigFov, bTrigger, bWallHack);
		}
		*/

		if (bBhop)
		{
			proc.flag = MemClass.readMem<BYTE>(proc.localPlayer + m_fFlags);

			if (isMoving(MemClass.readMem<Vector3>(proc.localPlayer + m_vecVelocity)))
			{
				if (GetAsyncKeyState(VK_SPACE) && proc.flag & (1 << 0))
				{
					proc.jump = proc.moduleBase + dwForceJump;
					MemClass.writeMem<DWORD>(proc.jump, 6);
				}
			}
		}

		if (bRadarHack)
		{
			for (int i = 0; i < 64; i++)
			{
				DWORD ent = MemClass.readMem<DWORD>(proc.moduleBase + dwEntityList + i * 0x10);
				if (ent != NULL)
				{
					MemClass.writeMem<bool>(ent + m_bSpotted, true);
				}
			}
		}

		if (bAntiFlash)
		{
			flashDur = MemClass.readMem<int>(proc.localPlayer + m_flFlashDuration);
			if (flashDur != 0)
			{
				MemClass.writeMem(proc.localPlayer + m_flFlashDuration, 0);
			}
		}
		/*
		*
		* Currently unavaliable due to lack of usage
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
			handletriggerBot();
		}
		if (bBrightness)
		{
			SetBrightness(6.0f);
		}
		else
		{
			SetBrightness(0.0f);
		}
		if (bWallHack)
		{
			HandleGlow();
		}


		Sleep(sleepTime);
	}

	return 0;
}
