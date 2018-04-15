#pragma once
#include "options.hpp"
#include "../CSGOSimple/valve_sdk/csgostructs.hpp"
#include "../CSGOSimple/helpers/utils.hpp"
#include "../CSGOSimple/helpers/math.hpp"
#include <DirectXMath.h>
#define MDRRRAD2DEG(x) DirectX::XMConvertToDegrees(x)
#define MDRRDEG2RAD(x) DirectX::XMConvertToRadians(x)
#define MDRRM_PI		3.14159265358979323846f

int backtracking_ticks = 12;

void inline MDRRSinCos(float radians, float* sine, float* cosine)
{
	*sine = sin(radians);
	*cosine = cos(radians);
}

void MDRRAngleVectors(const QAngle& angles, Vector* forward)
{
	float sp, sy, cp, cy;

	MDRRSinCos(MDRRDEG2RAD(angles.yaw), &sy, &cy);
	MDRRSinCos(MDRRDEG2RAD(angles.pitch), &sp, &cp);

	forward->x = cp * cy;
	forward->y = cp * sy;
	forward->z = -sp;
}
float MDRRGetFov(const QAngle& viewAngle, const QAngle& aimAngle)
{
	Vector ang, aim;

	MDRRAngleVectors(viewAngle, &aim);
	MDRRAngleVectors(aimAngle, &ang);

	return MDRRRAD2DEG(acos(aim.Dot(ang) / aim.LengthSqr()));
}
void MDRRVectorAngles(const Vector& forward, QAngle& angles)
{
	if (forward[1] == 0.0f && forward[0] == 0.0f)
	{
		angles[0] = (forward[2] > 0.0f) ? 270.0f : 90.0f; // Pitch (up/down)
		angles[1] = 0.0f; //yaw left/right
	}
	else
	{
		angles[0] = atan2(-forward[2], forward.Length2D()) * -180 / MDRRM_PI;
		angles[1] = atan2(forward[1], forward[0]) * 180 / MDRRM_PI;

		if (angles[1] > 90)
			angles[1] -= 180;
		else if (angles[1] < 90)
			angles[1] += 180;
		else if (angles[1] == 90)
			angles[1] = 0;
	}

	angles[2] = 0.0f;
}
QAngle MDRRCalcAngle(Vector src, Vector dst)
{
	QAngle angles;
	Vector delta = src - dst;
	MDRRVectorAngles(delta, angles);
	delta.Normalized();
	return angles;
}
struct BacktrackRecord
{
	C_BasePlayer* entity;
	Vector head;
	Vector origin;
};

struct BacktrackTick
{
	int tickcount;
	std::vector<BacktrackRecord> records;
};

class CBacktracking
{
	std::vector<BacktrackTick> ticks;
	C_BasePlayer* entity;
	Vector prevOrig;

public:
	void RegisterTick(CUserCmd* cmd)
	{
		auto pWeapon = g_LocalPlayer->m_hActiveWeapon();

		if (pWeapon) {
			if (pWeapon->IsPistol())
				backtracking_ticks = g_Options.pistol_backtracking_ticks;
			else if (pWeapon->IsRifle())
				backtracking_ticks = g_Options.rifle_backtracking_ticks;
			else if (pWeapon->IsSniper())
				backtracking_ticks = g_Options.sniper_backtracking_ticks;
			else if (pWeapon->IsShotgun())
				backtracking_ticks = g_Options.shotgun_backtracking_ticks;
			else if (pWeapon->IsSMG())
				backtracking_ticks = g_Options.smg_backtracking_ticks;
		}
		ticks.insert(ticks.begin(), BacktrackTick{ cmd->tick_count });
		auto& cur = ticks[0];

		while (ticks.size() > backtracking_ticks)
			ticks.pop_back();

		for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
		{
			auto entity = C_BasePlayer::GetPlayerByIndex(i);
			if (!entity ||
				entity->IsDormant() ||
				entity->m_iHealth() <= 0 ||
				entity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum() ||
				entity->m_bGunGameImmunity())
				continue;

			cur.records.emplace_back(BacktrackRecord{ entity, entity->GetBonePos(aimspot), entity->m_vecOrigin() });
		}
	}

