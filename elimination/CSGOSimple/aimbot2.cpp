#include "aimbot2.h"
#include "options.hpp"
#include <algorithm>
#include "helpers\math.hpp"
#include "helpers\utils.hpp"
#include "helpers/InputSys.hpp"
#include "hooks.hpp"
#include "autowall.hpp"
#include <string>

#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / g_GlobalVars->interval_per_tick ) )
#define TICKS_TO_TIME( t )		( g_GlobalVars->interval_per_tick *( t ) )

#define LC_NONE				0
#define LC_ALIVE			(1<<0)

#define LC_ORIGIN_CHANGED	(1<<8)
#define LC_ANGLES_CHANGED	(1<<9)
#define LC_SIZE_CHANGED		(1<<10)
#define LC_ANIMATION_CHANGED (1<<11)

#define LAG_COMPENSATION_TELEPORTED_DISTANCE_SQR ( 64.0f * 64.0f )
#define LAG_COMPENSATION_EPS_SQR ( 0.1f * 0.1f )
// Allow 4 units of error ( about 1 / 8 bbox width )
#define LAG_COMPENSATION_ERROR_EPS_SQR ( 4.0f * 4.0f )

#define Assert( _exp ) ((void)0)

class CAutowallRecord
{
public:
	CAutowallRecord(float _flPredictedDamage, unsigned int _iBestBone)
	{
		flPredictedDamage = _flPredictedDamage;
		iBestBone = _iBestBone;
	}

	CAutowallRecord()
	{
		flPredictedDamage = 0.0f;
		iBestBone = 0;
	}

	~CAutowallRecord()
	{

	}

	float flPredictedDamage = 0.0f;
	unsigned int iBestBone = 0;
};

float GetFOV2(const QAngle& viewAngle, const QAngle& aimAngle)
{
	QAngle delta = aimAngle - viewAngle;
	Math::NormalizeAngles(delta);
	return sqrtf(powf(delta.pitch, 2.0f) + powf(delta.yaw, 2.0f));
}

namespace Aimbot
{
	float GetHitchance2()
	{
		float hitchance = 101;
		if (!g_LocalPlayer->m_hActiveWeapon())
			return 0.0f;

		if (g_Options.hitchance > 1)
		{
			float inaccuracy = g_LocalPlayer->m_hActiveWeapon()->GetInaccuracy();
			if (inaccuracy == 0) inaccuracy = 0.0000001f;
			inaccuracy = 1 / inaccuracy;
			hitchance = inaccuracy;

		}

		return hitchance;
	}



	QAngle AimbotCalcAngle2(Vector src, Vector dst)
	{
		QAngle ret;

		Vector delta = src - dst;
		double hyp = delta.Length2D(); //delta.Length
		ret.yaw = (atan(delta.y / delta.x) * 57.295779513082f);
		ret.pitch = (atan(delta.z / hyp) * 57.295779513082f);
		ret[2] = 0.00;
		if (delta.x >= 0.0) ret.yaw += 180.0f;
		return ret;
	}


	bool doRCS2(CUserCmd* pCmd)
	{
		if (g_EngineClient->IsInGame() && g_LocalPlayer->IsAlive() && g_Options.rcs_amount > 0.0f)
		{
			auto punchAngles = g_LocalPlayer->m_aimPunchAngle() * g_Options.rcs_amount;
			if (punchAngles.pitch != 0.0f || punchAngles.yaw != 0)
			{
				pCmd->viewangles -= punchAngles;
				if (g_Options.antiuntrusted)
				{
					Math::NormalizeAngles(pCmd->viewangles);
					Math::ClampAngles(pCmd->viewangles);
				}
				return false;
			}
		}
	}

