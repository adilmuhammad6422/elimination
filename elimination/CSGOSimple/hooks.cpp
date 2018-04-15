#include "Hooks.hpp"
#include "Menu.hpp"
#include "Options.hpp"
#include "helpers/InputSys.hpp"
#include "helpers/Utils.hpp"
#include "features/bhop.hpp"
#include "features/Chams.hpp"
#include "features/Visuals.hpp"
#include "features/glow.hpp"
#include "features/memewalk.h"
#include "features/nightmode.h"
#include "features/skinchanger.h"
#include "features/grenadeprediction.h"
#include "features/thirdperson.h"
#include "features\visuals.hpp"
#include "Aimbot.h"
#include "AntiAims.h"
#include "SpectatorList.h"
#include "FakeLag.h"
#include "AutoStrafer.h"
#include "Circlestrafer.h"
#include "hitmarkers.h"
#include "triggerbot.h"
#include "backtracking.h"
#include "clantag.h"
#include "helpers/math.hpp"
#include "Resolver.h"
#include "aimbot2.h"

#include "SpoofedConvar.hpp"
#include "fonts\IconsFontAwesome.h"

//-----------------------------------------------------------------
// How to spoof convars example:
//
// ConVar* sv_cheats = g_CVar->FindVar("sv_cheats");
// SpoofedConvar* sv_cheats_spoofed = new SpoofedConvar(sv_cheats);
// sv_cheats_spoofed->SetInt(1);
//-----------------------------------------------------------------

#define RandomInt(min, max) (rand() % (max - min + 1) + min)

// Frame Timer
int FrameNum = 0;

bool lbyIndicator = false;
bool doTrace = false;


namespace Hooks
{
	static CreateClientClassFn GetWearableCreateFn()
	{
		auto pClass = g_CHLClient->GetAllClasses();

		while (strcmp(pClass->m_pNetworkName, "CEconWearable"))
			pClass = pClass->m_pNext;

		return pClass->m_pCreateFn;
	}

	// Function to apply skin data to weapons.
	inline bool ApplyCustomSkin(C_BaseAttributableItem* pWeapon, int nWeaponIndex, int skin) 
	{
		// Apply our changes to the fallback variables.
		pWeapon->m_nFallbackPaintKit() = skin;
		pWeapon->m_iEntityQuality() = 0.0f;
		pWeapon->m_nFallbackSeed() = 0;
		pWeapon->m_nFallbackStatTrak() = -1;
		pWeapon->m_flFallbackWear() = 0.000000001f;

		pWeapon->m_iItemDefinitionIndex() = nWeaponIndex;

		pWeapon->m_iItemIDHigh() = -1;

		return true;
	}

	inline bool ApplyCustomKnifeSkin(C_BaseAttributableItem* pWeapon, int nWeaponIndex, int skin)
	{
		// Apply our changes to the fallback variables.
		pWeapon->m_nFallbackPaintKit() = skin;
		pWeapon->m_iEntityQuality() = 3.0f;
		pWeapon->m_nFallbackSeed() = 0;
		pWeapon->m_nFallbackStatTrak() = -1;
		pWeapon->m_flFallbackWear() = 0.000000001f;

		pWeapon->m_iItemDefinitionIndex() = nWeaponIndex;

		pWeapon->m_iItemIDHigh() = -1;

		return true;
	}

	// Function to apply custom view models to weapons.
	inline bool ApplyCustomModel(C_BaseAttributableItem* pWeapon, const char* vMdl)
	{
		auto pLocal = (C_BasePlayer*)g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer());

		// Get the view model of this weapon.
		C_BaseViewModel* pViewModel = pLocal->m_hViewModel().Get();

		if (!pViewModel)
			return false;

		// Get the weapon belonging to this view model.
		auto hViewModelWeapon = pViewModel->m_hWeapon();
		C_BaseAttributableItem* pViewModelWeapon = (C_BaseAttributableItem*)g_EntityList->GetClientEntityFromHandle(hViewModelWeapon);

		if (pViewModelWeapon != pWeapon)
			return false;

		// Check if an override exists for this view model.
		int nViewModelIndex = pViewModel->m_nModelIndex();

		// Set the replacement model.
		pViewModel->m_nModelIndex() = g_MdlInfo->GetModelIndex(vMdl);

		return true;
	}

	vfunc_hook hlclient_hook;
	vfunc_hook direct3d_hook;
	vfunc_hook vguipanel_hook;
	vfunc_hook vguisurf_hook;
	vfunc_hook mdlrender_hook;
	vfunc_hook clientmode_hook;
	vfunc_hook gameevents_hook;
	vfunc_hook prediction_hook;
	vfunc_hook cvar_hook;
	vfunc_hook engine_hook;
	vfunc_hook mdlcache_hook;
	RecvPropHook* g_sequence_hook;

	// watermark font
	vgui::HFont watermark_font;

	// console watermark memes
	int consoleMeme = 0;

#define SEQUENCE_DEFAULT_DRAW 0
#define SEQUENCE_DEFAULT_IDLE1 1
#define SEQUENCE_DEFAULT_IDLE2 2
#define SEQUENCE_DEFAULT_LIGHT_MISS1 3
#define SEQUENCE_DEFAULT_LIGHT_MISS2 4
#define SEQUENCE_DEFAULT_HEAVY_MISS1 9
#define SEQUENCE_DEFAULT_HEAVY_HIT1 10
#define SEQUENCE_DEFAULT_HEAVY_BACKSTAB 11
#define SEQUENCE_DEFAULT_LOOKAT01 12

#define SEQUENCE_BUTTERFLY_DRAW 0
#define SEQUENCE_BUTTERFLY_DRAW2 1
#define SEQUENCE_BUTTERFLY_LOOKAT01 13
#define SEQUENCE_BUTTERFLY_LOOKAT03 15

#define SEQUENCE_FALCHION_IDLE1 1
#define SEQUENCE_FALCHION_HEAVY_MISS1 8
#define SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP 9
#define SEQUENCE_FALCHION_LOOKAT01 12
#define SEQUENCE_FALCHION_LOOKAT02 13

#define SEQUENCE_DAGGERS_IDLE1 1
#define SEQUENCE_DAGGERS_LIGHT_MISS1 2
#define SEQUENCE_DAGGERS_LIGHT_MISS5 6
#define SEQUENCE_DAGGERS_HEAVY_MISS2 11
#define SEQUENCE_DAGGERS_HEAVY_MISS1 12