	void Begin(CUserCmd* cmd)
	{
		entity = nullptr;

		float serverTime = g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick;
		auto weapon = g_LocalPlayer->m_hActiveWeapon();
		if (weapon)
		{
			if (cmd->buttons & IN_ATTACK && weapon->m_flNextPrimaryAttack() < serverTime + 0.001)
			{
				float fov = 170.5f;
				int tickcount = 0;
				bool hasTarget = false;
				Vector orig;

				for (auto& tick : ticks)
				{
					for (auto& record : tick.records)
					{
						QAngle angle = MDRRCalcAngle(g_LocalPlayer->GetEyePos(), record.head);
						float tmpFOV = MDRRGetFov(cmd->viewangles, angle);

						if (tmpFOV < fov)
						{
							fov = tmpFOV;
							tickcount = tick.tickcount;
							hasTarget = true;
							entity = record.entity;
							orig = record.origin;

							static auto invalidateBoneCache = Utils::PatternScan(GetModuleHandleW(L"client.dll"), "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81");
							unsigned long modelBoneCounter = **(unsigned long**)(invalidateBoneCache + 10);
							*(unsigned int*)((DWORD)entity + 0x2914) = 0xFF7FFFFF;
							*(unsigned int*)((DWORD)entity + 0x2680) = (modelBoneCounter - 1);
						}
					}
				}

				if (entity && hasTarget)
				{
					cmd->tick_count = tickcount;
					prevOrig = entity->m_vecOrigin();
					entity->m_vecOrigin() = orig;
				}
			}
		}
	}

	void End()
	{
		auto pWeapon = g_LocalPlayer->m_hActiveWeapon();

		if (pWeapon) {

			if (pWeapon->IsGrenade() || pWeapon->IsKnife() || pWeapon->m_iItemDefinitionIndex() == weapon_c4) return;

			if (pWeapon->IsPistol() && !g_Options.pistol_backtracking) return;
			else if (pWeapon->IsRifle() && !g_Options.rifle_backtracking) return;
			else if (pWeapon->IsSniper() && !g_Options.sniper_backtracking) return;
			else if (pWeapon->IsShotgun() && !g_Options.shotgun_backtracking) return;
			else if (pWeapon->IsSMG() && !g_Options.smg_backtracking) return;

			if (entity) entity->m_vecOrigin() = prevOrig;

			entity = nullptr;
		}
	}

	void Draw()
	{
		auto pWeapon = g_LocalPlayer->m_hActiveWeapon();

		if (pWeapon) {

			if (pWeapon->IsGrenade() || pWeapon->IsKnife() || pWeapon->m_iItemDefinitionIndex() == weapon_c4) return;

			if (pWeapon->IsPistol() && !g_Options.pistol_backtracking) return;
			else if (pWeapon->IsRifle() && !g_Options.rifle_backtracking) return;
			else if (pWeapon->IsSniper() && !g_Options.sniper_backtracking) return;
			else if (pWeapon->IsShotgun() && !g_Options.shotgun_backtracking) return;
			else if (pWeapon->IsSMG() && !g_Options.smg_backtracking) return;

			for (auto& tick : ticks)
			{
				for (auto& record : tick.records)
				{
					Vector screenPos;
					if (Math::WorldToScreen(record.head, screenPos))
					{
						g_VGuiSurface->DrawSetColor(63, 255, 223, 255);
						g_VGuiSurface->DrawFilledRect(screenPos.x, screenPos.y, screenPos.x + 2, screenPos.y + 2);
					}
				}
			}
		}
	}

};

CBacktracking Backtracking;