	//bool doNoRCS2(CUserCmd* pCmd)
	//{
	//	if (g_EngineClient->IsInGame() && g_LocalPlayer->IsAlive() && g_Options.no_recoil == true)
	//	{
	//		auto punchAngles = g_LocalPlayer->m_aimPunchAngle() * 2;
	//		if (punchAngles.pitch != 0.0f || punchAngles.yaw != 0)
	//		{
	//			pCmd->viewangles -= punchAngles;
	//			if (g_Options.antiuntrusted)
	//			{
	//				Math::NormalizeAngles(pCmd->viewangles);
	//				Math::ClampAngles(pCmd->viewangles);
	//			}
	//			return false;
	//		}
	//	}
	//}

	QAngle Smooth2(QAngle finalangle, QAngle viewangles)
	{
		float smooth;
		QAngle delta = finalangle - viewangles;
		Math::NormalizeAngles(delta);
		Math::ClampAngles(delta);
		if (g_Options.aimbot_smoothness > 0.0f) smooth = powf(g_Options.aimbot_smoothness, 0.4f);
		else if (g_Options.aimbot_smoothness <= 0.0f) smooth = powf(0.0001f, 0.4f);
		smooth = std::min(0.99f, smooth);
		QAngle toChange = delta - delta * smooth;
		return toChange;
	}

	/*float GetFOV(const QAngle& viewAngle, const QAngle& aimAngle)
	{
	QAngle delta = aimAngle - viewAngle;
	Math::NormalizeAngles(delta);
	return sqrtf(powf(delta.pitch, 2.0f) + powf(delta.yaw, 2.0f));
	}*/

	C_BasePlayer* GetClosestPlayerToCrosshair2()
	{
		float bestFov;
		C_BasePlayer* closestEntity = nullptr;
		bestFov = g_Options.aimbot_AimbotFOV;
		if (!g_LocalPlayer) return nullptr;
		for (int i = 1; i < g_EngineClient->GetMaxClients(); ++i)
		{
			C_BasePlayer* player = (C_BasePlayer*)g_EntityList->GetClientEntity(i);

			if (player
				&& player != g_LocalPlayer
				&& !player->IsDormant()
				&& player->IsAlive()
				&& !player->m_bGunGameImmunity())
			{
				if (player->m_iTeamNum() != g_LocalPlayer->m_iTeamNum())
				{
					player_info_t entityInformation;
					g_EngineClient->GetPlayerInfo(i, &entityInformation);

					Vector eVecTarget = player->GetBonePos(8);
					Vector pVecTarget = g_LocalPlayer->GetEyePos();

					QAngle viewAngles;
					g_EngineClient->GetViewAngles(viewAngles);

					float distance = pVecTarget.DistTo(eVecTarget);
					float fov = GetFOV2(viewAngles, Math::CalcAngle1(pVecTarget, eVecTarget));
					int hp = player->m_iHealth();

					if (fov < bestFov)
					{
						closestEntity = player;
						bestFov = fov;
					}
				}
			}
		}
		return closestEntity;
	}

	C_AutoWall* Autowall2 = new C_AutoWall();

	// http://prntscr.com/hpcvi0 let me jsut save this here for now