#define SEQUENCE_BOWIE_IDLE1 1

	RecvVarProxyFn fnSequenceProxyFn;
	void SetViewModelSequence(const CRecvProxyData *pDataConst, void *pStruct, void *pOut)
	{
		CRecvProxyData* pData = const_cast<CRecvProxyData*>(pDataConst);

		// Confirm that we are replacing our view model and not someone elses.
		C_BaseViewModel* pViewModel = (C_BaseViewModel*)pStruct;

		if (pViewModel) {
			IClientEntity* pOwner = g_EntityList->GetClientEntityFromHandle(pViewModel->m_hOwner());

			// Compare the owner entity of this view model to the local player entity.
			if (pOwner && pOwner->EntIndex() == g_EngineClient->GetLocalPlayer()) {
				// Get the filename of the current view model.
				const model_t* pModel = g_MdlInfo->GetModel(pViewModel->m_nModelIndex());
				const char* szModel = g_MdlInfo->GetModelName(pModel);

				// Store the current sequence.
				int m_nSequence = pData->m_Value.m_Int;

				if (!strcmp(szModel, "models/weapons/v_knife_butterfly.mdl")) {
					// Fix animations for the Butterfly Knife.
					switch (m_nSequence) {
					case SEQUENCE_DEFAULT_DRAW:
						m_nSequence = RandomInt(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2); break;
					case SEQUENCE_DEFAULT_LOOKAT01:
						m_nSequence = RandomInt(SEQUENCE_BUTTERFLY_LOOKAT01, SEQUENCE_BUTTERFLY_LOOKAT03); break;
					default:
						m_nSequence++;
					}
				}
				else if (!strcmp(szModel, "models/weapons/v_knife_falchion_advanced.mdl")) {
					// Fix animations for the Falchion Knife.
					switch (m_nSequence) {
					case SEQUENCE_DEFAULT_IDLE2:
						m_nSequence = SEQUENCE_FALCHION_IDLE1; break;
					case SEQUENCE_DEFAULT_HEAVY_MISS1:
						m_nSequence = RandomInt(SEQUENCE_FALCHION_HEAVY_MISS1, SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP); break;
					case SEQUENCE_DEFAULT_LOOKAT01:
						m_nSequence = RandomInt(SEQUENCE_FALCHION_LOOKAT01, SEQUENCE_FALCHION_LOOKAT02); break;
					case SEQUENCE_DEFAULT_DRAW:
					case SEQUENCE_DEFAULT_IDLE1:
						break;
					default:
						m_nSequence--;
					}
				}
				else if (!strcmp(szModel, "models/weapons/v_knife_push.mdl")) {
					// Fix animations for the Shadow Daggers.
					switch (m_nSequence) {
					case SEQUENCE_DEFAULT_IDLE2:
						m_nSequence = SEQUENCE_DAGGERS_IDLE1; break;
					case SEQUENCE_DEFAULT_LIGHT_MISS1:
					case SEQUENCE_DEFAULT_LIGHT_MISS2:
						m_nSequence = RandomInt(SEQUENCE_DAGGERS_LIGHT_MISS1, SEQUENCE_DAGGERS_LIGHT_MISS5); break;
					case SEQUENCE_DEFAULT_HEAVY_MISS1:
						m_nSequence = RandomInt(SEQUENCE_DAGGERS_HEAVY_MISS2, SEQUENCE_DAGGERS_HEAVY_MISS1); break;
					case SEQUENCE_DEFAULT_HEAVY_HIT1:
					case SEQUENCE_DEFAULT_HEAVY_BACKSTAB:
					case SEQUENCE_DEFAULT_LOOKAT01:
						m_nSequence += 3; break;
					case SEQUENCE_DEFAULT_DRAW:
					case SEQUENCE_DEFAULT_IDLE1:
						break;
					default:
						m_nSequence += 2;
					}
				}
				else if (!strcmp(szModel, "models/weapons/v_knife_survival_bowie.mdl")) {
					// Fix animations for the Bowie Knife.
					switch (m_nSequence) {
					case SEQUENCE_DEFAULT_DRAW:
					case SEQUENCE_DEFAULT_IDLE1:
						break;
					case SEQUENCE_DEFAULT_IDLE2:
						m_nSequence = SEQUENCE_BOWIE_IDLE1; break;
					default:
						m_nSequence--;
					}
				}

				// Set the fixed sequence.
				pData->m_Value.m_Int = m_nSequence;
			}
		}

		// Call original function with the modified data.
		fnSequenceProxyFn(pData, pStruct, pOut);
	}

    void Initialize()
    {
		hlclient_hook.setup(g_CHLClient);
		direct3d_hook.setup(g_D3DDevice9);
		vguipanel_hook.setup(g_VGuiPanel);
		vguisurf_hook.setup(g_VGuiSurface);
		mdlrender_hook.setup(g_MdlRender);
		gameevents_hook.setup(g_GameEvents);
		prediction_hook.setup(g_Prediction);
		cvar_hook.setup(g_CVar);
		clientmode_hook.setup(g_ClientMode);
		mdlcache_hook.setup(g_MdlCache);
		engine_hook.setup(g_EngineClient);

		direct3d_hook.hook_index(index::EndScene, hkEndScene);
		direct3d_hook.hook_index(index::Reset, hkReset);
		hlclient_hook.hook_index(index::FrameStageNotify, hkFrameStageNotify);
		hlclient_hook.hook_index(index::CreateMove, hkCreateMove_Proxy);
		vguipanel_hook.hook_index(index::PaintTraverse, hkPaintTraverse);
		vguisurf_hook.hook_index(index::PlaySound, hkPlaySound);
		mdlrender_hook.hook_index(index::DrawModelExecute, hkDrawModelExecute);
		gameevents_hook.hook_index(index::FireEventClientSide, hkFireEventClientSide);
		prediction_hook.hook_index(index::InPrediction, hkInPrediction);
		cvar_hook.hook_index(index::SVCheats, hkSVCheats);
		clientmode_hook.hook_index(index::ClientmodeCreateMove, hkClientmodeCreateMove);
		clientmode_hook.hook_index(index::OverrideView, hkOverrideView);
		clientmode_hook.hook_index(index::DoPostScreenSpaceEffects, hkDoPostScreenEffects);
		mdlcache_hook.hook_index(index::FindMDL, hkFindMDL);
		engine_hook.hook_index(8, hkGetPlayerInfo);

		// Spoofing sv_cheats
		static ConVar* vn234789vms89gdfv9f3 = g_CVar->FindVar("sv_cheats");
		static SpoofedConvar* sv_cheats_spoofed = new SpoofedConvar(vn234789vms89gdfv9f3);
		sv_cheats_spoofed->SetInt(1);
		meme = true;

		// Seperate font intialization for 'reasons'
		watermark_font = g_VGuiSurface->CreateFont_();
		g_VGuiSurface->SetFontGlyphSet(watermark_font, "Segoe UI Light", 16, 700, 0, 0, FONTFLAG_DROPSHADOW | FONTFLAG_ANTIALIAS);
        Visuals::CreateFonts();
		consoleMeme = 1;

		for (ClientClass* pClass = g_CHLClient->GetAllClasses(); pClass; pClass = pClass->m_pNext)
		{
			if (!strcmp(pClass->m_pNetworkName, "CBaseViewModel")) 
			{
				// Search for the 'm_nModelIndex' property.
				RecvTable* pClassTable = pClass->m_pRecvTable;

				for (int nIndex = 0; nIndex < pClassTable->m_nProps; nIndex++) 
				{
					RecvProp* pProp = &pClassTable->m_pProps[nIndex];

					if (!pProp || strcmp(pProp->m_pVarName, "m_nSequence"))
						continue;

					// Store the original proxy function.
					fnSequenceProxyFn = pProp->m_ProxyFn;

					// Replace the proxy function with our sequence changer.
					pProp->m_ProxyFn = (RecvVarProxyFn)SetViewModelSequence;

					break;
				}

				break;
			}
		}
    }
    //--------------------------------------------------------------------------------
    void Shutdown()
    {
		// Finding CVars for unhooking
		static auto cl_mouseenable = g_CVar->FindVar("cl_mouseenable");
		static auto cl_drawhud = g_CVar->FindVar("cl_drawhud");
		static auto crosshair = g_CVar->FindVar("crosshair");
		static auto mat_postprocess_enable = g_CVar->FindVar("mat_postprocess_enable");
		static auto mat_ambient_light_r = g_CVar->FindVar("mat_ambient_light_r");
		static auto mat_ambient_light_g = g_CVar->FindVar("mat_ambient_light_g");
		static auto mat_ambient_light_b = g_CVar->FindVar("mat_ambient_light_b");
		static auto viewmodel_fov = g_CVar->FindVar("viewmodel_fov");

		// Setting CVar values back to default
		cl_mouseenable->SetValue(true);
		crosshair->SetValue(true);
		cl_drawhud->SetValue(true);
		mat_postprocess_enable->SetValue(true);
		mat_ambient_light_r->SetValue(0.f);
		mat_ambient_light_g->SetValue(0.f);
		mat_ambient_light_b->SetValue(0.f);
		viewmodel_fov->SetValue(68);

        hlclient_hook.unhook_all();
        direct3d_hook.unhook_all();
        vguipanel_hook.unhook_all();
        vguisurf_hook.unhook_all();
        mdlrender_hook.unhook_all();
		gameevents_hook.unhook_all();
		prediction_hook.unhook_all();
		cvar_hook.unhook_all();
		clientmode_hook.unhook_all();
		Glow::Get().Shutdown();
        Visuals::DestroyFonts();
    }
    //--------------------------------------------------------------------------------
	long __stdcall hkEndScene(IDirect3DDevice9* device)
	{
		auto oEndScene = direct3d_hook.get_original<EndScene>(index::EndScene);

		// do this three frames after startup to stop that annoying sv_cheats notification from popping up after spoofing that shit
		if (consoleMeme == 5)
		{
			// conosole shit
			Color clr = Color(150, 255, 150, 255);
			g_EngineClient->ExecuteClientCmd("clear");

			Color clr1 = Color(0, 255, 0, 255);
			g_CVar->ConsoleColorPrintf(clr1, "Welcome to Kawaii.Exposed, user");

			consoleMeme++;
		}
		else if (consoleMeme != 6) consoleMeme++;

		if (!g_EngineClient->IsTakingScreenshot())
		{
			static auto viewmodel_fov = g_CVar->FindVar("viewmodel_fov");
			static auto mat_ambient_light_r = g_CVar->FindVar("mat_ambient_light_r");
			static auto mat_ambient_light_g = g_CVar->FindVar("mat_ambient_light_g");
			static auto mat_ambient_light_b = g_CVar->FindVar("mat_ambient_light_b");
			static auto crosshair_cvar = g_CVar->FindVar("crosshair");

			viewmodel_fov->m_fnChangeCallbacks.m_Size = 0;
			viewmodel_fov->SetValue(68 + g_Options.viewmodel_fov);
			mat_ambient_light_r->SetValue(g_Options.mat_ambient_light_r);
			mat_ambient_light_g->SetValue(g_Options.mat_ambient_light_g);
			mat_ambient_light_b->SetValue(g_Options.mat_ambient_light_b);
			crosshair_cvar->SetValue(!g_Options.esp_crosshair);
		}
		else if (g_EngineClient->IsTakingScreenshot())
		{
			static auto viewmodel_fov = g_CVar->FindVar("viewmodel_fov");
			static auto mat_ambient_light_r = g_CVar->FindVar("mat_ambient_light_r");
			static auto mat_ambient_light_g = g_CVar->FindVar("mat_ambient_light_g");
			static auto mat_ambient_light_b = g_CVar->FindVar("mat_ambient_light_b");
			static auto crosshair_cvar = g_CVar->FindVar("crosshair");

			viewmodel_fov->m_fnChangeCallbacks.m_Size = 0;
			viewmodel_fov->SetValue(68);
			mat_ambient_light_r->SetValue(0.f);
			mat_ambient_light_g->SetValue(0.f);
			mat_ambient_light_b->SetValue(0.f);
			crosshair_cvar->SetValue(true);
		}

		Menu::Get().Render();

		if (g_Options.rank_reveal && InputSys::Get().IsKeyDown(VK_TAB))
			Utils::RankRevealAll();
		return oEndScene(device);
	}
    //--------------------------------------------------------------------------------
	long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		auto oReset = direct3d_hook.get_original<Reset>(index::Reset);

		Visuals::DestroyFonts();
		Menu::Get().OnDeviceLost();

		auto hr = oReset(device, pPresentationParameters);

		if (hr >= 0) 
		{
			Menu::Get().OnDeviceReset();
			Visuals::CreateFonts();
		}

		return hr;
	}


	bool __stdcall hkClientmodeCreateMove(float input_sample_frametime, CUserCmd* pCmd)
	{
		auto oCreateMove = clientmode_hook.get_original<ClientmodeCreateMove>(index::ClientmodeCreateMove);

		oCreateMove(g_ClientMode, input_sample_frametime, pCmd);

		if (!pCmd->command_number) return true;

		if (g_LocalPlayer && g_LocalPlayer->IsAlive() && g_EngineClient->IsInGame())
		{

		}

		return false;
	}

	MDLHandle_t __stdcall hkFindMDL(char* FilePath)
	{
		auto oFindMdl = mdlcache_hook.get_original<FindMDL>(index::FindMDL);


		switch (g_Options.knife_model) {
		case No_Knife_Model:
			break;
		case Minecraft_Pickaxe:  // Added fire axe as the Bayo cannot be added in its format
			
			MDLHandle_t __fastcall hkFindMDL(void* ecx, void* edx, const char* pMDLRelativePath);

			if (strstr(FilePath, "v_knife_default_ct.mdl") || strstr(FilePath, "v_knife_default_t.mdl"))
			{
				sprintf(FilePath, "models/weapons/v_minecraft_pickaxe.mdl");

			}
			break;

		case Banana:  // bAnaNana

			MDLHandle_t __fastcall hkFindMDL(void* ecx, void* edx, const char* pMDLRelativePath);

			if (strstr(FilePath, "v_knife_default_ct.mdl") || strstr(FilePath, "v_knife_default_t.mdl"))
			{
				sprintf(FilePath, "models/weapons/eminem/bananabit/v_bananabit.mdl");

			}
			break;
		}


		switch (g_Options.player_model) {
		case No_Model:
			break;

		case Reina_Kousaka:
			
			MDLHandle_t __fastcall hkFindMDL(void* ecx, void* edx, const char* pMDLRelativePath);

			if (strstr(FilePath, "tm_") && strstr(FilePath, ".mdl") && !strstr(FilePath, "ctm_"))
			{
				sprintf(FilePath, "models/player/custom_player/caleon1/reinakousaka/reina_red.mdl");
			}
			if (strstr(FilePath, "ctm_") && strstr(FilePath, ".mdl"))
			{
				sprintf(FilePath, "models/player/custom_player/caleon1/reinakousaka/reina_blue.mdl");
			}
			break;

		case Mirai_Nikki:
			
			MDLHandle_t __fastcall hkFindMDL(void* ecx, void* edx, const char* pMDLRelativePath);
			
			if (strstr(FilePath, "tm_") && strstr(FilePath, ".mdl") && !strstr(FilePath, "ctm_"))
			{
				sprintf(FilePath, "models/player/custom_player/voikanaa/mirainikki/gasaiyono.mdl");
			
			}

		case Banana_Joe:
			
			MDLHandle_t __fastcall hkFindMDL(void* ecx, void* edx, const char* pMDLRelativePath);
			
			if (strstr(FilePath, "tm_") && strstr(FilePath, ".mdl") && !strstr(FilePath, "ctm_"))
			{
				sprintf(FilePath, "models/player/custom_player/kuristaja/octodad/octodad_tuxedo.mdl");
			}
			if (strstr(FilePath, "ctm_") && strstr(FilePath, ".mdl"))
			{
				sprintf(FilePath, "models/player/custom_player/kuristaja/octodad/octodad_tuxedo.mdl");
			}
		}
		return oFindMdl(g_MdlCache, FilePath);
	}
	//--------------------------------------------------------------------------------
	bool ExSendPacket = false;
	//--------------------------------------------------------------------------------
	void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket)
	{
		auto oCreateMove = hlclient_hook.get_original<CreateMove>(index::CreateMove);

		oCreateMove(g_CHLClient, sequence_number, input_sample_frametime, active);

		auto pCmd = g_Input->GetUserCmd(sequence_number);
		auto verified = g_Input->GetVerifiedCmd(sequence_number);

		if (!pCmd || !pCmd->command_number)
			return;

		if (bSendPacket && g_Options.antiaim_thirdperson_angle == ANTIAIM_THIRDPERSON_REAL) ChamFakeAngle = pCmd->viewangles;
		else if (!bSendPacket && g_Options.antiaim_thirdperson_angle == ANTIAIM_THIRDPERSON_FAKE) ChamFakeAngle = pCmd->viewangles;
		else if (g_Options.antiaim_thirdperson_angle == ANTIAIM_THIRDPERSON_BOTH) ChamFakeAngle = pCmd->viewangles;

		if (g_EngineClient->IsInGame())
		{
			Backtracking.RegisterTick(pCmd);

			if (g_LocalPlayer && g_LocalPlayer->IsAlive())
			{
				auto pWeapon = g_LocalPlayer->m_hActiveWeapon();
				if (pWeapon) {
					if (pWeapon->IsPistol() && g_Options.pistol_backtracking) Backtracking.Begin(pCmd);
					else if (pWeapon->IsRifle() && g_Options.rifle_backtracking) Backtracking.Begin(pCmd);
					else if (pWeapon->IsSniper() && g_Options.sniper_backtracking) Backtracking.Begin(pCmd);
					else if (pWeapon->IsShotgun() && g_Options.shotgun_backtracking) Backtracking.Begin(pCmd);
					else if (pWeapon->IsSMG() && g_Options.smg_backtracking) Backtracking.Begin(pCmd);

					if (pWeapon->IsPistol() && g_Options.pistol_backtracking) Backtracking.End();
					else if (pWeapon->IsRifle() && g_Options.rifle_backtracking) Backtracking.End();
					else if (pWeapon->IsSniper() && g_Options.sniper_backtracking) Backtracking.End();
					else if (pWeapon->IsShotgun() && g_Options.shotgun_backtracking) Backtracking.End();
					else if (pWeapon->IsSMG() && g_Options.smg_backtracking) Backtracking.End();
				}
			}
		}

		QAngle oldAngle = pCmd->viewangles;
		float oldForward = pCmd->forwardmove;
		float oldSideMove = pCmd->sidemove;

		// if (g_LocalPlayer->IsAlive() && g_EngineClient->IsInGame()) CircleStrafer::OnCreateMove(pCmd);

		// Bunnyhop
		if (g_LocalPlayer->IsAlive() && g_EngineClient->IsInGame())
		BunnyHop::OnCreateMove(pCmd);


		//if (g_Options.aimbot_rage == true) 
			//{
			//	g_Options.aimbot_AimbotFOV = 180;
			//}


		// Moonwalk
		if (g_Options.memewalk && g_LocalPlayer->IsAlive() && g_EngineClient->IsInGame())
			memewalk::OnCreateMove(pCmd);

		// Spectator List
		specList::doSpectatorList();

		// Airstuck
		if (InputSys::Get().IsKeyDown(g_Options.airstuck_key) && g_EngineClient->IsInGame() && !(pCmd->buttons & IN_ATTACK)  && !(pCmd->buttons & IN_ATTACK2) && !(pCmd->buttons & IN_USE) && g_LocalPlayer->IsAlive()) pCmd->tick_count = 16777216;
	//	if (g_LocalPlayer->IsAlive() && g_EngineClient->IsInGame()) fake_lag::on_create_move(pCmd);
		if (g_LocalPlayer->IsAlive() && g_EngineClient->IsInGame()) AutoStrafer::OnCreateMove(pCmd);
		if (g_LocalPlayer->IsAlive() && g_EngineClient->IsInGame()) Aimbot::doAimbot(pCmd);
		if (g_LocalPlayer->IsAlive() && g_EngineClient->IsInGame()) Aimbot::doRCS(pCmd);
		if (g_LocalPlayer->IsAlive() && g_EngineClient->IsInGame()) Aimbot::doAimbot2(pCmd);

		//if (g_Options.no_recoil == true) {
		//	Aimbot::doNoRCS2(pCmd);
		//}

	//	if (g_LocalPlayer->IsAlive()) {
	//		Aimbot::doNoRCS(pCmd);
	//	}

		// thirdperson
		thirdperson::FrameStageNotify();

		// switching antiaim side
		static bool sidePress = false;
		if (InputSys::Get().IsKeyDown(g_Options.tankAntiaimKey) && !sidePress)
		{
			aaSide = !aaSide;
			sidePress = true;
		}
		else if (!InputSys::Get().IsKeyDown(g_Options.tankAntiaimKey) && sidePress)
		{
			sidePress = false;
		}

		if (g_LocalPlayer->IsAlive() && g_EngineClient->IsInGame()) AntiAims::OnCreateMove(pCmd);

		// Fake walk not working correctly i dont know how to make xD
		// AntiAims::FakeWalk(pCmd);

		grenade_prediction::Get().Tick(pCmd->buttons);

		if (pCmd->buttons & IN_ATTACK) doTrace = true;
		else doTrace = false;

		// packets
		if (g_LocalPlayer->IsAlive()) bSendPacket = sendPacket;
		else if (!g_LocalPlayer->IsAlive()) bSendPacket = true;

		// Getting antiaim angles
		if (!bSendPacket && g_Options.antiaim_thirdperson_angle == ANTIAIM_THIRDPERSON_REAL) LastTickViewAngles = pCmd->viewangles;
		else if (bSendPacket && g_Options.antiaim_thirdperson_angle == ANTIAIM_THIRDPERSON_FAKE) LastTickViewAngles = pCmd->viewangles;
		else if (g_Options.antiaim_thirdperson_angle == ANTIAIM_THIRDPERSON_BOTH) LastTickViewAngles = pCmd->viewangles;

		if (g_EngineClient->IsInGame() && g_LocalPlayer->IsAlive())
		{
			C_BaseCombatWeapon *activeWeapon = (C_BaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(g_LocalPlayer->m_hActiveWeapon());
			auto weapon = g_LocalPlayer->m_hActiveWeapon();
			if (weapon)
			{
				if (!g_Options.antiaim_knife && activeWeapon->GetCSWeaponData()->WeaponType == WEAPONTYPE_KNIFE)
				{
					// set LastTickViewAngles to my view
					static QAngle vA;
					g_EngineClient->GetViewAngles(vA);
					LastTickViewAngles = vA;
				}
			}
		}

		if (g_Options.triggerbotactive)
			triggerbot::Triggerbot(pCmd);

		if (g_LocalPlayer->m_vecVelocity().Length() > 35.0f)
		{
			if (g_LocalPlayer->m_fFlags() & FL_ONGROUND) lbyIndicator = false;
			else if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND)) lbyIndicator = true;
		}
		else lbyIndicator = true;

		if (g_Options.antiuntrusted)
		{
			Math::NormalizeAngles(pCmd->viewangles);
			Math::ClampAngles(pCmd->viewangles);
		}

		Math::CorrectMovement(oldAngle, pCmd, oldForward, oldSideMove);

		//if (g_LocalPlayer->IsAlive())
		//{
		//	AntiAims::LegitAA(pCmd, bSendPacket);
		//}

		verified->m_cmd = *pCmd;
		verified->m_crc = pCmd->GetChecksum();

		ExSendPacket = bSendPacket;
	}
	//--------------------------------------------------------------------------------
	bool __fastcall hkFireEventClientSide(IGameEventManager2* thisptr, void* edx, IGameEvent* pEvent)
	{
		auto oFunc = gameevents_hook.get_original<FireEventClientSide>(index::FireEventClientSide);

		// No events? just call the original.
		if (!pEvent)
			return oFunc(thisptr, pEvent);

		Resolver::FireGameEvent(pEvent);


			return oFunc(thisptr, pEvent);
	}
    //--------------------------------------------------------------------------------
	__declspec(naked) void __stdcall hkCreateMove_Proxy(int sequence_number, float input_sample_frametime, bool active)
	{
		__asm
		{
			push ebp
			mov  ebp, esp
			push ebx
			lea  ecx, [esp]
			push ecx
			push dword ptr[active]
			push dword ptr[input_sample_frametime]
			push dword ptr[sequence_number]
			call Hooks::hkCreateMove
			pop  ebx
			pop  ebp
			retn 0Ch
		}
	}
	//--------------------------------------------------------------------------------
	void DrawFilledCircle(Vector2D position, float points, float radius)
	{
		std::vector<Vertex_t> vertices;
		float step = (float)M_PI * 2.0f / points;

		for (float a = 0; a < (M_PI * 2.0f); a += step)
			vertices.push_back(Vertex_t(Vector2D(radius * cosf(a) + position.x, radius * sinf(a) + position.y)));

		g_VGuiSurface->DrawTexturedPolygon(points, vertices.data());
	}
	//--------------------------------------------------------------------------------
	void DrawFilledCircle(float x, float y, float points, float radius)
	{
		std::vector<Vertex_t> vertices;
		float step = (float)M_PI * 2.0f / points;

		for (float a = 0; a < (M_PI * 2.0f); a += step)
			vertices.push_back(Vertex_t(Vector2D(radius * cosf(a) + x, radius * sinf(a) + y)));

		g_VGuiSurface->DrawTexturedPolygon(points, vertices.data());
	}
    //--------------------------------------------------------------------------------
	void __stdcall hkPaintTraverse(vgui::VPANEL panel, bool forceRepaint, bool allowForce)
	{
		static auto panelId = vgui::VPANEL{ 0 };
		static auto oPaintTraverse = vguipanel_hook.get_original<PaintTraverse>(index::PaintTraverse);

		oPaintTraverse(g_VGuiPanel, panel, forceRepaint, allowForce);

		if (!panelId) 
		{
			const auto panelName = g_VGuiPanel->GetName(panel);
			if (!strcmp(panelName, "FocusOverlayPanel")) panelId = panel;
		}
		else if (panelId == panel) 
		{
			if (g_Options.visuals_fov_circle && g_EngineClient->IsInGame() && g_LocalPlayer->IsAlive() && g_LocalPlayer)
			{
				auto pWeapon = g_LocalPlayer->m_hActiveWeapon();
				if (pWeapon)
				{
					static float aimbotFOV;
					if (pWeapon->IsPistol() && g_Options.pistol_aimbot_AimbotFOV > 0) aimbotFOV = g_Options.pistol_aimbot_AimbotFOV;
					else if (pWeapon->IsRifle() && g_Options.rifle_aimbot_AimbotFOV > 0) aimbotFOV = g_Options.rifle_aimbot_AimbotFOV;
					else if (pWeapon->IsSniper() && g_Options.sniper_aimbot_AimbotFOV > 0) aimbotFOV = g_Options.sniper_aimbot_AimbotFOV;
					else if (pWeapon->IsShotgun() && g_Options.shotgun_aimbot_AimbotFOV > 0) aimbotFOV = g_Options.shotgun_aimbot_AimbotFOV;
					else if (pWeapon->IsSMG() && g_Options.smg_aimbot_AimbotFOV > 0) aimbotFOV = g_Options.smg_aimbot_AimbotFOV;

					if (aimbotFOV > 0.0f && !pWeapon->IsKnife() && !pWeapon->IsGrenade() && pWeapon->m_iItemDefinitionIndex() != weapon_c4)
					{
						static float Radius = 0.0f;
						static float fov = 0.0f;
						int w, h;
						g_EngineClient->GetScreenSize(w, h);
						Radius = tanf((DEG2RAD(fov)) / 6) / tanf(97) * w;
						DrawFilledCircle((float)w / 2.0f, (float)h / 2.0f, 50, (float)Radius);
					}
				}
			}


			if (g_Options.watermarks == true)
			{
				g_VGuiSurface->DrawSetTextFont(watermark_font);
				g_VGuiSurface->DrawSetTextColor(Color(255, 255, 255, 255));
				g_VGuiSurface->DrawSetTextPos(10, 10);
				g_VGuiSurface->DrawPrintText(L"Elimination <3", wcslen(L"Elimination <3"));
			}

			if (g_Options.antiuntrusted && g_Options.watermarks == true)
			{
				g_VGuiSurface->DrawSetTextColor(Color(0, 255, 0, 255));
				g_VGuiSurface->DrawSetTextPos(10, 24);
				g_VGuiSurface->DrawPrintText(L"Anti Untrusted Enabled", wcslen(L"Anti Untrusted Enabled"));
			}
			else if (!g_Options.antiuntrusted && g_Options.watermarks == true)
			{
				g_VGuiSurface->DrawSetTextColor(Color(255, 0, 0, 255));
				g_VGuiSurface->DrawSetTextPos(10, 24);
				g_VGuiSurface->DrawPrintText(L"Anti Untrusted Disabled", wcslen(L"Anti Untrusted Disabled"));
			}

			if (g_EngineClient->IsConnected() && g_EngineClient->IsInGame() && g_Options.hitmarkers == 1)
				hitmarker::singleton()->on_paint();

			if (g_Options.backtracking_tracer && g_EngineClient->IsInGame()) Backtracking.Draw();

			if (g_EngineClient->IsInGame() && !g_EngineClient->IsTakingScreenshot()) 
			{
				// grenade prediction
			//	if (g_LocalPlayer->IsAlive() && g_Options.grenade_path_esp && sendPacket) grenadeprediction::PredictGrenade();
				grenade_prediction::Get().Paint();

				for (auto i = 1; i <= g_EntityList->GetMaxEntities(); ++i) 
				{
					auto entity = (C_BasePlayer*)C_BaseEntity::GetEntityByIndex(i);

					if (!entity)
						continue;

					if (entity == g_LocalPlayer)
						continue;

					if (i < 65 && g_Options.esp_enabled) 
					{
						auto player = entity;
						if (!entity->IsDormant() && player->IsAlive() && Visuals::Player::Begin(player)) 
						{
							if (g_Options.esp_player_snaplines) Visuals::Player::RenderSnapline();
							if (g_Options.esp_player_boxes)     Visuals::Player::RenderBox();
							if (g_Options.esp_player_weapons)   Visuals::Player::RenderWeapon();
							if (g_Options.esp_player_names)     Visuals::Player::RenderName();
							if (g_Options.esp_player_health)    Visuals::Player::RenderHealth();
							if (g_Options.esp_player_armour)    Visuals::Player::RenderArmour();
						//	if (g_Options.skeleton_shit)   Visuals::Misc::DrawSkeleton(player);
						}
					}
					else if (g_Options.esp_dropped_weapons && entity->IsWeapon() && g_Options.esp_enabled)
						Visuals::Misc::RenderWeapon((C_BaseCombatWeapon*)entity);
					else if (g_Options.esp_defuse_kit && entity->IsDefuseKit() && g_Options.esp_enabled)
						Visuals::Misc::RenderDefuseKit(entity);
					else if (entity->IsPlantedC4() && g_Options.esp_enabled && g_Options.esp_planted_c4)
							Visuals::Misc::RenderPlantedC4(entity);
				}

				if (g_Options.esp_crosshair)
					Visuals::Misc::RenderCrosshair();


				nightmode::doNightmode();
				
				switch (g_Options.chatspam) {
				case chatspam_none:
					break;
				case KAWAII_SPAM:
					Visuals::Misc::ChatSpam();
					break;
				case TUMBLR_SPAM:
					Visuals::Misc::ChatSpam();
					break;
				}

				if (g_Options.sky == VIETNAM)
					Utils::SetSky("vietnam");
				if (g_Options.sky == VERTIGO)
					Utils::SetSky("vertigo");
				if (g_Options.sky == SKY_CSGO_NIGHT02)
					Utils::SetSky("sky_csgo_night02");
				if (g_Options.sky == SKY_CSGO_NIGHT02B)
					Utils::SetSky("sky_csgo_night02b");

				if (g_LocalPlayer->IsAlive())
				{
					// getting screen size
					static int w;
					static int h;
					g_EngineClient->GetScreenSize(w, h);
		
				}
			}
		}
	}
	//--------------------------------------------------------------------------------

	void __stdcall hkPlaySound(const char* name)
	{
		static auto oPlaySound = vguisurf_hook.get_original<PlaySound>(index::PlaySound);

		oPlaySound(g_VGuiSurface, name);

		if (g_Options.autoaccept)
		{
			// Auto Accept
			if (strstr(name, "UI/competitive_accept_beep.wav")) 
			{
				static auto fnAccept = (void(*)())Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F8 83 EC 08 56 8B 35 ? ? ? ? 57 83 BE");

				// accepting the game
				fnAccept();

				// flashing window
				FLASHWINFO fi;
				fi.cbSize = sizeof(FLASHWINFO);
				fi.hwnd = InputSys::Get().GetMainWindow();
				fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
				fi.uCount = 0;
				fi.dwTimeout = 0;
				FlashWindowEx(&fi);
			}
		}
	}
	//--------------------------------------------------------------------------------
	void __stdcall hkOverrideView(CViewSetup* setup)
	{
		static auto ofunc = clientmode_hook.get_original<OverrideView>(index::OverrideView);

		if (g_EngineClient->IsInGame() && !g_EngineClient->IsTakingScreenshot())
		{
			if (!g_LocalPlayer->m_bIsScoped()) setup->fov += g_Options.fov;
		}

		grenade_prediction::Get().View(setup);

		ofunc(g_ClientMode, setup);
	}
	//--------------------------------------------------------------------------------
	int __stdcall hkDoPostScreenEffects(int a1)
	{
		auto oDoPostScreenEffects = clientmode_hook.get_original<DoPostScreenEffects>(index::DoPostScreenSpaceEffects);
		
		// glow
		if (g_LocalPlayer && g_Options.glow_enabled && !g_EngineClient->IsTakingScreenshot())
			Glow::Get().Run();
		else if (g_LocalPlayer && g_Options.glow_enabled && g_EngineClient->IsTakingScreenshot())
			Glow::Get().Shutdown();

		return oDoPostScreenEffects(g_ClientMode, a1);
	}
	//--------------------------------------------------------------------------------
	bool __stdcall hkInPrediction()
	{
		auto oInPrediction = prediction_hook.get_original<InPrediction>(index::InPrediction);

		if (g_Options.visuals_no_aimpunch || g_Options.no_recoil)
		{
			QAngle viewPunch = g_LocalPlayer->m_viewPunchAngle();
			QAngle aimPunch = g_LocalPlayer->m_aimPunchAngle();
			g_LocalPlayer->m_viewPunchAngle() -= (viewPunch + (aimPunch * 2 * 0.4499999f));
		}
		return oInPrediction(g_Prediction);
	}

	//--------------------------------------------------------------------------------
	void __stdcall hkFrameStageNotify(ClientFrameStage_t stage)
	{
		static auto ofunc = hlclient_hook.get_original<FrameStageNotify>(index::FrameStageNotify);

		auto pLocal = (C_BasePlayer*)g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer());

		if (g_EngineClient->IsInGame() && stage == FRAME_RENDER_START)
		{			
			// disable post processing
			static auto mat_postprocess_enable = g_CVar->FindVar("mat_postprocess_enable");
			if (!g_EngineClient->IsTakingScreenshot())
			{
				if (g_Options.disable_post_processing && mat_postprocess_enable)
					mat_postprocess_enable->SetValue(false);
				else if (!g_Options.disable_post_processing && mat_postprocess_enable)
					mat_postprocess_enable->SetValue(true);
			}
			else
			{
				mat_postprocess_enable->SetValue(true);
			}

			// Stage
			Visuals::Misc::RenderDrugs();
			Visuals::Misc::RenderPaper();
			Visuals::Misc::RenderLowResTextures();

			// precache models 


			// thirdperson fix, had in createmove for some reason??


			//  Clantag

			if (g_Options.clantag == NO_CLANTAG)
			{
				Utils::SetClantag("");
			}

			if (g_Options.clantag == ELIMINATION_STATIC)
			{
				Utils::SetClantag("$ Elimination $");
			}

			if (g_Options.clantag == UFFYA)
			{
				Utils::SetClantag("uff ya $");
			}

			if (g_Options.clantag == SILVER)
			{
				Utils::SetClantag("sorry im silver");
			}

			if (g_Options.clantag == BHOP)
			{
				Utils::SetClantag(".bhop.");
			}

			if (g_Options.clantag == VALVE)
			{
				Utils::SetClantag("[VALV\xE1\xB4\xB1]");
			}

			if (g_Options.clantag == SKEET)
			{
				static int counter = 0;
				static int motion = 0;
				int serverTime = g_GlobalVars->interval_per_tick * (float)g_LocalPlayer->m_nTickBase() * 2.5;

				if (counter % 48 == 0)
					motion++;

				int value = serverTime % 46;

				switch (value)
				{
				case 0:
					Utils::SetClantag("                  ");
					break;
				case 1:
					Utils::SetClantag("                 g");
					break;
				case 2:
					Utils::SetClantag("                ga");
					break;
				case 3:
					Utils::SetClantag("               gam");
					break;
				case 4:
					Utils::SetClantag("              game");
					break;
				case 5:
					Utils::SetClantag("             games");
					break;
				case 6:
					Utils::SetClantag("            gamese");
					break;
				case 7:
					Utils::SetClantag("           gamesen");
					break;
				case 8:
					Utils::SetClantag("          gamesens");
					break;
				case 9:
					Utils::SetClantag("         gamesense");
					break;
				case 10:
					Utils::SetClantag("        gamesense ");
					break;
				case 11:
					Utils::SetClantag("        gamesense ");
					break;
				case 12:
					Utils::SetClantag("        gamesense ");
					break;
				case 13:
					Utils::SetClantag("       gamesense  ");
					break;
				case 14:
					Utils::SetClantag("       gamesense  ");
					break;
				case 15:
					Utils::SetClantag("       gamesense  ");
					break;
				case 16:
					Utils::SetClantag("      gamesense   ");
					break;
				case 17:
					Utils::SetClantag("      gamesense   ");
					break;
				case 18:
					Utils::SetClantag("      gamesense   ");
					break;
				case 19:
					Utils::SetClantag("     gamesense    ");
					break;
				case 20:
					Utils::SetClantag("     gamesense    ");
					break;
				case 21:
					Utils::SetClantag("     gamesense    ");
					break;
				case 22:
					Utils::SetClantag("    gamesense     ");
					break;
				case 23:
					Utils::SetClantag("    gamesense     ");
					break;
				case 24:
					Utils::SetClantag("    gamesense     ");
					break;
				case 25:
					Utils::SetClantag("   gamesense      ");
					break;
				case 26:
					Utils::SetClantag("   gamesense      ");
					break;
				case 27:
					Utils::SetClantag("   gamesense      ");
					break;
				case 28:
					Utils::SetClantag("  gamesense       ");
					break;
				case 29:
					Utils::SetClantag("  gamesense       ");
					break;
				case 30:
					Utils::SetClantag("  gamesense       ");
					break;
				case 31:
					Utils::SetClantag(" gamesense        ");
					break;
				case 32:
					Utils::SetClantag(" gamesense        ");
					break;
				case 33:
					Utils::SetClantag(" gamesense        ");
					break;
				case 34:
					Utils::SetClantag("gamesense         ");
					break;
				case 35:
					Utils::SetClantag("gamesense         ");
					break;
				case 36:
					Utils::SetClantag("gamesense         ");
					break;
				case 37:
					Utils::SetClantag("amesense          ");
					break;
				case 38:
					Utils::SetClantag("mesense           ");
					break;
				case 39:
					Utils::SetClantag("esense            ");
					break;
				case 40:
					Utils::SetClantag("sense             ");
					break;
				case 41:
					Utils::SetClantag("ense              ");
					break;
				case 42:
					Utils::SetClantag("nse               ");
					break;
				case 43:
					Utils::SetClantag("se                ");
					break;
				case 44:
					Utils::SetClantag("e                 ");
					break;
				case 45:
					Utils::SetClantag("                  ");
					break;


				}
				counter++;
			}

			if (g_Options.clantag == OWO)
			{
				static int counter = 0;
				static int motion = 0;
				int serverTime = g_GlobalVars->interval_per_tick * (float)g_LocalPlayer->m_nTickBase() * 2.5;

				if (counter % 48 == 0)
					motion++;

				int value = serverTime % 35;

				switch (value)
				{
				case 0:Utils::SetClantag("                   "); break;
				case 1:Utils::SetClantag("                  o"); break;
				case 2:Utils::SetClantag("                 wo"); break;
				case 3:Utils::SetClantag("                owo"); break;
				case 4:Utils::SetClantag("               owo "); break;
				case 5:Utils::SetClantag("              owo  "); break;
				case 6:Utils::SetClantag("             owo   "); break;
				case 7:Utils::SetClantag("            owo    "); break;
				case 8:Utils::SetClantag("           owo     "); break;
				case 9:Utils::SetClantag("          owo      "); break;
				case 10:Utils::SetClantag("        owo       "); break;
				case 11:Utils::SetClantag("       owo        "); break;
				case 12:Utils::SetClantag("      owo         "); break;
				case 13:Utils::SetClantag("     owo          "); break;
				case 14:Utils::SetClantag("    owo           "); break;
				case 15:Utils::SetClantag("   owo            "); break;
				case 16:Utils::SetClantag("  owo             "); break;
				case 17:Utils::SetClantag(" owo              "); break;
				case 18:Utils::SetClantag("owo               "); break;
				case 19:Utils::SetClantag("wo                "); break;
				case 20:Utils::SetClantag("o                 "); break;
				case 21:Utils::SetClantag("                  "); break;

				}
				counter++;
			}

			if (g_Options.clantag == STARS)
			{
				static int counter = 0;
				static int motion = 0;
				int serverTime = g_GlobalVars->interval_per_tick * (float)g_LocalPlayer->m_nTickBase() * 2.5;

				if (counter % 48 == 0)
					motion++;

				int value = serverTime % 46;

				switch (value)
				{
				case 0:
					Utils::SetClantag("★☆☆☆☆");
					break;
				case 1:
					Utils::SetClantag("☆★☆☆☆");
					break;
				case 2:
					Utils::SetClantag("☆☆★☆☆");
					break;
				case 3:
					Utils::SetClantag("☆☆☆★☆");
					break;
				case 4:
					Utils::SetClantag("☆☆☆☆★");
					break;
				case 5:
					Utils::SetClantag("☆☆☆★☆");
					break;
				case 6:
					Utils::SetClantag("☆☆★☆☆");
					break;
				case 7:
					Utils::SetClantag("☆★☆☆☆");
					break;
				case 8:
					Utils::SetClantag("★☆☆☆☆");
					break;
				}
				counter++;
			}

			if (g_Options.clantag == ANIMIATED_XD)
			{
				static int counter = 0;
				static int motion = 0;
				int serverTime = g_GlobalVars->interval_per_tick * (float)g_LocalPlayer->m_nTickBase() * 2.5;

				if (counter % 48 == 0)
					motion++;

				int value = serverTime % 35;

				switch (value)
				{
				case 0:
					Utils::SetClantag("- Elimination");
					break;
				case 1:
					Utils::SetClantag("\ Elimination");
					break;
				case 2:
					Utils::SetClantag("/ Elimination");
					break;
				case 3:
					Utils::SetClantag("- Elimination");
					break;

				}
				counter++;
			}


			switch (g_Options.player_model) 
			{
			case No_Model:
				if (g_Options.legit_aimspot == Head)
					aimspot = 8;
				if (g_Options.legit_aimspot == Neck)
					aimspot = 7;
				if (g_Options.legit_aimspot == Chest)
					aimspot = 6;
				if (g_Options.legit_aimspot == Stomach)
					aimspot = 4;
				break;
			default:
				if (g_Options.legit_aimspot == Head)
					aimspot = 6;
				if (g_Options.legit_aimspot == Neck)
					aimspot = 5;
				if (g_Options.legit_aimspot == Chest)
					aimspot = 4;
				if (g_Options.legit_aimspot == Stomach)
					aimspot = 2;
				break;
			}

			auto weapon = g_LocalPlayer->m_hActiveWeapon();
			if (weapon) 
			{
				if (weapon->IsPistol()) g_Options.legit_aimspot = g_Options.pistol_legit_aimspot;
				else if (weapon->IsRifle()) g_Options.legit_aimspot = g_Options.rifle_legit_aimspot;
				else if (weapon->IsSniper()) g_Options.legit_aimspot = g_Options.sniper_legit_aimspot;
				else if (weapon->IsShotgun()) g_Options.legit_aimspot = g_Options.shotgun_legit_aimspot;
				else if (weapon->IsSMG()) g_Options.legit_aimspot = g_Options.smg_legit_aimspot;
			}		

			//// Third person, need to set engine viewangles 
			//thirdperson::FrameStageNotify();

			if (g_Options.noflash == true)
				g_LocalPlayer->m_flFlashDuration() = 0;

			// setting antiaim view angles
			if (pLocal->IsAlive())
			{
				if (!ExSendPacket && g_Options.antiaim_thirdperson_angle == ANTIAIM_THIRDPERSON_REAL)
				{
					if (*(bool*)((DWORD)g_Input + 0xA5))
						*(QAngle*)((DWORD)pLocal + 0x31C8) = LastTickViewAngles;
				}
				else if (ExSendPacket && g_Options.antiaim_thirdperson_angle == ANTIAIM_THIRDPERSON_FAKE)
				{
					if (*(bool*)((DWORD)g_Input + 0xA5))
						*(QAngle*)((DWORD)pLocal + 0x31C8) = LastTickViewAngles;
				}
				else if (g_Options.antiaim_thirdperson_angle == ANTIAIM_THIRDPERSON_BOTH)
				{
					if (*(bool*)((DWORD)g_Input + 0xA5))
						*(QAngle*)((DWORD)pLocal + 0x31C8) = LastTickViewAngles;
				}
			}

			static QAngle old_aim_punch_angle;
			static QAngle old_view_punch_angle;

			// resolver
			Resolver::OnFrameStageNotify(stage);

			//skinchanger::Applyknife();
		}

		// skinchanger full update
		// only do it once per press
		static bool bbbbbbbb = false;
		if (InputSys::Get().IsKeyDown(VK_F4) && !bbbbbbbb)
		{
			g_ClientState->ForceFullUpdate();
			bbbbbbbb = true;
		}
		else if (!InputSys::Get().IsKeyDown(VK_F4) && bbbbbbbb) bbbbbbbb = false;

		// skinchanger
		//if (g_EngineClient->IsInGame() && g_EngineClient->IsConnected() && stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		//{
		//	static int GloveT = g_MdlInfo->GetModelIndex("models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl");
		//	static int GloveCT = g_MdlInfo->GetModelIndex("models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl");
		//	static int GloveBloodhound = g_MdlInfo->GetModelIndex("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl");
		//	static int GloveSport = g_MdlInfo->GetModelIndex("models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl");
		//	static int GloveSlick = g_MdlInfo->GetModelIndex("models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl");
		//	static int GloveLeatherWrap = g_MdlInfo->GetModelIndex("models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl");
		//	static int GloveMoto = g_MdlInfo->GetModelIndex("models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl");
		//	static int GloveSpecialist = g_MdlInfo->GetModelIndex("models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl");

		//	// wearables
		//	DWORD* hMyWearables = (DWORD*)((size_t)pLocal + 0x2EF4);

		//	// destroying gloves on death
		//	if (pLocal->m_lifeState() != LIFE_ALIVE && g_EngineClient->IsInGame())
		//	{
		//		C_BaseAttributableItem* Gloves = (C_BaseAttributableItem*)g_EntityList->GetClientNetworkable(hMyWearables[0] & 0xFFF);
		//		if (Gloves)
		//		{
		//			Gloves->SetDestroyedOnRecreateEntities();
		//			Gloves->Release();
		//		}
		//	}

		//	// Creating the gloves if the player is alive, is connected, and is in game
		//	if (g_EngineClient->IsInGame() && g_EngineClient->IsConnected() && pLocal->m_lifeState() == LIFE_ALIVE)
		//	{
		//		auto hWeapons = pLocal->m_hMyWeapons();

		//		static bool bUpdate = false;

		//		if (hMyWearables)
		//		{
		//			for (ClientClass* pClass = g_CHLClient->GetAllClasses(); pClass; pClass = pClass->m_pNext)
		//			{
		//				if (pClass->m_ClassID != ClassId_CEconWearable)
		//					continue;

		//				int iEntry = (g_EntityList->GetHighestEntityIndex() + 1);
		//				int	iSerial = RandomInt(0x0, 0xFFF);

		//				pClass->m_pCreateFn(iEntry, iSerial);
		//				hMyWearables[0] = iEntry | (iSerial << 16);

		//				bUpdate = true;
		//				break;
		//			}

		//			player_info_t LocalPlayerInfo;
		//			g_EngineClient->GetPlayerInfo(pLocal->EntIndex(), &LocalPlayerInfo);

		//			auto pWeapon = (C_BaseAttributableItem*)g_EntityList->GetClientEntity(hMyWearables[0] & 0xFFF);

		//			if (pWeapon)
		//			{
		//				if (bUpdate)
		//				{
		//					pWeapon->m_nFallbackPaintKit() = g_Options.glove_skin;
		//					pWeapon->m_iEntityQuality() = 4;
		//					pWeapon->m_nFallbackSeed() = 1337;
		//					pWeapon->m_nFallbackStatTrak() = -1;
		//					pWeapon->m_flFallbackWear() = 0.00000001f;

		//					if (g_Options.glove == GLOVE_DEFAULT) {

		//						if (pLocal->m_iTeamNum() == 0) {
		//							pWeapon->m_iItemDefinitionIndex() = 5029;
		//							pWeapon->m_nModelIndex() = GloveCT;
		//						}

		//						else if (pLocal->m_iTeamNum() == 1) {
		//							pWeapon->m_iItemDefinitionIndex() = 5028;
		//							pWeapon->m_nModelIndex() = GloveCT;
		//						}
		//					}

		//					if (g_Options.glove == GLOVE_BLOODHOUND)
		//							pWeapon->m_iItemDefinitionIndex() = studded_bloodhound_gloves;
		//							pWeapon->m_nModelIndex() = GloveBloodhound;

		//					if (g_Options.glove == GLOVE_SPORT)
		//							pWeapon->m_iItemDefinitionIndex() = sporty_gloves;
		//							pWeapon->m_nModelIndex() = GloveSport;

		//					if (g_Options.glove == GLOVE_SLICK)
		//							pWeapon->m_iItemDefinitionIndex() = slick_gloves;
		//							pWeapon->m_nModelIndex() = GloveSport;

		//					if (g_Options.glove == GLOVE_LEATHERWRAP)
		//							pWeapon->m_iItemDefinitionIndex() = leather_handwraps;
		//							pWeapon->m_nModelIndex() = GloveLeatherWrap;

		//					if (g_Options.glove == GLOVE_MOTO)
		//							pWeapon->m_iItemDefinitionIndex() = motorcycle_gloves;
		//							pWeapon->m_nModelIndex() = GloveMoto;

		//					if (g_Options.glove == GLOVE_SPECIALIST)
		//							pWeapon->m_iItemDefinitionIndex() = specialist_gloves;
		//							pWeapon->m_nModelIndex() = GloveSpecialist;

		//					//switch (g_Options.glove)
		//					//{
		//					//case GLOVE_DEFAULT:
		//					//	// ct
		//					//	if (pLocal->m_iTeamNum() == 1) pWeapon->m_iItemDefinitionIndex() = 5029;
		//					//	// t
		//					//	else if (pLocal->m_iTeamNum() == 0) pWeapon->m_iItemDefinitionIndex() = 5028;
		//					//	break;

		//					//case GLOVE_BLOODHOUND: pWeapon->m_iItemDefinitionIndex() = studded_bloodhound_gloves; break;
		//					//case GLOVE_SPORT: pWeapon->m_iItemDefinitionIndex() = sporty_gloves; break;
		//					//case GLOVE_SLICK: pWeapon->m_iItemDefinitionIndex() = slick_gloves; break;
		//					//case GLOVE_LEATHERWRAP: pWeapon->m_iItemDefinitionIndex() = leather_handwraps; break;
		//					//case GLOVE_MOTO: pWeapon->m_iItemDefinitionIndex() = motorcycle_gloves; break;
		//					//case GLOVE_SPECIALIST: pWeapon->m_iItemDefinitionIndex() = specialist_gloves; break;
		//					//}

		//					//pWeapon->m_iItemIDHigh() = -1;

		//					//switch (g_Options.glove)
		//					//{
		//					//case GLOVE_DEFAULT:
		//					//	// ct
		//					//	if (pLocal->m_iTeamNum() == 1) pWeapon->m_nModelIndex() = GloveCT;
		//					//	// t
		//					//	else if (pLocal->m_iTeamNum() == 0) pWeapon->m_nModelIndex() = GloveT;
		//					//	break;

		//					//case GLOVE_BLOODHOUND: pWeapon->m_nModelIndex() = GloveBloodhound; break;
		//					//case GLOVE_SPORT: pWeapon->m_nModelIndex() = GloveSport; break;
		//					//case GLOVE_SLICK: pWeapon->m_nModelIndex() = GloveSlick; break;
		//					//case GLOVE_LEATHERWRAP: pWeapon->m_nModelIndex() = GloveLeatherWrap; break;
		//					//case GLOVE_MOTO: pWeapon->m_nModelIndex() = GloveMoto; break;
		//					//case GLOVE_SPECIALIST: pWeapon->m_nModelIndex() = GloveSpecialist; break;
		//					//}

		//					auto pValid = g_EntityList->GetClientEntity(pWeapon->EntIndex());
		//					if (pValid) pWeapon->PreDataUpdate(0);

		//					pWeapon->m_OriginalOwnerXuidLow() = LocalPlayerInfo.xuid_low;

		//					// console spam fix
		//					*reinterpret_cast<int*>(uintptr_t(pWeapon) + 0x64) = -1;

		//					bUpdate = false;
		//				}
		//			}
		//		}


				if (g_EngineClient->IsInGame() && g_EngineClient->IsConnected() && stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
				{

				auto hWeapons = pLocal->m_hMyWeapons();
				// skin changer
				if (hWeapons)
				{
					static char* KnifeCT = "models/weapons/v_knife_ct.mdl";
					static char* KnifeT = "models/weapons/v_knife_t.mdl";
					static char* Bayonet = "models/weapons/v_knife_bayonet.mdl";
					static char* Butterfly = "models/weapons/v_knife_butterfly.mdl";
					static char* Flip = "models/weapons/v_knife_flip.mdl";
					static char* Gut = "models/weapons/v_knife_gut.mdl";
					static char* Karambit = "models/weapons/v_knife_karam.mdl";
					static char* M9Bayonet = "models/weapons/v_knife_m9_bay.mdl";
					static char* Huntsman = "models/weapons/v_knife_tactical.mdl";
					static char* Falchion = "models/weapons/v_knife_falchion_advanced.mdl";
					static char* Dagger = "models/weapons/v_knife_push.mdl";
					static char* Bowie = "models/weapons/v_knife_survival_bowie.mdl";

					// go through all weapons
					for (int i = 0; hWeapons[i]; i++)
					{
						// Get the weapon entity from the handle.
						auto pWeapon = (C_BaseAttributableItem*)g_EntityList->GetClientEntityFromHandle(hWeapons[i]);
						int pWeaponType = ((C_BaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(hWeapons[i]))->GetCSWeaponData()->WeaponType;

						if (!pWeapon)
							continue;

						// Knife changer if holding a knife
						if (pWeaponType == WEAPONTYPE_KNIFE)
						{
							// model indexes
							switch (g_Options.knife)
							{
							case KNIFE_DEFAULT:
								if (pLocal->m_iTeamNum() == 0) pWeapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(KnifeT);
								if (pLocal->m_iTeamNum() == 1) pWeapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(KnifeCT);
								break;
							case KNIFE_BAYONET:
								pWeapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Bayonet);
								break;
							case KNIFE_FLIP:
								pWeapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Flip);
								break;
							case KNIFE_GUT:
								pWeapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Gut);
								break;
							case KNIFE_KARAMBIT:
								pWeapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Karambit);
								break;
							case KNIFE_M9BAYONET:
								pWeapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(M9Bayonet);
								break;
							case KNIFE_HUNTSMAN:
								pWeapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Huntsman);
								break;
							case KNIFE_FALCHION:
								pWeapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Falchion);
								break;
							case KNIFE_BOWIE:
								pWeapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Bowie);
								break;
							case KNIFE_BUTTERFLY:
								pWeapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Butterfly);
								break;
							case KNIFE_PUSHDAGGER:
								pWeapon->m_nModelIndex() = g_MdlInfo->GetModelIndex(Dagger);
								break;
							}

							// changine knife model
							switch (g_Options.knife)
							{
							case KNIFE_DEFAULT:
								if (pLocal->m_iTeamNum() == 0) ApplyCustomModel(pWeapon, KnifeT);
								if (pLocal->m_iTeamNum() == 1) ApplyCustomModel(pWeapon, KnifeCT);
								break;
							case KNIFE_BAYONET:
								ApplyCustomModel(pWeapon, Bayonet);
								break;
							case KNIFE_FLIP:
								ApplyCustomModel(pWeapon, Flip);
								break;
							case KNIFE_GUT:
								ApplyCustomModel(pWeapon, Gut);
								break;
							case KNIFE_KARAMBIT:
								ApplyCustomModel(pWeapon, Karambit);
								break;
							case KNIFE_M9BAYONET:
								ApplyCustomModel(pWeapon, M9Bayonet);
								break;
							case KNIFE_HUNTSMAN:
								ApplyCustomModel(pWeapon, Huntsman);
								break;
							case KNIFE_FALCHION:
								ApplyCustomModel(pWeapon, Falchion);
								break;
							case KNIFE_BOWIE:
								ApplyCustomModel(pWeapon, Bowie);
								break;
							case KNIFE_BUTTERFLY:
								ApplyCustomModel(pWeapon, Butterfly);
								break;
							case KNIFE_PUSHDAGGER:
								ApplyCustomModel(pWeapon, Dagger);
								break;
							}

							// changing knife skin
							switch (g_Options.knife)
							{
							case KNIFE_DEFAULT:
								if (pLocal->m_iTeamNum() == 0) ApplyCustomSkin(pWeapon, weapon_knife_t, 0);
								if (pLocal->m_iTeamNum() == 1) ApplyCustomSkin(pWeapon, weapon_knife_ct, 0);
								break;
							case KNIFE_BAYONET:
								switch (g_Options.knife_skin)
								{
								case DEFAULTKNIFE:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 0);
									break;
								case Safari_MeshKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 72);
									break;
								case Boreal_ForestKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 77);
									break;
								case Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 418);
									break;
								case Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 419);
									break;
								case Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 420);
									break;
								case Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 421);
									break;
								case RubyKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 415);
									break;
								case SapphireKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 416);
									break;
								case Black_PearlKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 418);
									break;
								case SlaughterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 59);
									break;
								case FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 38);
									break;
								case Crimson_WebKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 12);
									break;
								case NightKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 40);
									break;
								case Blue_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 42);
									break;
								case StainedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 43);
									break;
								case Case_HardenedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 44);
									break;
								case UltravioletKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 98);
									break;
								case Urban_MaskedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 143);
									break;
								case Damascus_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 410);
									break;
								case ScorchedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 175);
									break;
								case Bright_WaterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 578);
									break;
								case EmeraldKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 568);
									break;
								case Gamma_Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 569);
									break;
								case Gamma_Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 570);
									break;
								case Gamma_Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 571);
									break;
								case Gamma_Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 572);
									break;
								case FreehandKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 582);
									break;
								case Tiger_ToothKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 409);
									break;
								case Rust_CoatKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 414);
									break;
								case Marble_FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 413);
									break;
								case AutotronicKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 576);
									break;
								case Black_LaminateKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 566);
									break;
								case LoreKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 561);
									break;
								case Forest_DDPATKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bayonet, 5);
									break;
								}
								break;
							case KNIFE_BOWIE:
								switch (g_Options.knife_skin)
								{
								case DEFAULTKNIFE:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 0);
									break;
								case Safari_MeshKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 72);
									break;
								case Boreal_ForestKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 77);
									break;
								case Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 418);
									break;
								case Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 419);
									break;
								case Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 420);
									break;
								case Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 421);
									break;
								case RubyKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 415);
									break;
								case SapphireKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 416);
									break;
								case Black_PearlKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 418);
									break;
								case SlaughterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 59);
									break;
								case FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 38);
									break;
								case Crimson_WebKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 12);
									break;
								case NightKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 40);
									break;
								case Blue_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 42);
									break;
								case StainedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 43);
									break;
								case Case_HardenedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 44);
									break;
								case UltravioletKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 98);
									break;
								case Urban_MaskedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 143);
									break;
								case Damascus_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 410);
									break;
								case ScorchedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 175);
									break;
								case Bright_WaterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 578);
									break;
								case EmeraldKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 568);
									break;
								case Gamma_Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 569);
									break;
								case Gamma_Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 570);
									break;
								case Gamma_Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 571);
									break;
								case Gamma_Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 572);
									break;
								case FreehandKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 582);
									break;
								case Tiger_ToothKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 409);
									break;
								case Rust_CoatKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 414);
									break;
								case Marble_FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 413);
									break;
								case AutotronicKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 576);
									break;
								case Black_LaminateKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 566);
									break;
								case LoreKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 561);
									break;
								case Forest_DDPATKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_bowie, 5);
									break;
								}
								break;
							case KNIFE_PUSHDAGGER:
								switch (g_Options.knife_skin)
								{
								case DEFAULTKNIFE:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 0);
									break;
								case Safari_MeshKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 72);
									break;
								case Boreal_ForestKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 77);
									break;
								case Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 418);
									break;
								case Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 419);
									break;
								case Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 420);
									break;
								case Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 421);
									break;
								case RubyKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 415);
									break;
								case SapphireKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 416);
									break;
								case Black_PearlKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 418);
									break;
								case SlaughterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 59);
									break;
								case FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 38);
									break;
								case Crimson_WebKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 12);
									break;
								case NightKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 40);
									break;
								case Blue_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 42);
									break;
								case StainedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 43);
									break;
								case Case_HardenedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 44);
									break;
								case UltravioletKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 98);
									break;
								case Urban_MaskedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 143);
									break;
								case Damascus_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 410);
									break;
								case ScorchedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 175);
									break;
								case Bright_WaterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 578);
									break;
								case EmeraldKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 568);
									break;
								case Gamma_Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 569);
									break;
								case Gamma_Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 570);
									break;
								case Gamma_Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 571);
									break;
								case Gamma_Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 572);
									break;
								case FreehandKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 582);
									break;
								case Tiger_ToothKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 409);
									break;
								case Rust_CoatKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 414);
									break;
								case Marble_FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 413);
									break;
								case AutotronicKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 576);
									break;
								case Black_LaminateKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 566);
									break;
								case LoreKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 561);
									break;
								case Forest_DDPATKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_pushdagger, 5);
									break;
								}
								break;
							case KNIFE_BUTTERFLY:
								switch (g_Options.knife_skin)
								{
								case DEFAULTKNIFE:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 0);
									break;
								case Safari_MeshKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 72);
									break;
								case Boreal_ForestKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 77);
									break;
								case Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 418);
									break;
								case Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 419);
									break;
								case Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 420);
									break;
								case Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 421);
									break;
								case RubyKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 415);
									break;
								case SapphireKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 416);
									break;
								case Black_PearlKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 418);
									break;
								case SlaughterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 59);
									break;
								case FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 38);
									break;
								case Crimson_WebKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 12);
									break;
								case NightKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 40);
									break;
								case Blue_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 42);
									break;
								case StainedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 43);
									break;
								case Case_HardenedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 44);
									break;
								case UltravioletKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 98);
									break;
								case Urban_MaskedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 143);
									break;
								case Damascus_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 410);
									break;
								case ScorchedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 175);
									break;
								case Bright_WaterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 578);
									break;
								case EmeraldKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 568);
									break;
								case Gamma_Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 569);
									break;
								case Gamma_Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 570);
									break;
								case Gamma_Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 571);
									break;
								case Gamma_Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 572);
									break;
								case FreehandKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 582);
									break;
								case Tiger_ToothKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 409);
									break;
								case Rust_CoatKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 414);
									break;
								case Marble_FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 413);
									break;
								case AutotronicKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 576);
									break;
								case Black_LaminateKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 566);
									break;
								case LoreKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 561);
									break;
								case Forest_DDPATKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_butterfly, 5);
									break;
								}
								break;
							case KNIFE_FALCHION:
								switch (g_Options.knife_skin)
								{
								case DEFAULTKNIFE:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 0);
									break;
								case Safari_MeshKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 72);
									break;
								case Boreal_ForestKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 77);
									break;
								case Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 418);
									break;
								case Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 419);
									break;
								case Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 420);
									break;
								case Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 421);
									break;
								case RubyKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 415);
									break;
								case SapphireKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 416);
									break;
								case Black_PearlKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 418);
									break;
								case SlaughterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 59);
									break;
								case FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 38);
									break;
								case Crimson_WebKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 12);
									break;
								case NightKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 40);
									break;
								case Blue_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 42);
									break;
								case StainedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 43);
									break;
								case Case_HardenedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 44);
									break;
								case UltravioletKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 98);
									break;
								case Urban_MaskedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 143);
									break;
								case Damascus_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 410);
									break;
								case ScorchedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 175);
									break;
								case Bright_WaterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 578);
									break;
								case EmeraldKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 568);
									break;
								case Gamma_Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 569);
									break;
								case Gamma_Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 570);
									break;
								case Gamma_Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 571);
									break;
								case Gamma_Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 572);
									break;
								case FreehandKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 582);
									break;
								case Tiger_ToothKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 409);
									break;
								case Rust_CoatKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 414);
									break;
								case Marble_FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 413);
									break;
								case AutotronicKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 576);
									break;
								case Black_LaminateKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 566);
									break;
								case LoreKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 561);
									break;
								case Forest_DDPATKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_falchion, 5);
									break;
								}
								break;
							case KNIFE_FLIP:
								switch (g_Options.knife_skin)
								{
								case DEFAULTKNIFE:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 0);
									break;
								case Safari_MeshKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 72);
									break;
								case Boreal_ForestKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 77);
									break;
								case Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 418);
									break;
								case Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 419);
									break;
								case Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 420);
									break;
								case Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 421);
									break;
								case RubyKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 415);
									break;
								case SapphireKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 416);
									break;
								case Black_PearlKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 418);
									break;
								case SlaughterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 59);
									break;
								case FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 38);
									break;
								case Crimson_WebKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 12);
									break;
								case NightKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 40);
									break;
								case Blue_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 42);
									break;
								case StainedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 43);
									break;
								case Case_HardenedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 44);
									break;
								case UltravioletKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 98);
									break;
								case Urban_MaskedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 143);
									break;
								case Damascus_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 410);
									break;
								case ScorchedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 175);
									break;
								case Bright_WaterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 578);
									break;
								case EmeraldKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 568);
									break;
								case Gamma_Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 569);
									break;
								case Gamma_Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 570);
									break;
								case Gamma_Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 571);
									break;
								case Gamma_Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 572);
									break;
								case FreehandKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 582);
									break;
								case Tiger_ToothKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 409);
									break;
								case Rust_CoatKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 414);
									break;
								case Marble_FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 413);
									break;
								case AutotronicKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 576);
									break;
								case Black_LaminateKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 566);
									break;
								case LoreKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 561);
									break;
								case Forest_DDPATKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_flip, 5);
									break;
								}
								break;
							case KNIFE_GUT:
								switch (g_Options.knife_skin)
								{
								case DEFAULTKNIFE:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 0);
									break;
								case Safari_MeshKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 72);
									break;
								case Boreal_ForestKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 77);
									break;
								case Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 418);
									break;
								case Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 419);
									break;
								case Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 420);
									break;
								case Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 421);
									break;
								case RubyKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 415);
									break;
								case SapphireKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 416);
									break;
								case Black_PearlKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 418);
									break;
								case SlaughterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 59);
									break;
								case FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 38);
									break;
								case Crimson_WebKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 12);
									break;
								case NightKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 40);
									break;
								case Blue_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 42);
									break;
								case StainedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 43);
									break;
								case Case_HardenedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 44);
									break;
								case UltravioletKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 98);
									break;
								case Urban_MaskedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 143);
									break;
								case Damascus_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 410);
									break;
								case ScorchedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 175);
									break;
								case Bright_WaterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 578);
									break;
								case EmeraldKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 568);
									break;
								case Gamma_Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 569);
									break;
								case Gamma_Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 570);
									break;
								case Gamma_Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 571);
									break;
								case Gamma_Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 572);
									break;
								case FreehandKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 582);
									break;
								case Tiger_ToothKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 409);
									break;
								case Rust_CoatKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 414);
									break;
								case Marble_FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 413);
									break;
								case AutotronicKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 576);
									break;
								case Black_LaminateKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 566);
									break;
								case LoreKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 561);
									break;
								case Forest_DDPATKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_gut, 5);
									break;
								}
								break;
							case KNIFE_KARAMBIT:
								switch (g_Options.knife_skin)
								{
								case DEFAULTKNIFE:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 0);
									break;
								case Safari_MeshKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 72);
									break;
								case Boreal_ForestKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 77);
									break;
								case Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 418);
									break;
								case Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 419);
									break;
								case Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 420);
									break;
								case Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 421);
									break;
								case RubyKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 415);
									break;
								case SapphireKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 416);
									break;
								case Black_PearlKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 418);
									break;
								case SlaughterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 59);
									break;
								case FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 38);
									break;
								case Crimson_WebKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 12);
									break;
								case NightKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 40);
									break;
								case Blue_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 42);
									break;
								case StainedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 43);
									break;
								case Case_HardenedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 44);
									break;
								case UltravioletKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 98);
									break;
								case Urban_MaskedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 143);
									break;
								case Damascus_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 410);
									break;
								case ScorchedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 175);
									break;
								case Bright_WaterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 578);
									break;
								case EmeraldKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 568);
									break;
								case Gamma_Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 569);
									break;
								case Gamma_Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 570);
									break;
								case Gamma_Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 571);
									break;
								case Gamma_Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 572);
									break;
								case FreehandKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 582);
									break;
								case Tiger_ToothKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 409);
									break;
								case Rust_CoatKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 414);
									break;
								case Marble_FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 413);
									break;
								case AutotronicKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 576);
									break;
								case Black_LaminateKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 566);
									break;
								case LoreKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 561);
									break;
								case Forest_DDPATKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_karambit, 5);
									break;
								}
								break;
							case KNIFE_M9BAYONET:
								switch (g_Options.knife_skin)
								{
								case DEFAULTKNIFE:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 0);
									break;
								case Safari_MeshKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 72);
									break;
								case Boreal_ForestKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 77);
									break;
								case Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 418);
									break;
								case Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 419);
									break;
								case Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 420);
									break;
								case Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 421);
									break;
								case RubyKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 415);
									break;
								case SapphireKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 416);
									break;
								case Black_PearlKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 418);
									break;
								case SlaughterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 59);
									break;
								case FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 38);
									break;
								case Crimson_WebKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 12);
									break;
								case NightKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 40);
									break;
								case Blue_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 42);
									break;
								case StainedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 43);
									break;
								case Case_HardenedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 44);
									break;
								case UltravioletKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 98);
									break;
								case Urban_MaskedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 143);
									break;
								case Damascus_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 410);
									break;
								case ScorchedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 175);
									break;
								case Bright_WaterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 578);
									break;
								case EmeraldKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 568);
									break;
								case Gamma_Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 569);
									break;
								case Gamma_Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 570);
									break;
								case Gamma_Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 571);
									break;
								case Gamma_Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 572);
									break;
								case FreehandKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 582);
									break;
								case Tiger_ToothKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 409);
									break;
								case Rust_CoatKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 414);
									break;
								case Marble_FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 413);
									break;
								case AutotronicKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 576);
									break;
								case Black_LaminateKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 566);
									break;
								case LoreKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 561);
									break;
								case Forest_DDPATKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_m9bayonet, 5);
									break;
								}
								break;
							case KNIFE_HUNTSMAN:
								switch (g_Options.knife_skin)
								{
								case DEFAULTKNIFE:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 0);
									break;
								case Safari_MeshKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 72);
									break;
								case Boreal_ForestKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 77);
									break;
								case Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 418);
									break;
								case Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 419);
									break;
								case Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 420);
									break;
								case Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 421);
									break;
								case RubyKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 415);
									break;
								case SapphireKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 416);
									break;
								case Black_PearlKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 418);
									break;
								case SlaughterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 59);
									break;
								case FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 38);
									break;
								case Crimson_WebKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 12);
									break;
								case NightKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 40);
									break;
								case Blue_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 42);
									break;
								case StainedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 43);
									break;
								case Case_HardenedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 44);
									break;
								case UltravioletKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 98);
									break;
								case Urban_MaskedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 143);
									break;
								case Damascus_SteelKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 410);
									break;
								case ScorchedKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 175);
									break;
								case Bright_WaterKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 578);
									break;
								case EmeraldKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 568);
									break;
								case Gamma_Doppler_Phase_1Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 569);
									break;
								case Gamma_Doppler_Phase_2Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 570);
									break;
								case Gamma_Doppler_Phase_3Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 571);
									break;
								case Gamma_Doppler_Phase_4Knife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 572);
									break;
								case FreehandKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 582);
									break;
								case Tiger_ToothKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 409);
									break;
								case Rust_CoatKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 414);
									break;
								case Marble_FadeKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 413);
									break;
								case AutotronicKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 576);
									break;
								case Black_LaminateKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 566);
									break;
								case LoreKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 561);
									break;
								case Forest_DDPATKnife:
									ApplyCustomKnifeSkin(pWeapon, weapon_huntsman, 5);
									break;
								}
								pWeapon->m_hOwnerEntity() = g_LocalPlayer;
								break;
							}
							// stattrak fix
							break;
						}
					}


					for (int i = 0; hWeapons[i]; i++)
					{
						// Get the weapon entity from the handle.
						auto pWeapon = (C_BaseAttributableItem*)g_EntityList->GetClientEntityFromHandle(hWeapons[i]);
						int pWeaponType = ((C_BaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(hWeapons[i]))->GetCSWeaponData()->WeaponType;

						if (!pWeapon)
							continue;
						// weapons skins
						switch (pWeapon->m_iItemDefinitionIndex())
						{
						case weapon_deagle:
							switch (g_Options.deagle)
							{
							case DEFAULTDEAGLE:
								ApplyCustomSkin(pWeapon, weapon_deagle, 0);
								break;
							case Blaze:
								ApplyCustomSkin(pWeapon, weapon_deagle, 37);
								break;
							case Pilotdeagle:
								ApplyCustomSkin(pWeapon, weapon_deagle, 347);
								break;
							case Midnight_Storm:
								ApplyCustomSkin(pWeapon, weapon_deagle, 468);
								break;
							case Urban_DDPATdeagle:
								ApplyCustomSkin(pWeapon, weapon_deagle, 17);
								break;
							case Nightdeagle:
								ApplyCustomSkin(pWeapon, weapon_deagle, 40);
								break;
							case Hypnotic:
								ApplyCustomSkin(pWeapon, weapon_deagle, 61);
								break;
							case Mudderdeagle:
								ApplyCustomSkin(pWeapon, weapon_deagle, 90);
								break;
							case Golden_Koi:
								ApplyCustomSkin(pWeapon, weapon_deagle, 185);
								break;
							case Cobalt_Disruption:
								ApplyCustomSkin(pWeapon, weapon_deagle, 231);
								break;
							case Crimson_Webdeagle:
								ApplyCustomSkin(pWeapon, weapon_deagle, 232);
								break;
							case Urban_Rubbledeagle:
								ApplyCustomSkin(pWeapon, weapon_deagle, 237);
								break;
							case Naga:
								ApplyCustomSkin(pWeapon, weapon_deagle, 397);
								break;
							case Hand_Cannon:
								ApplyCustomSkin(pWeapon, weapon_deagle, 328);
								break;
							case Heirloom:
								ApplyCustomSkin(pWeapon, weapon_deagle, 273);
								break;
							case Meteorite:
								ApplyCustomSkin(pWeapon, weapon_deagle, 296);
								break;
							case Kumicho_Dragon:
								ApplyCustomSkin(pWeapon, weapon_deagle, 527);
								break;
							case Conspiracy:
								ApplyCustomSkin(pWeapon, weapon_deagle, 351);
								break;
							case Bronze_Decodeagle:
								ApplyCustomSkin(pWeapon, weapon_deagle, 425);
								break;
							case Sunset_Storm:
								ApplyCustomSkin(pWeapon, weapon_deagle, 470);
								break;
							case Directive:
								ApplyCustomSkin(pWeapon, weapon_deagle, 603);
								break;
							case Oxide_Blaze:
								ApplyCustomSkin(pWeapon, weapon_deagle, 645);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_elite:
							switch (g_Options.elites)
							{
							case DEFAULTDUALIES:
								ApplyCustomSkin(pWeapon, weapon_elite, 0);
								break;
							case Anodized_Navyelites:
								ApplyCustomSkin(pWeapon, weapon_elite, 28);
								break;
							case Stainedelites:
								ApplyCustomSkin(pWeapon, weapon_elite, 43);
								break;
							case Contractorelites:
								ApplyCustomSkin(pWeapon, weapon_elite, 46);
								break;
							case Colonyelites:
								ApplyCustomSkin(pWeapon, weapon_elite, 47);
								break;
							case Demolition:
								ApplyCustomSkin(pWeapon, weapon_elite, 153);
								break;
							case Dualing_Dragons:
								ApplyCustomSkin(pWeapon, weapon_elite, 491);
								break;
							case Black_Limba:
								ApplyCustomSkin(pWeapon, weapon_elite, 190);
								break;
							case Cobalt_Quartzelites:
								ApplyCustomSkin(pWeapon, weapon_elite, 249);
								break;
							case Hemoglobin:
								ApplyCustomSkin(pWeapon, weapon_elite, 220);
								break;
							case Urban_Shock:
								ApplyCustomSkin(pWeapon, weapon_elite, 396);
								break;
							case Marina:
								ApplyCustomSkin(pWeapon, weapon_elite, 261);
								break;
							case Panther:
								ApplyCustomSkin(pWeapon, weapon_elite, 276);
								break;
							case Retribution:
								ApplyCustomSkin(pWeapon, weapon_elite, 307);
								break;
							case Briar:
								ApplyCustomSkin(pWeapon, weapon_elite, 330);
								break;
							case Duelist:
								ApplyCustomSkin(pWeapon, weapon_elite, 447);
								break;
							case Moon_in_Libraelites:
								ApplyCustomSkin(pWeapon, weapon_elite, 450);
								break;
							case Cartelelites:
								ApplyCustomSkin(pWeapon, weapon_elite, 528);
								break;
							case Ventilators:
								ApplyCustomSkin(pWeapon, weapon_elite, 544);
								break;
							case Royal_Consorts:
								ApplyCustomSkin(pWeapon, weapon_elite, 625);
								break;
							case Cobra_Strike:
								ApplyCustomSkin(pWeapon, weapon_elite, 658);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;

						case weapon_fiveseven:
							switch (g_Options.fiveseven)
							{
							case DEFAULTFIVESEVEN:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 0);
								break;
							case Candy_Apple57:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 3);
								break;
							case Case_Hardened57:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 44);
								break;
							case Contractor57:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 46);
								break;
							case Forest_Night:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 78);
								break;
							case Orange_Peel57:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 141);
								break;
							case Jungle:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 151);
								break;
							case Anodized_Gunmetal:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 210);
								break;
							case Nightshade:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 223);
								break;
							case Silver_Quartz:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 252);
								break;
							case Nitro57:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 254);
								break;
							case Kami57:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 265);
								break;
							case Copper_Galaxy:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 274);
								break;
							case Fowl_Play:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 352);
								break;
							case Hot_Shot:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 377);
								break;
							case Urban_Hazard:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 387);
								break;
							case Monkey_Buisness:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 427);
								break;
							case Neon_Kimono:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 464);
								break;
							case Orange_Kimono57:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 465);
								break;
							case Crimson_Kimono:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 466);
								break;
							case Retrobution:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 510);
								break;
							case Trimuvirate:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 530);
								break;
							case Violent_Daimyo:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 585);
								break;
							case Scumbria57:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 605);
								break;
							case Capillary:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 646);
								break;
							case Hyper_Beast57:
								ApplyCustomSkin(pWeapon, weapon_fiveseven, 660);
								pWeapon->m_hOwnerEntity() = g_LocalPlayer;
								break;
							}
							break;
						case weapon_glock:
							switch (g_Options.glock)
							{
							case DEFAULT_GLOCKSKIN:
								ApplyCustomSkin(pWeapon, weapon_glock, 0);
								break;
							case Groundwater:
								ApplyCustomSkin(pWeapon, weapon_glock, 2);
								break;
							case Candy_Apple:
								ApplyCustomSkin(pWeapon, weapon_glock, 3);
								break;
							case Fade:
								ApplyCustomSkin(pWeapon, weapon_glock, 38);
								break;
							case Night:
								ApplyCustomSkin(pWeapon, weapon_glock, 40);
								break;
							case Dragon_Tattoo:
								ApplyCustomSkin(pWeapon, weapon_glock, 48);
								break;
							case Twilight_Galaxy:
								ApplyCustomSkin(pWeapon, weapon_glock, 437);
								break;
							case Brass:
								ApplyCustomSkin(pWeapon, weapon_glock, 159);
								break;
							case Catacombs:
								ApplyCustomSkin(pWeapon, weapon_glock, 399);
								break;
							case Wraiths:
								ApplyCustomSkin(pWeapon, weapon_glock, 495);
								break;
							case Wasteland_RebelGlock:
								ApplyCustomSkin(pWeapon, weapon_glock, 586);
								break;
							case Sand_DuneGlock:
								ApplyCustomSkin(pWeapon, weapon_glock, 208);
								break;
							case Steel_Disruption:
								ApplyCustomSkin(pWeapon, weapon_glock, 230);
								break;
							case Blue_Fissure:
								ApplyCustomSkin(pWeapon, weapon_glock, 278);
								break;
							case Death_Rattle:
								ApplyCustomSkin(pWeapon, weapon_glock, 293);
								break;
							case Water_Elemental:
								ApplyCustomSkin(pWeapon, weapon_glock, 353);
								break;
							case Reactor:
								ApplyCustomSkin(pWeapon, weapon_glock, 367);
								break;
							case Grinder:
								ApplyCustomSkin(pWeapon, weapon_glock, 381);
								break;
							case Bunsen_Burner:
								ApplyCustomSkin(pWeapon, weapon_glock, 479);
								break;
							case Royal_Legion:
								ApplyCustomSkin(pWeapon, weapon_glock, 532);
								break;
							case WeaselGlock:
								ApplyCustomSkin(pWeapon, weapon_glock, 607);
								break;
							case Ironwork:
								ApplyCustomSkin(pWeapon, weapon_glock, 623);
								break;
							case Off_World:
								ApplyCustomSkin(pWeapon, weapon_glock, 680);
								break;
							}
								pWeapon->m_hOwnerEntity() = g_LocalPlayer;
								break;
							
						case weapon_ak47:
							switch (g_Options.ak47)
							{
							case DEFAULT_AK47SKIN:
								ApplyCustomSkin(pWeapon, weapon_ak47, 0);
								break;
							case First_Class:
								ApplyCustomSkin(pWeapon, weapon_ak47, 341);
								break;
							case Red_Laminate:
								ApplyCustomSkin(pWeapon, weapon_ak47, 14);
								break;
							case Case_Hardened:
								ApplyCustomSkin(pWeapon, weapon_ak47, 44);
								break;
							case Safari_Mesh:
								ApplyCustomSkin(pWeapon, weapon_ak47, 72);
								break;
							case Jungle_Spray:
								ApplyCustomSkin(pWeapon, weapon_ak47, 122);
								break;
							case Predator:
								ApplyCustomSkin(pWeapon, weapon_ak47, 170);
								break;
							case Black_Laminate:
								ApplyCustomSkin(pWeapon, weapon_ak47, 172);
								break;
							case Fire_Serpent:
								ApplyCustomSkin(pWeapon, weapon_ak47, 180);
								break;
							case Frontside_Misty:
								ApplyCustomSkin(pWeapon, weapon_ak47, 490);
								break;
							case Cartel:
								ApplyCustomSkin(pWeapon, weapon_ak47, 394);
								break;
							case Emerald_Pinstripe:
								ApplyCustomSkin(pWeapon, weapon_ak47, 300);
								break;
							case Point_Disarray:
								ApplyCustomSkin(pWeapon, weapon_ak47, 506);
								break;
							case Blue_Laminate:
								ApplyCustomSkin(pWeapon, weapon_ak47, 226);
								break;
							case Redline:
								ApplyCustomSkin(pWeapon, weapon_ak47, 282);
								break;
							case Vulcan:
								ApplyCustomSkin(pWeapon, weapon_ak47, 302);
								break;
							case Jaguar:
								ApplyCustomSkin(pWeapon, weapon_ak47, 316);
								break;
							case Jet_Set:
								ApplyCustomSkin(pWeapon, weapon_ak47, 340);
								break;
							case Fuel_Injector:
								ApplyCustomSkin(pWeapon, weapon_ak47, 524);
								break;
							case Wasteland_Rebel:
								ApplyCustomSkin(pWeapon, weapon_ak47, 380);
								break;
							case Elite_Build:
								ApplyCustomSkin(pWeapon, weapon_ak47, 422);
								break;
							case Hydroponic:
								ApplyCustomSkin(pWeapon, weapon_ak47, 456);
								break;
							case Aquamarine_Revenge:
								ApplyCustomSkin(pWeapon, weapon_ak47, 474);
								break;
							case Neon_Revolution:
								ApplyCustomSkin(pWeapon, weapon_ak47, 600);
								break;
							case BloodsportAK:
								ApplyCustomSkin(pWeapon, weapon_ak47, 639);
								break;
							case Orbit_Mk01:
								ApplyCustomSkin(pWeapon, weapon_ak47, 656);
								break;
							case The_Empress:
								ApplyCustomSkin(pWeapon, weapon_ak47, 675);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_aug:
							switch (g_Options.aug) {
							case DEFAULTAUG:
								ApplyCustomSkin(pWeapon, weapon_aug, 0);
								break;
							case Wings:
								ApplyCustomSkin(pWeapon, weapon_aug, 73);
								break;
							case Copperhead:
								ApplyCustomSkin(pWeapon, weapon_aug, 10);
								break;
							case Bengal_Tiger:
								ApplyCustomSkin(pWeapon, weapon_aug, 9);
								break;
							case Condemned:
								ApplyCustomSkin(pWeapon, weapon_aug, 110);
								break;
							case Hot_Rod:
								ApplyCustomSkin(pWeapon, weapon_aug, 33);
								break;
							case Storm:
								ApplyCustomSkin(pWeapon, weapon_aug, 100);
								break;
							case Contractor:
								ApplyCustomSkin(pWeapon, weapon_aug, 46);
								break;
							case ColonyAug:
								ApplyCustomSkin(pWeapon, weapon_aug, 47);
								break;
							case Aristocrat:
								ApplyCustomSkin(pWeapon, weapon_aug, 583);
								break;
							case Anodized_NavyAug:
								ApplyCustomSkin(pWeapon, weapon_aug, 197);
								break;
							case Ricochet:
								ApplyCustomSkin(pWeapon, weapon_aug, 507);
								break;
							case Chameleon:
								ApplyCustomSkin(pWeapon, weapon_aug, 280);
								break;
							case TorqueAug:
								ApplyCustomSkin(pWeapon, weapon_aug, 305);
								break;
							case Radiation_HazardAug:
								ApplyCustomSkin(pWeapon, weapon_aug, 375);
								break;
							case Daedalus:
								ApplyCustomSkin(pWeapon, weapon_aug, 444);
								break;
							case Akihabara_Accept:
								ApplyCustomSkin(pWeapon, weapon_aug, 455);
								break;
							case Fleet_Flock:
								ApplyCustomSkin(pWeapon, weapon_aug, 541);
								break;
							case Syd_Mead:
								ApplyCustomSkin(pWeapon, weapon_aug, 601);
								break;
							case Triqua:
								ApplyCustomSkin(pWeapon, weapon_aug, 674);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_awp:
							switch (g_Options.awp) {
							case DEFAULTAWP:
								ApplyCustomSkin(pWeapon, weapon_awp, 0);
								break;
							case BOOM:
								ApplyCustomSkin(pWeapon, weapon_awp, 174);
								break;
							case Dragon_Lore:
								ApplyCustomSkin(pWeapon, weapon_awp, 344);
								break;
							case Pink_DDPAT:
								ApplyCustomSkin(pWeapon, weapon_awp, 84);
								break;
							case Elite_BuildAWP:
								ApplyCustomSkin(pWeapon, weapon_awp, 525);
								break;
							case Snake_Camo:
								ApplyCustomSkin(pWeapon, weapon_awp, 30);
								break;
							case Lightning_Strike:
								ApplyCustomSkin(pWeapon, weapon_awp, 51);
								break;
							case Safari_MeshAWP:
								ApplyCustomSkin(pWeapon, weapon_awp, 72);
								break;
							case Corticera:
								ApplyCustomSkin(pWeapon, weapon_awp, 181);
								break;
							case RedlineAWP:
								ApplyCustomSkin(pWeapon, weapon_awp, 259);
								break;
							case Man_o_war:
								ApplyCustomSkin(pWeapon, weapon_awp, 395);
								break;
							case Phobos:
								ApplyCustomSkin(pWeapon, weapon_awp, 584);
								break;
							case Graphite:
								ApplyCustomSkin(pWeapon, weapon_awp, 212);
								break;
							case Electric_Hive:
								ApplyCustomSkin(pWeapon, weapon_awp, 227);
								break;
							case Pit_Viper:
								ApplyCustomSkin(pWeapon, weapon_awp, 251);
								break;
							case AsiimovAWP:
								ApplyCustomSkin(pWeapon, weapon_awp, 279);
								break;
							case Worm_God:
								ApplyCustomSkin(pWeapon, weapon_awp, 424);
								break;
							case Medusa:
								ApplyCustomSkin(pWeapon, weapon_awp, 446);
								break;
							case Sun_in_Leo:
								ApplyCustomSkin(pWeapon, weapon_awp, 451);
								break;
							case Hyper_BeastAWP:
								ApplyCustomSkin(pWeapon, weapon_awp, 475);
								break;
							case Fever_Dream:
								ApplyCustomSkin(pWeapon, weapon_awp, 640);
								break;
							case Oni_Taiji:
								ApplyCustomSkin(pWeapon, weapon_awp, 662);
								break;
							}
								pWeapon->m_hOwnerEntity() = g_LocalPlayer;
								break;				
						case weapon_famas:
							switch (g_Options.famas) {
							case DEFAULT_FAMAS:
								ApplyCustomSkin(pWeapon, weapon_famas, 0);
								break;
							case Contrast_Spray:
								ApplyCustomSkin(pWeapon, weapon_famas, 22);
								break;
							case Colony:
								ApplyCustomSkin(pWeapon, weapon_famas, 47);
								break;
							case Cyanospatter:
								ApplyCustomSkin(pWeapon, weapon_famas, 92);
								break;
							case Djinn:
								ApplyCustomSkin(pWeapon, weapon_famas, 429);
								break;
							case Afterimage:
								ApplyCustomSkin(pWeapon, weapon_famas, 154);
								break;
							case Doomkitty:
								ApplyCustomSkin(pWeapon, weapon_famas, 178);
								break;
							case Survivor_Z:
								ApplyCustomSkin(pWeapon, weapon_famas, 492);
								break;
							case Spitfire:
								ApplyCustomSkin(pWeapon, weapon_famas, 194);
								break;
							case Teardown:
								ApplyCustomSkin(pWeapon, weapon_famas, 244);
								break;
							case Hexane:
								ApplyCustomSkin(pWeapon, weapon_famas, 218);
								break;
							case PulseFamas:
								ApplyCustomSkin(pWeapon, weapon_famas, 260);
								break;
							case Sergeant:
								ApplyCustomSkin(pWeapon, weapon_famas, 288);
								break;
							case Styx:
								ApplyCustomSkin(pWeapon, weapon_famas, 371);
								break;
							case Valence:
								ApplyCustomSkin(pWeapon, weapon_famas, 529);
								break;
							case Neural_Net:
								ApplyCustomSkin(pWeapon, weapon_famas, 477);
								break;
							case Roll_Cage:
								ApplyCustomSkin(pWeapon, weapon_famas, 604);
								break;
							case Mecha_IndustriesFamas:
								ApplyCustomSkin(pWeapon, weapon_famas, 626);
								break;
							case Macabre:
								ApplyCustomSkin(pWeapon, weapon_famas, 659);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_g3sg1:
							switch (g_Options.g3sg1)
							{
							case DEFAULTG3:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 0);
								break;
							case Desert_Stormg3:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 8);
								break;
							case Arctic_Camo:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 6);
								break;
							case Contractorg3:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 46);
								break;
							case Safari_Meshg3:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 72);
								break;
							case Polar_Camo:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 74);
								break;
							case Jungle_Dashedg3:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 147);
								break;
							case VariCamog3:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 235);
								break;
							case Flux:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 493);
								break;
							case Demeter:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 195);
								break;
							case Azure_Zebra:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 229);
								break;
							case Green_Apple:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 294);
								break;
							case Orange_Kimonog3:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 465);
								break;
							case Murky:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 382);
								break;
							case Chronos:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 438);
								break;
							case The_Executioner:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 511);
								break;
							case Orange_Crash:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 545);
								break;
							case Ventilator:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 606);
								break;
							case Stingerg3s:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 628);
								break;
							case Hunterg3:
								ApplyCustomSkin(pWeapon, weapon_g3sg1, 677);
								break;
							}

							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_galilar:
							switch (g_Options.galil)
							{
							case Default_GALILSKIN:
								ApplyCustomSkin(pWeapon, weapon_galilar, 0);
								break;
							case Orange_DDPAT:
								ApplyCustomSkin(pWeapon, weapon_galilar, 83);
								break;
							case Eco:
								ApplyCustomSkin(pWeapon, weapon_galilar, 428);
								break;
							case Winter_Forest:
								ApplyCustomSkin(pWeapon, weapon_galilar, 76);
								break;
							case Sage_Spray:
								ApplyCustomSkin(pWeapon, weapon_galilar, 119);
								break;
							case VariCamo:
								ApplyCustomSkin(pWeapon, weapon_galilar, 235);
								break;
							case Chatterbox:
								ApplyCustomSkin(pWeapon, weapon_galilar, 398);
								break;
							case Stone_Cold:
								ApplyCustomSkin(pWeapon, weapon_galilar, 494);
								break;
							case Shattered:
								ApplyCustomSkin(pWeapon, weapon_galilar, 192);
								break;
							case Kami:
								ApplyCustomSkin(pWeapon, weapon_galilar, 308);
								break;
							case Blue_Titanium:
								ApplyCustomSkin(pWeapon, weapon_galilar, 216);
								break;
							case Urban_Rubble:
								ApplyCustomSkin(pWeapon, weapon_galilar, 237);
								break;
							case Hunting_Blind:
								ApplyCustomSkin(pWeapon, weapon_galilar, 241);
								break;
							case Sandstorm:
								ApplyCustomSkin(pWeapon, weapon_galilar, 264);
								break;
							case Tuxedo:
								ApplyCustomSkin(pWeapon, weapon_galilar, 297);
								break;
							case Cerberus:
								ApplyCustomSkin(pWeapon, weapon_galilar, 379);
								break;
							case Aqua_Terrace:
								ApplyCustomSkin(pWeapon, weapon_galilar, 460);
								break;
							case Rocket_Pop:
								ApplyCustomSkin(pWeapon, weapon_galilar, 478);
								break;
							case Firefight:
								ApplyCustomSkin(pWeapon, weapon_galilar, 546);
								break;
							case Black_Sand:
								ApplyCustomSkin(pWeapon, weapon_galilar, 629);
								break;
							case Crimson_Tsunami:
								ApplyCustomSkin(pWeapon, weapon_galilar, 647);
								break;
							case sugar_rush:
								ApplyCustomSkin(pWeapon, weapon_galilar, 661);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;

						case weapon_m249:
							switch (g_Options.m249)
							{
							case DEFAULTM249:
								ApplyCustomSkin(pWeapon, weapon_m249, 0);
								break;
							case Contrast_Spraym249:
								ApplyCustomSkin(pWeapon, weapon_m249, 22);
								break;
							case Blizzard_Marbleized:
								ApplyCustomSkin(pWeapon, weapon_m249, 75);
								break;
							case Nebula_Crusader:
								ApplyCustomSkin(pWeapon, weapon_m249, 496);
								break;
							case Jungle_DDPAT:
								ApplyCustomSkin(pWeapon, weapon_m249, 202);
								break;
							case Gator_Meshm249:
								ApplyCustomSkin(pWeapon, weapon_m249, 243);
								break;
							case Magma:
								ApplyCustomSkin(pWeapon, weapon_m249, 266);
								break;
							case System_Lock:
								ApplyCustomSkin(pWeapon, weapon_m249, 401);
								break;
							case Shipping_Forecast:
								ApplyCustomSkin(pWeapon, weapon_m249, 452);
								break;
							case Impact_Drill:
								ApplyCustomSkin(pWeapon, weapon_m249, 472);
								break;
							case Spectre:
								ApplyCustomSkin(pWeapon, weapon_m249, 547);
								break;
							case Emerald_Poison_Dart:
								ApplyCustomSkin(pWeapon, weapon_m249, 648);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_m4a4:
							switch (g_Options.m4a4)
							{
							case DEFAULT_M4A4:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 0);
								break;
							case Desert_Storm:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 8);
								break;
							case Tornadom4a4:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 101);
								break;
							case Radiation_Hazard:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 167);
								break;
							case Jungle_Tiger:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 16);
								break;
							case Modern_Hunter:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 164);
								break;
							case Urban_DDPAT:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 17);
								break;
							case Bullet_Rain:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 155);
								break;
							case Faded_Zebra:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 176);
								break;
							case Zirka:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 187);
								break;
							case Asiimov:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 255);
								break;
							case Howl:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 309);
								break;
							case X_Ray:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 215);
								break;
							case Desert_Strike:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 336);
								break;
							case Desolate_Space:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 588);
								break;
							case Griffin:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 384);
								break;
							case Dragon_King:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 400);
								break;
							case Poseidon:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 449);
								break;
							case Daybreak:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 471);
								break;
							case Evil_Daimyo:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 480);
								break;
							case Royal_Paladin:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 512);
								break;
							case The_Battlestar:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 533);
								break;
							case Buzz_Kill:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 632);
								break;
							case Hell_Fire:
								ApplyCustomSkin(pWeapon, weapon_m4a4, 664);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_mac10:
							switch (g_Options.mac10)
							{
							case DEFAULTMAC10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 0);
								break;
							case Tornadomac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 101);
								break;
							case Candy_Applemac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 3);
								break;
							case Silvermac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 32);
								break;
							case Urban_DDPATmac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 17);
								break;
							case Fademac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 38);
								break;
							case Neon_Ridermac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 433);
								break;
							case Ultravioletmac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 98);
								break;
							case Palmmac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 157);
								break;
							case Gravenmac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 188);
								break;
							case Tattermac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 337);
								break;
							case Carnivoremac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 589);
								break;
							case Amber_Fademac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 246);
								break;
							case Rangeenmac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 498);
								break;
							case Heatmac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 284);
								break;
							case Cursemac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 310);
								break;
							case Indigomac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 333);
								break;
							case Commutermac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 343);
								break;
							case Lapis_Gatormac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 534);
								break;
							case Nuclear_Gardenmac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 372);
								break;
							case Malachitemac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 402);
								break;
							case Last_Dive:
								ApplyCustomSkin(pWeapon, weapon_mac10, 651);
								break;
							case Aloha:
								ApplyCustomSkin(pWeapon, weapon_mac10, 665);
								break;
							case Oceanicmac10:
								ApplyCustomSkin(pWeapon, weapon_mac10, 682);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_p90:
							switch (g_Options.p90)
							{
							case DEFAULTP99:
								ApplyCustomSkin(pWeapon, weapon_p90, 0);
								break;
							case Leatherp90:
								ApplyCustomSkin(pWeapon, weapon_p90, 342);
								break;
							case Virusp90:
								ApplyCustomSkin(pWeapon, weapon_p90, 20);
								break;
							case Stormp90:
								ApplyCustomSkin(pWeapon, weapon_p90, 100);
								break;
							case Cold_Bloodedp90:
								ApplyCustomSkin(pWeapon, weapon_p90, 67);
								break;
							case Glacier_Meshp90:
								ApplyCustomSkin(pWeapon, weapon_p90, 111);
								break;
							case Sand_Sprayp90:
								ApplyCustomSkin(pWeapon, weapon_p90, 124);
								break;
							case Death_by_Kittyp90:
								ApplyCustomSkin(pWeapon, weapon_p90, 156);
								break;
							case Ash_Woodp90:
								ApplyCustomSkin(pWeapon, weapon_p90, 234);
								break;
							case Fallout_Warningp90:
								ApplyCustomSkin(pWeapon, weapon_p90, 169);
								break;
							case Scorchedp90:
								ApplyCustomSkin(pWeapon, weapon_p90, 175);
								break;
							case Emerald_Dragonp90:
								ApplyCustomSkin(pWeapon, weapon_p90, 182);
								break;
							case Teardownp90:
								ApplyCustomSkin(pWeapon, weapon_p90, 244);
								break;
							case Blind_Spotp90:
								ApplyCustomSkin(pWeapon, weapon_p90, 228);
								break;
							case Trigonp90:
								ApplyCustomSkin(pWeapon, weapon_p90, 283);
								break;
							case Desert_Warfarep90:
								ApplyCustomSkin(pWeapon, weapon_p90, 311);
								break;
							case Modulep90:
								ApplyCustomSkin(pWeapon, weapon_p90, 335);
								break;
							case Asiimovp90:
								ApplyCustomSkin(pWeapon, weapon_p90, 359);
								break;
							case Shapewoodp90:
								ApplyCustomSkin(pWeapon, weapon_p90, 516);
								break;
							case Elite_Buildp90:
								ApplyCustomSkin(pWeapon, weapon_p90, 486);
								break;
							case Chopperp90:
								ApplyCustomSkin(pWeapon, weapon_p90, 593);
								break;
							case Grim:
								ApplyCustomSkin(pWeapon, weapon_p90, 611);
								break;
							case Shallow_Grave:
								ApplyCustomSkin(pWeapon, weapon_p90, 636);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_ump45:
							switch (g_Options.ump45)
							{
							case DEFAULTUMP:
								ApplyCustomSkin(pWeapon, weapon_ump45, 0);
								break;
							case Blazeump:
								ApplyCustomSkin(pWeapon, weapon_ump45, 37);
								break;
							case Gunsmokeump:
								ApplyCustomSkin(pWeapon, weapon_ump45, 15);
								break;
							case Urban_DDPATump:
								ApplyCustomSkin(pWeapon, weapon_ump45, 17);
								break;
							case Carbon_Fiberump:
								ApplyCustomSkin(pWeapon, weapon_ump45, 70);
								break;
							case Grand_Prixump:
								ApplyCustomSkin(pWeapon, weapon_ump45, 436);
								break;
							case Caramelump:
								ApplyCustomSkin(pWeapon, weapon_ump45, 93);
								break;
							case Fallout_Warningump:
								ApplyCustomSkin(pWeapon, weapon_ump45, 169);
								break;
							case Scorchedump:
								ApplyCustomSkin(pWeapon, weapon_ump45, 175);
								break;
							case Bone_Pileump:
								ApplyCustomSkin(pWeapon, weapon_ump45, 193);
								break;
							case Delusionump:
								ApplyCustomSkin(pWeapon, weapon_ump45, 392);
								break;
							case Corporalump:
								ApplyCustomSkin(pWeapon, weapon_ump45, 281);
								break;
							case Indigoump:
								ApplyCustomSkin(pWeapon, weapon_ump45, 333);
								break;
							case Labyrinthump:
								ApplyCustomSkin(pWeapon, weapon_ump45, 362);
								break;
							case Minotaurs_Labyrinthump:
								ApplyCustomSkin(pWeapon, weapon_ump45, 441);
								break;
							case Riotump:
								ApplyCustomSkin(pWeapon, weapon_ump45, 488);
								break;
							case Primal_Saberump:
								ApplyCustomSkin(pWeapon, weapon_ump45, 556);
								break;
							case BriefingUMP:
								ApplyCustomSkin(pWeapon, weapon_ump45, 615);
								break;
							case Scaffold:
								ApplyCustomSkin(pWeapon, weapon_ump45, 652);
								break;
							case Metal_Flowers:
								ApplyCustomSkin(pWeapon, weapon_ump45, 672);
								break;
							case Exposure:
								ApplyCustomSkin(pWeapon, weapon_ump45, 688);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_xm1014:
							switch (g_Options.xm1014)
							{
							case DEFAULTXM:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 0);
								break;
							case Teclu_Burner:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 521);
								break;
							case Blaze_Orange:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 166);
								break;
							case VariCamo_Blue:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 238);
								break;
							case Blue_Steel:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 42);
								break;
							case Blue_SpruceXM:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 96);
								break;
							case Grassland:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 95);
								break;
							case Urban_PerforatedXM:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 135);
								break;
							case Fallout_WarningXM:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 169);
								break;
							case JungleXM:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 205);
								break;
							case Scumbria:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 505);
								break;
							case CaliCamoXM:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 240);
								break;
							case Tranquility:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 393);
								break;
							case Red_Python:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 320);
								break;
							case Heaven_Guard:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 314);
								break;
							case Red_Leather:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 348);
								break;
							case Bone_Machine:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 370);
								break;
							case Quicksilver:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 407);
								break;
							case Black_Tie:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 557);
								break;
							case Slipstream:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 616);
								break;
							case Seasons:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 654);
								break;
							case Ziggy:
								ApplyCustomSkin(pWeapon, weapon_xm1014, 689);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_bizon:
							switch (g_Options.bizon)
							{
							case DEFAULTPPBIZON:
								ApplyCustomSkin(pWeapon, weapon_bizon, 0);
								break;
							case Photic_Zonebizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 526);
								break;
							case Blue_Streakbizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 13);
								break;
							case Modern_Hunterbizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 164);
								break;
							case Forest_Leavesbizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 25);
								break;
							case Carbon_Fiberbizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 70);
								break;
							case Sand_Dashedbizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 148);
								break;
							case Urban_Dashedbizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 149);
								break;
							case Brassbizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 159);
								break;
							case Irradiated_Alertbizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 171);
								break;
							case Rust_Coatbizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 203);
								break;
							case Water_Sigilbizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 224);
								break;
							case Night_Opsbizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 236);
								break;
							case Cobalt_Halftonebizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 267);
								break;
							case Harvesterbizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 594);
								break;
							case Antiquebizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 306);
								break;
							case Osirisbizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 349);
								break;
							case Chemical_Greenbizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 376);
								break;
							case Fuel_Rodbizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 508);
								break;
							case Bamboo_Printbizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 457);
								break;
							case Judgement_of_Anubisbizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 542);
								break;
							case Jungle_Slipstreambizon:
								ApplyCustomSkin(pWeapon, weapon_bizon, 611);
								break;
							case High_Roller:
								ApplyCustomSkin(pWeapon, weapon_bizon, 636);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_mag7:
							switch (g_Options.mag7)
							{
							case DEFAULTNEGEV:
								ApplyCustomSkin(pWeapon, weapon_mag7, 0);
								break;
							case Counter_Terracemag7:
								ApplyCustomSkin(pWeapon, weapon_mag7, 462);
								break;
							case Metallic_DDPATmag7:
								ApplyCustomSkin(pWeapon, weapon_mag7, 34);
								break;
							case Silvermag7:
								ApplyCustomSkin(pWeapon, weapon_mag7, 32);
								break;
							case Stormmag7:
								ApplyCustomSkin(pWeapon, weapon_mag7, 100);
								break;
							case Bulldozermag7:
								ApplyCustomSkin(pWeapon, weapon_mag7, 39);
								break;
							case Heatmag7:
								ApplyCustomSkin(pWeapon, weapon_mag7, 431);
								break;
							case Sand_Dunemag7:
								ApplyCustomSkin(pWeapon, weapon_mag7, 99);
								break;
							case Irradiated_Alertmag7:
								ApplyCustomSkin(pWeapon, weapon_mag7, 171);
								break;
							case Mementomag7:
								ApplyCustomSkin(pWeapon, weapon_mag7, 177);
								break;
							case Hazardmag7:
								ApplyCustomSkin(pWeapon, weapon_mag7, 198);
								break;
							case Cobalt_Coremag7:
								ApplyCustomSkin(pWeapon, weapon_mag7, 499);
								break;
							case Heaven_Guardmag7:
								ApplyCustomSkin(pWeapon, weapon_mag7, 291);
								break;
							case Praetorianmag7:
								ApplyCustomSkin(pWeapon, weapon_mag7, 535);
								break;
							case Firestartermag7:
								ApplyCustomSkin(pWeapon, weapon_mag7, 385);
								break;
							case Seabirdmag7:
								ApplyCustomSkin(pWeapon, weapon_mag7, 473);
								break;
							case Petroglyph:
								ApplyCustomSkin(pWeapon, weapon_mag7, 608);
								break;
							case Sonar:
								ApplyCustomSkin(pWeapon, weapon_mag7, 633);
								break;
							case Hard_Water:
								ApplyCustomSkin(pWeapon, weapon_mag7, 666);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_negev:
							switch (g_Options.negev)
							{
							case DEFAULTNEGEV:
								ApplyCustomSkin(pWeapon, weapon_negev, 0);
								break;
							case Anodized_NavyNegev:
								ApplyCustomSkin(pWeapon, weapon_negev, 28);
								break;
							case Man_o_warNegev:
								ApplyCustomSkin(pWeapon, weapon_negev, 432);
								break;
							case PalmNegev:
								ApplyCustomSkin(pWeapon, weapon_negev, 201);
								break;
							case CaliCamo:
								ApplyCustomSkin(pWeapon, weapon_negev, 240);
								break;
							case TerrainNegev:
								ApplyCustomSkin(pWeapon, weapon_negev, 285);
								break;
							case Army_SheenNegev:
								ApplyCustomSkin(pWeapon, weapon_negev, 298);
								break;
							case Bratatat:
								ApplyCustomSkin(pWeapon, weapon_negev, 317);
								break;
							case Desert_StrikeNegev:
								ApplyCustomSkin(pWeapon, weapon_negev, 355);
								break;
							case Nuclear_Waste:
								ApplyCustomSkin(pWeapon, weapon_negev, 369);
								break;
							case Loudmouth:
								ApplyCustomSkin(pWeapon, weapon_negev, 483);
								break;
							case Power_Loader:
								ApplyCustomSkin(pWeapon, weapon_negev, 514);
								break;
							case Dazzle:
								ApplyCustomSkin(pWeapon, weapon_negev, 610);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_sawedoff:
							switch (g_Options.sawedoff) {
							case DEFAULTSAWEDOFF:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 0);
								break;
							case FirstClass:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 345);
								break;
							case ForestDDPAT:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 5);
								break;
							case SnakeCamo:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 30);
								break;
							case OrangeDDPAT:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 83);
								break;
							case Copper:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 41);
								break;
							case Origami:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 434);
								break;
							case SageSpray:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 119);
								break;
							case IrradiatedAlert:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 171);
								break;
							case Mosaico:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 204);
								break;
							case Serenity:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 405);
								break;
							case AmberFade:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 246);
								break;
							case FullStop:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 250);
								break;
							case Highwayman:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 390);
								break;
							case TheKraken:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 256);
								break;
							case RustCoat:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 323);
								break;
							case BambooShadow:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 458);
								break;
							case Yorick:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 517);
								break;
							case Fubar:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 552);
								break;
							case Limelight:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 596);
								break;
							case Wasteland_Princess:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 638);
								break;
							case Zander:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 655);
								break;
							case Morris:
								ApplyCustomSkin(pWeapon, weapon_sawedoff, 673);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_tec9:
							switch (g_Options.tec9) {
							case DEFAULT_TEC9:
								ApplyCustomSkin(pWeapon, weapon_tec9, 0);
								break;
							case GroundwaterTEC9:
								ApplyCustomSkin(pWeapon, weapon_tec9, 2);
								break;
							case AvalancheTEC9:
								ApplyCustomSkin(pWeapon, weapon_tec9, 520);
								break;
							case Terrace:
								ApplyCustomSkin(pWeapon, weapon_tec9, 463);
								break;
							case Urban_DDPATTEC9:
								ApplyCustomSkin(pWeapon, weapon_tec9, 17);
								break;
							case Ossified:
								ApplyCustomSkin(pWeapon, weapon_tec9, 36);
								break;
							case Hades:
								ApplyCustomSkin(pWeapon, weapon_tec9, 439);
								break;
							case Brasstec:
								ApplyCustomSkin(pWeapon, weapon_tec9, 159);
								break;
							case VariCamoTEC9:
								ApplyCustomSkin(pWeapon, weapon_tec9, 235);
								break;
							case Nuclear_Threat:
								ApplyCustomSkin(pWeapon, weapon_tec9, 179);
								break;
							case Red_Quartz:
								ApplyCustomSkin(pWeapon, weapon_tec9, 248);
								break;
							case TornadoTEC9:
								ApplyCustomSkin(pWeapon, weapon_tec9, 206);
								break;
							case Blue_TitaniumTEC9:
								ApplyCustomSkin(pWeapon, weapon_tec9, 216);
								break;
							case Army_MeshTEC9:
								ApplyCustomSkin(pWeapon, weapon_tec9, 242);
								break;
							case Titanium_Bit:
								ApplyCustomSkin(pWeapon, weapon_tec9, 272);
								break;
							case SandstormTEC9:
								ApplyCustomSkin(pWeapon, weapon_tec9, 289);
								break;
							case IsaacTEC9:
								ApplyCustomSkin(pWeapon, weapon_tec9, 303);
								break;
							case Jambiya:
								ApplyCustomSkin(pWeapon, weapon_tec9, 539);
								break;
							case Toxic:
								ApplyCustomSkin(pWeapon, weapon_tec9, 374);
								break;
							case Bamboo_Forest:
								ApplyCustomSkin(pWeapon, weapon_tec9, 459);
								break;
							case Re_Entry:
								ApplyCustomSkin(pWeapon, weapon_tec9, 555);
								break;
							case Ice_Cap:
								ApplyCustomSkin(pWeapon, weapon_tec9, 599);
								break;
							case Fuel_Injectortec:
								ApplyCustomSkin(pWeapon, weapon_tec9, 614);
								break;
							case Cut_Out:
								ApplyCustomSkin(pWeapon, weapon_tec9, 671);
								break;
							case Cracked_Opal:
								ApplyCustomSkin(pWeapon, weapon_tec9, 684);
								break;
							}
								pWeapon->m_hOwnerEntity() = g_LocalPlayer;
								break;
						case weapon_p2000:
							switch (g_Options.p2000) {
							case Defaultp2000:
								ApplyCustomSkin(pWeapon, weapon_p2000, 0);
								break;
							case Silverp2000:
								ApplyCustomSkin(pWeapon, weapon_p2000, 32);
								break;
							case Grassland_Leavesp2000:
								ApplyCustomSkin(pWeapon, weapon_p2000, 104);
								break;
							case Granite_Marbleized:
								ApplyCustomSkin(pWeapon, weapon_p2000, 21);
								break;
							case Handgun:
								ApplyCustomSkin(pWeapon, weapon_p2000, 485);
								break;
							case Scorpionp2000:
								ApplyCustomSkin(pWeapon, weapon_p2000, 71);
								break;
							case Grasslandp2000:
								ApplyCustomSkin(pWeapon, weapon_p2000, 95);
								break;
							case Corticerap2000:
								ApplyCustomSkin(pWeapon, weapon_p2000, 184);
								break;
							case Ocean_Foam:
								ApplyCustomSkin(pWeapon, weapon_p2000, 211);
								break;
							case Pulsep2000:
								ApplyCustomSkin(pWeapon, weapon_p2000, 338);
								break;
							case Amber_Fadep2000:
								ApplyCustomSkin(pWeapon, weapon_p2000, 246);
								break;
							case Red_FragCam:
								ApplyCustomSkin(pWeapon, weapon_p2000, 275);
								break;
							case Chainmailp2000:
								ApplyCustomSkin(pWeapon, weapon_p2000, 327);
								break;
							case Coach_Class:
								ApplyCustomSkin(pWeapon, weapon_p2000, 346);
								break;
							case Ivory:
								ApplyCustomSkin(pWeapon, weapon_p2000, 357);
								break;
							case Fire_Elemental:
								ApplyCustomSkin(pWeapon, weapon_p2000, 389);
								break;
							case Pathfinder:
								ApplyCustomSkin(pWeapon, weapon_p2000, 443);
								break;
							case Imperial:
								ApplyCustomSkin(pWeapon, weapon_p2000, 515);
								break;
							case Oceanic:
								ApplyCustomSkin(pWeapon, weapon_p2000, 550);
								break;
							case Imperial_Dragon:
								ApplyCustomSkin(pWeapon, weapon_p2000, 591);
								break;
							case Turf:
								ApplyCustomSkin(pWeapon, weapon_p2000, 635);
								break;
							case Woodsman:
								ApplyCustomSkin(pWeapon, weapon_p2000, 667);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_mp7:
							switch (g_Options.mp7) {
							case DEFAULTMP7:
								ApplyCustomSkin(pWeapon, weapon_mp7, 0);
								break;
							case Whiteoutmp7:
								ApplyCustomSkin(pWeapon, weapon_mp7, 102);
								break;
							case Forest_DDPATmp7:
								ApplyCustomSkin(pWeapon, weapon_mp7, 5);
								break;
							case Anodized_Navymp7:
								ApplyCustomSkin(pWeapon, weapon_mp7, 28);
								break;
							case Skullsmp7:
								ApplyCustomSkin(pWeapon, weapon_mp7, 11);
								break;
							case Gunsmokemp7:
								ApplyCustomSkin(pWeapon, weapon_mp7, 15);
								break;
							case Orange_Peelmp7:
								ApplyCustomSkin(pWeapon, weapon_mp7, 141);
								break;
							case Army_Reconmp7:
								ApplyCustomSkin(pWeapon, weapon_mp7, 245);
								break;
							case Groundwatermp7:
								ApplyCustomSkin(pWeapon, weapon_mp7, 209);
								break;
							case Ocean_Foammp7:
								ApplyCustomSkin(pWeapon, weapon_mp7, 213);
								break;
							case Special_Deliverymp7:
								ApplyCustomSkin(pWeapon, weapon_mp7, 500);
								break;
							case Full_Stopmp7:
								ApplyCustomSkin(pWeapon, weapon_mp7, 250);
								break;
							case Urban_Hazardmp7:
								ApplyCustomSkin(pWeapon, weapon_mp7, 354);
								break;
							case Olive_Plaidmp7:
								ApplyCustomSkin(pWeapon, weapon_mp7, 365);
								break;
							case Armor_Coremp7:
								ApplyCustomSkin(pWeapon, weapon_mp7, 423);
								break;
							case Asterionmp7:
								ApplyCustomSkin(pWeapon, weapon_mp7, 442);
								break;
							case Nemesismp7:
								ApplyCustomSkin(pWeapon, weapon_mp7, 481);
								break;
							case Impiremp7:
								ApplyCustomSkin(pWeapon, weapon_mp7, 536);
								break;
							case Cirrus:
								ApplyCustomSkin(pWeapon, weapon_mp7, 627);
								break;
							case Akoben:
								ApplyCustomSkin(pWeapon, weapon_mp7, 649);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_mp9:
							switch (g_Options.mp9) {
							case DEFAULTMP9:
								ApplyCustomSkin(pWeapon, weapon_mp9, 0);
								break;
							case Ruby_Poison_Dartmp9:
								ApplyCustomSkin(pWeapon, weapon_mp9, 482);
								break;
							case Hot_Rodmp9:
								ApplyCustomSkin(pWeapon, weapon_mp9, 33);
								break;
							case Stormmp9:
								ApplyCustomSkin(pWeapon, weapon_mp9, 100);
								break;
							case Bulldozermp9:
								ApplyCustomSkin(pWeapon, weapon_mp9, 39);
								break;
							case Hypnoticmp9:
								ApplyCustomSkin(pWeapon, weapon_mp9, 61);
								break;
							case Sand_Dashedmp9:
								ApplyCustomSkin(pWeapon, weapon_mp9, 148);
								break;
							case Orange_Peelmp9:
								ApplyCustomSkin(pWeapon, weapon_mp9, 141);
								break;
							case Dry_Seasonmp9:
								ApplyCustomSkin(pWeapon, weapon_mp9, 199);
								break;
							case Dark_Agemp9:
								ApplyCustomSkin(pWeapon, weapon_mp9, 329);
								break;
							case Rose_Ironmp9:
								ApplyCustomSkin(pWeapon, weapon_mp9, 262);
								break;
							case Green_Plaidmp9:
								ApplyCustomSkin(pWeapon, weapon_mp9, 366);
								break;
							case Setting_Sunmp9:
								ApplyCustomSkin(pWeapon, weapon_mp9, 366);
								break;
							case Dartmp9:
								ApplyCustomSkin(pWeapon, weapon_mp9, 386);
								break;
							case Deadly_Poisonmp9:
								ApplyCustomSkin(pWeapon, weapon_mp9, 403);
								break;
							case Pandoras_Boxmp9:
								ApplyCustomSkin(pWeapon, weapon_mp9, 448);
								break;
							case Bioleakmp9:
								ApplyCustomSkin(pWeapon, weapon_mp9, 549);
								break;
							case Airlock:
								ApplyCustomSkin(pWeapon, weapon_mp9, 609);
								break;
							case Sand_Scale:
								ApplyCustomSkin(pWeapon, weapon_mp9, 630);
								break;
							case Goo:
								ApplyCustomSkin(pWeapon, weapon_mp9, 679);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_nova:
							switch (g_Options.nova) {
							case DEFAULTNOVA:
								ApplyCustomSkin(pWeapon, weapon_nova, 0);
								break;
							case Candy_AppleNova:
								ApplyCustomSkin(pWeapon, weapon_nova, 3);
								break;
							case Blaze_OrangeNova:
								ApplyCustomSkin(pWeapon, weapon_nova, 166);
								break;
							case Modern_HunterNova:
								ApplyCustomSkin(pWeapon, weapon_nova, 164);
								break;
							case Forest_LeavesNova:
								ApplyCustomSkin(pWeapon, weapon_nova, 25);
								break;
							case Bloomstick:
								ApplyCustomSkin(pWeapon, weapon_nova, 62);
								break;
							case Sand_DuneNova:
								ApplyCustomSkin(pWeapon, weapon_nova, 99);
								break;
							case Polar_MeshNova:
								ApplyCustomSkin(pWeapon, weapon_nova, 107);
								break;
							case WalnutNova:
								ApplyCustomSkin(pWeapon, weapon_nova, 158);
								break;
							case PredatorNova:
								ApplyCustomSkin(pWeapon, weapon_nova, 170);
								break;
							case Tempest:
								ApplyCustomSkin(pWeapon, weapon_nova, 191);
								break;
							case GraphiteNova:
								ApplyCustomSkin(pWeapon, weapon_nova, 214);
								break;
							case Ghost_Camo:
								ApplyCustomSkin(pWeapon, weapon_nova, 255);
								break;
							case Rising_Skull:
								ApplyCustomSkin(pWeapon, weapon_nova, 263);
								break;
							case Antique:
								ApplyCustomSkin(pWeapon, weapon_nova, 286);
								break;
							case Green_AppleNova:
								ApplyCustomSkin(pWeapon, weapon_nova, 294);
								break;
							case Caged_Steel:
								ApplyCustomSkin(pWeapon, weapon_nova, 299);
								break;
							case Koi:
								ApplyCustomSkin(pWeapon, weapon_nova, 356);
								break;
							case Moon_in_Libra:
								ApplyCustomSkin(pWeapon, weapon_nova, 450);
								break;
							case RangerNova:
								ApplyCustomSkin(pWeapon, weapon_nova, 484);
								break;
							case Hyper_BeastNova:
								ApplyCustomSkin(pWeapon, weapon_nova, 537);
								break;
							case Exo:
								ApplyCustomSkin(pWeapon, weapon_nova, 590);
								break;
							case Gila:
								ApplyCustomSkin(pWeapon, weapon_nova, 634);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_p250:
							switch (g_Options.p250) {
							case DEFAULTP250:
								ApplyCustomSkin(pWeapon, weapon_p250, 0);
								break;
							case Metallic_DDPATp250:
								ApplyCustomSkin(pWeapon, weapon_p250, 34);
								break;
							case Whiteoutp250:
								ApplyCustomSkin(pWeapon, weapon_p250, 102);
								break;
							case Splashp250:
								ApplyCustomSkin(pWeapon, weapon_p250, 162);
								break;
							case Gunsmokep250:
								ApplyCustomSkin(pWeapon, weapon_p250, 15);
								break;
							case Modern_Hunterp250:
								ApplyCustomSkin(pWeapon, weapon_p250, 164);
								break;
							case Bone_Maskp250:
								ApplyCustomSkin(pWeapon, weapon_p250, 27);
								break;
							case Boreal_Forestp250:
								ApplyCustomSkin(pWeapon, weapon_p250, 77);
								break;
							case Sand_Dunep250:
								ApplyCustomSkin(pWeapon, weapon_p250, 99);
								break;
							case Nuclear_Threatp250:
								ApplyCustomSkin(pWeapon, weapon_p250, 168);
								break;
							case Mehndip250:
								ApplyCustomSkin(pWeapon, weapon_p250, 258);
								break;
							case Facetsp250:
								ApplyCustomSkin(pWeapon, weapon_p250, 207);
								break;
							case Hivep250:
								ApplyCustomSkin(pWeapon, weapon_p250, 219);
								break;
							case Wingshotp250:
								ApplyCustomSkin(pWeapon, weapon_p250, 501);
								break;
							case Muertosp250:
								ApplyCustomSkin(pWeapon, weapon_p250, 404);
								break;
							case Steel_Disruptionp250:
								ApplyCustomSkin(pWeapon, weapon_p250, 230);
								break;
							case Undertowp250:
								ApplyCustomSkin(pWeapon, weapon_p250, 271);
								break;
							case Franklinp250:
								ApplyCustomSkin(pWeapon, weapon_p250, 295);
								break;
							case Supernovap250:
								ApplyCustomSkin(pWeapon, weapon_p250, 358);
								break;
							case Contaminationp250:
								ApplyCustomSkin(pWeapon, weapon_p250, 373);
								break;
							case Cartelp250:
								ApplyCustomSkin(pWeapon, weapon_p250, 388);
								break;
							case Valencep250:
								ApplyCustomSkin(pWeapon, weapon_p250, 426);
								break;
							case Crimson_Kimonop250:
								ApplyCustomSkin(pWeapon, weapon_p250, 466);
								break;
							case Mint_Kimonop250:
								ApplyCustomSkin(pWeapon, weapon_p250, 467);
								break;
							case Asiimovp250:
								ApplyCustomSkin(pWeapon, weapon_p250, 551);
								break;
							case Iron_Cladp250:
								ApplyCustomSkin(pWeapon, weapon_p250, 592);
								break;
							case Ripple:
								ApplyCustomSkin(pWeapon, weapon_p250, 650);
								break;
							case Red_Rock:
								ApplyCustomSkin(pWeapon, weapon_p250, 668);
								break;
							case See_Ya_Later:
								ApplyCustomSkin(pWeapon, weapon_p250, 678);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_scar20:
							switch (g_Options.scar20) {
							case DEFAULTSCAR:
								ApplyCustomSkin(pWeapon, weapon_scar20, 0);
								break;
							case Splash_Jam:
								ApplyCustomSkin(pWeapon, weapon_scar20, 165);
								break;
							case StormScar:
								ApplyCustomSkin(pWeapon, weapon_scar20, 100);
								break;
							case ContractorScar:
								ApplyCustomSkin(pWeapon, weapon_scar20, 46);
								break;
							case Carbon_Fiber:
								ApplyCustomSkin(pWeapon, weapon_scar20, 70);
								break;
							case Sand_Mesh:
								ApplyCustomSkin(pWeapon, weapon_scar20, 116);
								break;
							case Palm:
								ApplyCustomSkin(pWeapon, weapon_scar20, 157);
								break;
							case Emerald:
								ApplyCustomSkin(pWeapon, weapon_scar20, 196);
								break;
							case Green_Marine:
								ApplyCustomSkin(pWeapon, weapon_scar20, 502);
								break;
							case Crimson_WebScar:
								ApplyCustomSkin(pWeapon, weapon_scar20, 232);
								break;
							case Cardiac:
								ApplyCustomSkin(pWeapon, weapon_scar20, 391);
								break;
							case Army_SheenScar:
								ApplyCustomSkin(pWeapon, weapon_scar20, 298);
								break;
							case CyrexScar:
								ApplyCustomSkin(pWeapon, weapon_scar20, 312);
								break;
							case Grotto:
								ApplyCustomSkin(pWeapon, weapon_scar20, 406);
								break;
							case Outbreak:
								ApplyCustomSkin(pWeapon, weapon_scar20, 518);
								break;
							case Bloodsport:
								ApplyCustomSkin(pWeapon, weapon_scar20, 597);
								break;
							case Powercore:
								ApplyCustomSkin(pWeapon, weapon_scar20, 612);
								break;
							case Blueprint:
								ApplyCustomSkin(pWeapon, weapon_scar20, 642);
								break;
							case Jungle_Slipstream:
								ApplyCustomSkin(pWeapon, weapon_scar20, 685);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_sg553:
							switch (g_Options.sg553) {
							case DEFAULTSG553:
								ApplyCustomSkin(pWeapon, weapon_sg553, 0);
								break;
							case Tornado:
								ApplyCustomSkin(pWeapon, weapon_sg553, 101);
								break;
							case Anodized_Navy:
								ApplyCustomSkin(pWeapon, weapon_sg553, 28);
								break;
							case Bulldozer:
								ApplyCustomSkin(pWeapon, weapon_sg553, 39);
								break;
							case Ultraviolet:
								ApplyCustomSkin(pWeapon, weapon_sg553, 98);
								break;
							case Waves_Perforated:
								ApplyCustomSkin(pWeapon, weapon_sg553, 136);
								break;
							case Wave_Spray:
								ApplyCustomSkin(pWeapon, weapon_sg553, 186);
								break;
							case Gator_Mesh:
								ApplyCustomSkin(pWeapon, weapon_sg553, 243);
								break;
							case Damascus_Steel:
								ApplyCustomSkin(pWeapon, weapon_sg553, 247);
								break;
							case Pulse:
								ApplyCustomSkin(pWeapon, weapon_sg553, 287);
								break;
							case Army_Sheen:
								ApplyCustomSkin(pWeapon, weapon_sg553, 298);
								break;
							case Traveler:
								ApplyCustomSkin(pWeapon, weapon_sg553, 363);
								break;
							case Fallout_Warning:
								ApplyCustomSkin(pWeapon, weapon_sg553, 378);
								break;
							case Tiger_Moth:
								ApplyCustomSkin(pWeapon, weapon_sg553, 519);
								break;
							case Cyrexsg553:
								ApplyCustomSkin(pWeapon, weapon_sg553, 487);
								break;
							case Atlas:
								ApplyCustomSkin(pWeapon, weapon_sg553, 553);
								break;
							case Aerial:
								ApplyCustomSkin(pWeapon, weapon_sg553, 598);
								break;
							case Triarch:
								ApplyCustomSkin(pWeapon, weapon_sg553, 613);
								break;
							case Phantom:
								ApplyCustomSkin(pWeapon, weapon_sg553, 686);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_ssg08:
							switch (g_Options.ssg08)
							{
							case DEFAULTSSG:
								ApplyCustomSkin(pWeapon, weapon_ssg08, 0);
								break;
							case Lichen_Dashed:
								ApplyCustomSkin(pWeapon, weapon_ssg08, 26);
								break;
							case Dark_Waterssg:
								ApplyCustomSkin(pWeapon, weapon_ssg08, 60);
								break;
							case Blue_Spruce:
								ApplyCustomSkin(pWeapon, weapon_ssg08, 96);
								break;
							case Sand_Dune:
								ApplyCustomSkin(pWeapon, weapon_ssg08, 99);
								break;
							case Mayan_Dreams:
								ApplyCustomSkin(pWeapon, weapon_ssg08, 200);
								break;
							case Blood_in_the_Water:
								ApplyCustomSkin(pWeapon, weapon_ssg08, 222);
								break;
							case Big_Iron:
								ApplyCustomSkin(pWeapon, weapon_ssg08, 503);
								break;
							case Tropical_Storm:
								ApplyCustomSkin(pWeapon, weapon_ssg08, 233);
								break;
							case Acid_Fade:
								ApplyCustomSkin(pWeapon, weapon_ssg08, 253);
								break;
							case Slashed:
								ApplyCustomSkin(pWeapon, weapon_ssg08, 304);
								break;
							case Detour:
								ApplyCustomSkin(pWeapon, weapon_ssg08, 319);
								break;
							case Necropos:
								ApplyCustomSkin(pWeapon, weapon_ssg08, 538);
								break;
							case Abyss:
								ApplyCustomSkin(pWeapon, weapon_ssg08, 361);
								break;
							case Ghost_Crusader:
								ApplyCustomSkin(pWeapon, weapon_ssg08, 554);
								break;
							case Dragonfire:
								ApplyCustomSkin(pWeapon, weapon_ssg08, 624);
								break;
							case Deaths_Head:
								ApplyCustomSkin(pWeapon, weapon_ssg08, 670);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;

						case weapon_m4a1s:
							switch (g_Options.m4a1s)
							{
							case DEFAULT_M4A1:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 0);
								break;
							case Dark_Water:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 60);
								break;
							case Hyper_Beast:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 430);
								break;
							case Boreal_Forest:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 77);
								break;
							case VariCamoM4:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 60);
								break;
							case Golden_Coil:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 497);
								break;
							case Nitro:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 254);
								break;
							case Bright_Water:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 189);
								break;
							case Mecha_Industries:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 587);
								break;
							case Atomic_Alloy:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 301);
								break;
							case Blood_Tiger:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 217);
								break;
							case Guardian:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 257);
								break;
							case Master_Piece:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 321);
								break;
							case Knight:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 326);
								break;
							case Cyrex:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 360);
								break;
							case Basilisk:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 383);
								break;
							case Icarus_Fell:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 440);
								break;
							case Hot_Rodm4a1:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 445);
								break;
							case Chanticos_Fire:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 548);
								break;
							case Flashback:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 631);
								break;
							case Decimator:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 644);
								break;
							case Briefing:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 663);
								break;
							case Leaded_Glass:
								ApplyCustomSkin(pWeapon, weapon_m4a1s, 681);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_usp:
							switch (g_Options.usps)
							{
							case DEFAULTUSP:
								ApplyCustomSkin(pWeapon, weapon_usp, 0);
								break;
							case Forest_Leaves:
								ApplyCustomSkin(pWeapon, weapon_usp, 25);
								break;
							case Dark_WaterUSP:
								ApplyCustomSkin(pWeapon, weapon_usp, 60);
								break;
							case Overgrowth:
								ApplyCustomSkin(pWeapon, weapon_usp, 183);
								break;
							case Caiman:
								ApplyCustomSkin(pWeapon, weapon_usp, 339);
								break;
							case Blood_TigerUSP:
								ApplyCustomSkin(pWeapon, weapon_usp, 217);
								break;
							case Serum:
								ApplyCustomSkin(pWeapon, weapon_usp, 221);
								break;
							case Kill_Confirmed:
								ApplyCustomSkin(pWeapon, weapon_usp, 504);
								break;
							case Night_Ops:
								ApplyCustomSkin(pWeapon, weapon_usp, 236);
								break;
							case Stainless:
								ApplyCustomSkin(pWeapon, weapon_usp, 277);
								break;
							case GuardianUSP:
								ApplyCustomSkin(pWeapon, weapon_usp, 290);
								break;
							case Orion:
								ApplyCustomSkin(pWeapon, weapon_usp, 313);
								break;
							case Road_Rash:
								ApplyCustomSkin(pWeapon, weapon_usp, 318);
								break;
							case Royal_Blue:
								ApplyCustomSkin(pWeapon, weapon_usp, 332);
								break;
							case Business_Class:
								ApplyCustomSkin(pWeapon, weapon_usp, 364);
								break;
							case Para_Green:
								ApplyCustomSkin(pWeapon, weapon_usp, 454);
								break;
							case Torque:
								ApplyCustomSkin(pWeapon, weapon_usp, 489);
								break;
							case Lead_Conduit:
								ApplyCustomSkin(pWeapon, weapon_usp, 540);
								break;
							case CyrexUSP:
								ApplyCustomSkin(pWeapon, weapon_usp, 637);
								break;
							case Neo_NoirUSP:
								ApplyCustomSkin(pWeapon, weapon_usp, 653);
								break;
							case BlueprintUSP:
								ApplyCustomSkin(pWeapon, weapon_usp, 657);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_cz75:
							switch (g_Options.cz75a)
							{
							case DEFAULTCZ75:
								ApplyCustomSkin(pWeapon, weapon_cz75, 0);
								break;
							case Pole_Position:
								ApplyCustomSkin(pWeapon, weapon_cz75, 435);
								break;
							case Crimson_Webcz:
								ApplyCustomSkin(pWeapon, weapon_cz75, 12);
								break;
							case Hexanecz:
								ApplyCustomSkin(pWeapon, weapon_cz75, 218);
								break;
							case Tread_Plate:
								ApplyCustomSkin(pWeapon, weapon_cz75, 268);
								break;
							case The_Fuschia_Is_Now:
								ApplyCustomSkin(pWeapon, weapon_cz75, 269);
								break;
							case Victoria:
								ApplyCustomSkin(pWeapon, weapon_cz75, 270);
								break;
							case Tuxedocz:
								ApplyCustomSkin(pWeapon, weapon_cz75, 297);
								break;
							case Army_Sheencz:
								ApplyCustomSkin(pWeapon, weapon_cz75, 298);
								break;
							case Poison_Dart:
								ApplyCustomSkin(pWeapon, weapon_cz75, 315);
								break;
							case Nitrocz:
								ApplyCustomSkin(pWeapon, weapon_cz75, 322);
								break;
							case Chalicecz:
								ApplyCustomSkin(pWeapon, weapon_cz75, 325);
								break;
							case Twist:
								ApplyCustomSkin(pWeapon, weapon_cz75, 334);
								break;
							case Tigriscz:
								ApplyCustomSkin(pWeapon, weapon_cz75, 350);
								break;
							case Green_Plaidcz:
								ApplyCustomSkin(pWeapon, weapon_cz75, 366);
								break;
							case Emeraldcz:
								ApplyCustomSkin(pWeapon, weapon_cz75, 453);
								break;
							case Yellow_Jacket:
								ApplyCustomSkin(pWeapon, weapon_cz75, 476);
								break;
							case Red_Astor:
								ApplyCustomSkin(pWeapon, weapon_cz75, 543);
								break;
							case Imprintcz:
								ApplyCustomSkin(pWeapon, weapon_cz75, 603);
								break;
							case Xiangliu:
								ApplyCustomSkin(pWeapon, weapon_cz75, 643);
								break;
							case Tacticat:
								ApplyCustomSkin(pWeapon, weapon_cz75, 687);
								break;
							case Polymercz:
								ApplyCustomSkin(pWeapon, weapon_cz75, 622);
								break;
							}
							pWeapon->m_hOwnerEntity() = g_LocalPlayer;
							break;
						case weapon_revolver:
							switch (g_Options.revolver)
							{
							case DEFAULT_REVOLVERSKIN:
								ApplyCustomSkin(pWeapon, weapon_revolver, 0);
								break;
							case Crimson_Web:
								ApplyCustomSkin(pWeapon, weapon_revolver, 12);
								break;
							case Bone_Mask:
								ApplyCustomSkin(pWeapon, weapon_revolver, 27);
								break;
							case Urban_Perforated:
								ApplyCustomSkin(pWeapon, weapon_revolver, 135);
								break;
							case Waves_Perforated2:
								ApplyCustomSkin(pWeapon, weapon_revolver, 136);
								break;
							case Orange_Peel:
								ApplyCustomSkin(pWeapon, weapon_revolver, 141);
								break;
							case Urban_Masked:
								ApplyCustomSkin(pWeapon, weapon_revolver, 143);
								break;
							case Jungle_Dashed:
								ApplyCustomSkin(pWeapon, weapon_revolver, 147);
								break;
							case Sand_Dashed:
								ApplyCustomSkin(pWeapon, weapon_revolver, 148);
								break;
							case Urban_Dashed:
								ApplyCustomSkin(pWeapon, weapon_revolver, 149);
								break;
							case Dry_Season:
								ApplyCustomSkin(pWeapon, weapon_revolver, 199);
								break;
							case FadeR8:
								ApplyCustomSkin(pWeapon, weapon_revolver, 522);
								break;
							case Amber_Fade:
								ApplyCustomSkin(pWeapon, weapon_revolver, 523);
								break;
							case Reboot:
								ApplyCustomSkin(pWeapon, weapon_revolver, 595);
								break;
							case Llama_Cannon:
								ApplyCustomSkin(pWeapon, weapon_revolver, 683);
								break;
								pWeapon->m_hOwnerEntity() = g_LocalPlayer;
								break;
							}
						}
					}
					}
			}
			ofunc(g_CHLClient, stage);
		}
	//--------------------------------------------------------------------------------
	void __stdcall hkDrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
	{
		static auto ofunc = mdlrender_hook.get_original<DrawModelExecute>(index::DrawModelExecute);

		if (!g_EngineClient->IsTakingScreenshot())
			Chams::Get().OnDrawModelExecute(ctx, state, pInfo, pCustomBoneToWorld);

		ofunc(g_MdlRender, ctx, state, pInfo, pCustomBoneToWorld);

		g_MdlRender->ForcedMaterialOverride(nullptr);
	}
	//--------------------------------------------------------------------------------
	bool __stdcall hkSVCheats()
	{
		auto oSVCheats = cvar_hook.get_original<SVCheats>(index::SVCheats);

		if (!oSVCheats) return false;

		static auto CAM_Think = (DWORD)Utils::PatternScan(GetModuleHandleW(L"client.dll"), "85 C0 75 30 38 86");

		if (CAM_Think && g_Options.thirdperson && (DWORD)_ReturnAddress() == CAM_Think) return true;
		return oSVCheats(g_CVar);
	}
	//--------------------------------------------------------------------------------
	bool __fastcall hkGetPlayerInfo(void* ecx, void* edx, int ent_num, player_info_t* pInfo)
	{
		auto oGetPlayerInfo = engine_hook.get_original<GetPlayerInfo>(8);

		pInfo->steamID64 = 0;
		memset(pInfo->szName, '\0', sizeof(pInfo->szName));

		return oGetPlayerInfo(g_EngineClient, edx, ent_num, pInfo);
	}
}





































































































