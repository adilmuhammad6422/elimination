#pragma once
#include "valve_sdk\csgostructs.hpp"

//class CBacktracking
//{
//public:
//	void RegisterTick(CUserCmd* cmd);
//	void Begin(CUserCmd* cmd);
//	void End();
//	void Draw();
//};
//extern CBacktracking Backtracking;


namespace Aimbot
{
	bool doRCS2(CUserCmd* pCmd);
	bool doNoRCS2(CUserCmd* pCmd);
	void doAimbot2(CUserCmd* pCmd);
}