	CAutowallRecord Hitscan(C_BasePlayer* pEntity)
	{
		float flBestDamage = 0.0f;
		float flPredictedDamage = 0.0f;
		int Hitbox = 0;
		auto pWeapon = g_LocalPlayer->m_hActiveWeapon();
		CAutowallRecord AutowallRecord;

		switch (g_Options.hitscan_amount)
		{
		case HITSCAN_NONE:
			// lmao
			break;
		case HITSCAN_LOW:
		{
			if (pWeapon) {
				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(8), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 8;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(6), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 6;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(3), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 3;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(39), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 34;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(73), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 73;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(66), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 66;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(67), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 67;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(74), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 74;
				}
			}
		}
		break;
		case HITSCAN_MEDIUM:
		{
			if (pWeapon) {
				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(8), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 8;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(38), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 38;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(39), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 39;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(11), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 11;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(83), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 83;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(4), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 3;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(37), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 37;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(72), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 72;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(66), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 66;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(73), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 73;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(67), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 67;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(74), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 74;
				}
			}
		}
		break;
		case HITSCAN_HIGH:
		{
			if (pWeapon) {
				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(8), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 8;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(7), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 7;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(2), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 2;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(6), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 6;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(5), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 5;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(4), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 4;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(3), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 3;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(77), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 77;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(70), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 70;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(73), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 73;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(66), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 66;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(74), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 74;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(67), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 67;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(75), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 75;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(69), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 69;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(83), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 83;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(36), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 36;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(39), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 39;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(51), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 51;
				}

				Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(17), flPredictedDamage, Hitbox);
				if (flPredictedDamage > flBestDamage)
				{
					flBestDamage = flPredictedDamage;

					AutowallRecord.flPredictedDamage = flPredictedDamage;
					AutowallRecord.iBestBone = 17;
				}
			}
		}
		break;
		case HITSCAN_EXTREME:
		{
			for (unsigned int i = 0; i < 81; ++i) // EVERY SINGLE BONE IN THE BODY!! EXTREME LAG!!! YOU BETTER HAVE 2 XEON CORES AND 3 GTX 1080TI IN UR PC
			{
				if (pWeapon) {
					Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, pEntity->GetBonePos(i), flPredictedDamage, Hitbox);

					if (flPredictedDamage > flBestDamage)
					{
						flBestDamage = flPredictedDamage;

						AutowallRecord.flPredictedDamage = flPredictedDamage;
						AutowallRecord.iBestBone = i;
					}
				}
			}
		}
		break;
		default:
			break;
		}

		return AutowallRecord;
	}

	void doAimbot2(CUserCmd* pCmd)
	{
		auto activeWeapon = g_LocalPlayer->m_hActiveWeapon();

		if (activeWeapon) {
			int Hitbox = 0;

			//			if (g_EngineClient->IsInGame() && g_LocalPlayer->IsAlive() && g_Options.aimbot_rage)
			{
				C_BasePlayer* pClosest = GetClosestPlayerToCrosshair2();

				if (pClosest && pClosest->m_iTeamNum() != g_LocalPlayer->m_iTeamNum() && pClosest->IsAlive())
				{
					if (g_Options.hitscan_amount > 0)
					{
						CAutowallRecord HitscanRecord = Hitscan(pClosest);

						QAngle AimbotTargetAngle;
						Vector BonePos = pClosest->GetBonePos(HitscanRecord.iBestBone);

						auto pWeapon = g_LocalPlayer->m_hActiveWeapon();

						/* ANTI-ONEWAY */
						Vector BonePosMin = BonePos;
						Vector BonePosMax = BonePos;

						bool GotBottom = false;

						for (float zAdd = 0.0f; zAdd <= 3.0f; zAdd += 0.5f)
						{
							float flPredictedDamage = 0.0f;
							int iHitHitbox = 0;

							if (Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, Vector(BonePos.x, BonePos.y, BonePos.z + zAdd), flPredictedDamage, iHitHitbox))
							{
								if (flPredictedDamage > 0.0f && iHitHitbox == HITGROUP_HEAD)
								{
									if (!GotBottom)
									{
										BonePosMin += zAdd;
										GotBottom = true;
									}

									BonePosMax += zAdd;
								}
							}
						}

						float hbDelta = BonePosMax.z - BonePosMin.z;
						BonePos.z += (hbDelta / 2.0f);
						/* ANTI-ONEWAY */

						AimbotTargetAngle = Math::CalcAngle1(g_LocalPlayer->GetEyePos(), BonePos);

						if (g_Options.aimbot_silent) pCmd->viewangles = AimbotTargetAngle;
						else if (!g_Options.aimbot_silent) g_EngineClient->SetViewAngles(AimbotTargetAngle);

						float Damage2 = 0;
						Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, BonePos, Damage2, Hitbox);

						if (g_Options.autoshoot)
						{
							if (Damage2 >= g_Options.autowall_min_damage)
							{
								auto activeWeapon = g_LocalPlayer->m_hActiveWeapon();

								if ((activeWeapon->GetCSWeaponData()->WeaponType != WEAPONTYPE_KNIFE) && (activeWeapon->GetCSWeaponData()->WeaponType != WEAPONTYPE_GRENADE) && (activeWeapon->GetCSWeaponData()->WeaponType != WEAPONTYPE_C4))
								{
									if (activeWeapon)
									{
										if (g_Options.hitchance <= GetHitchance2())
										{
											float flServerTime = g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick;
											bool canShoot = activeWeapon->m_flNextPrimaryAttack() <= flServerTime;

											static bool bJustShot = false;

											if (!bJustShot && canShoot && g_Options.autoshoot && !activeWeapon->IsReloading())
											{
												pCmd->buttons |= IN_ATTACK;
												bJustShot = !bJustShot;
											}
											else if (g_Options.autoscope && !g_LocalPlayer->m_bIsScoped() && !canShoot && !activeWeapon->IsReloading())
												pCmd->buttons |= IN_ATTACK2;
											if (bJustShot && g_Options.autoshoot && !activeWeapon->IsReloading())
											{
												bJustShot = !bJustShot;
											}
										}
									}
								}
							}
						}
					}
					else
					{
						QAngle AimbotTargetAngle;
						Vector BonePos = pClosest->GetBonePos(8);
						auto pWeapon = g_LocalPlayer->m_hActiveWeapon();

						/* ANTI-ONEWAY */
						Vector BonePosMin = BonePos;
						Vector BonePosMax = BonePos;

						bool GotBottom = false;

						for (float zAdd = 0.0f; zAdd <= 3.0f; zAdd += 0.5f)
						{
							float flPredictedDamage = 0.0f;
							int iHitHitbox = 0;

							if (Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, Vector(BonePos.x, BonePos.y, BonePos.z + zAdd), flPredictedDamage, iHitHitbox))
							{
								if (flPredictedDamage > 0.0f && iHitHitbox == HITGROUP_HEAD)
								{
									if (!GotBottom)
									{
										BonePosMin += zAdd;
										GotBottom = true;
									}

									BonePosMax += zAdd;
								}
							}
						}

						float hbDelta = BonePosMax.z - BonePosMin.z;
						BonePos.z += (hbDelta / 2.0f);
						/* ANTI-ONEWAY */

						AimbotTargetAngle = Math::CalcAngle1(g_LocalPlayer->GetEyePos(), BonePos);

						if (g_Options.aimbot_silent) pCmd->viewangles = AimbotTargetAngle;
						else if (!g_Options.aimbot_silent) g_EngineClient->SetViewAngles(AimbotTargetAngle);

						float Damage2 = 0;
						Autowall2->PenetrateWall(g_LocalPlayer, pWeapon, BonePos, Damage2, Hitbox);

						if (g_Options.autoshoot)
						{
							if (Damage2 >= g_Options.autowall_min_damage)
							{
								auto activeWeapon = (C_BaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(g_LocalPlayer->m_hActiveWeapon());

								if ((activeWeapon->GetCSWeaponData()->WeaponType != WEAPONTYPE_KNIFE) && (activeWeapon->GetCSWeaponData()->WeaponType != WEAPONTYPE_GRENADE) && (activeWeapon->GetCSWeaponData()->WeaponType != WEAPONTYPE_C4))
								{
									if (activeWeapon)
									{
										if (g_Options.hitchance <= GetHitchance2())
										{
											float flServerTime = g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick;
											bool canShoot = activeWeapon->m_flNextPrimaryAttack() <= flServerTime;

											static bool bJustShot = false;

											if (!bJustShot && canShoot && g_Options.autoshoot && !activeWeapon->IsReloading())
											{
												pCmd->buttons |= IN_ATTACK;
												bJustShot = !bJustShot;
											}
											else if (g_Options.autoscope && !g_LocalPlayer->m_bIsScoped() && !canShoot && !activeWeapon->IsReloading())
												pCmd->buttons |= IN_ATTACK2;
											if (bJustShot && g_Options.autoshoot && !activeWeapon->IsReloading())
											{
												bJustShot = !bJustShot;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}





































































































