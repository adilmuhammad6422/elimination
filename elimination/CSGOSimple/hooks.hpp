#pragma once

#include "valve_sdk/csgostructs.hpp"
#include "helpers/vfunc_hook.hpp"
#include <d3d9.h>

extern int aimspot;
extern int pistol_aimbot_key;

namespace index
{
	constexpr auto EndScene = 42;
	constexpr auto Reset = 16;
	constexpr auto PaintTraverse = 41;
	constexpr auto CreateMove = 21;
	constexpr auto PlaySound = 82;
	constexpr auto FrameStageNotify = 36;
	constexpr auto DrawModelExecute = 21;
	constexpr auto DoPostScreenSpaceEffects = 44;
	constexpr auto OverrideView = 18;
	constexpr auto FireEventClientSide = 9;
	constexpr auto InPrediction = 14;
	constexpr auto SVCheats = 13;
	constexpr auto ClientmodeCreateMove = 24;
	constexpr auto FindMDL = 10;
}

class RecvPropHook
{
public:
	RecvPropHook(RecvProp* prop, RecvVarProxyFn proxy_fn) :
		m_property(prop),
		m_original_proxy_fn(prop->m_ProxyFn)
	{
		SetProxyFunction(proxy_fn);
	}

	~RecvPropHook()
	{
		m_property->m_ProxyFn = m_original_proxy_fn;
	}

	RecvVarProxyFn GetOriginalFunction() const
	{
		return m_original_proxy_fn;
	}

	void SetProxyFunction(RecvVarProxyFn proxy_fn) const
	{
		m_property->m_ProxyFn = proxy_fn;
	}

private:
	RecvProp* m_property;
	RecvVarProxyFn m_original_proxy_fn;
};

namespace Hooks
{
	void Initialize();
	void Shutdown();

	extern vfunc_hook hlclient_hook;
	extern vfunc_hook direct3d_hook;
	extern vfunc_hook vguipanel_hook;
	extern vfunc_hook vguisurf_hook;
	extern vfunc_hook mdlrender_hook;
	extern vfunc_hook gameevents_hook;
	extern vfunc_hook prediction_hook;
	extern vfunc_hook cvar_hook;
	extern vfunc_hook mdlcache_hook;

	using EndScene = long(__stdcall*)(IDirect3DDevice9*);
	using Reset = long(__stdcall*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
	using CreateMove = void(__thiscall*)(IBaseClientDLL*, int, float, bool);
	using PaintTraverse = void(__thiscall*)(IPanel*, vgui::VPANEL, bool, bool);
	using FrameStageNotify = void(__thiscall*)(IBaseClientDLL*, ClientFrameStage_t);
	using PlaySound = void(__thiscall*)(ISurface*, const char* name);
	using DrawModelExecute = void(__thiscall*)(IVModelRender*, IMatRenderContext*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t*);
	using FireEvent = bool(__thiscall*)(IGameEventManager2*, IGameEvent* pEvent);
	using DoPostScreenEffects = int(__thiscall*)(IClientMode*, int);
	using OverrideView = void(__thiscall*)(IClientMode*, CViewSetup*);
	using FireEventClientSide = bool(__thiscall*)(IGameEventManager2*, IGameEvent*);
	using InPrediction = bool(__thiscall*)(IPrediction*);
	using SVCheats = bool(__thiscall*)(ICvar*);
	using ClientmodeCreateMove = bool(__thiscall*)(IClientMode*, float, CUserCmd*);
	using FindMDL = MDLHandle_t(__thiscall*)(IMDLCache*, char*);
	using GetPlayerInfo = bool(__fastcall*)(IVEngineClient*, void*, int, player_info_t*);

	long __stdcall hkEndScene(IDirect3DDevice9* device);
	long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);
	void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket);
	void __stdcall hkCreateMove_Proxy(int sequence_number, float input_sample_frametime, bool active);
	void __stdcall hkPaintTraverse(vgui::VPANEL panel, bool forceRepaint, bool allowForce);
	void __stdcall hkPlaySound(const char* name);
	void __stdcall hkDrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld);
	void __stdcall hkFrameStageNotify(ClientFrameStage_t stage);
	int  __stdcall hkDoPostScreenEffects(int a1);
	void __stdcall hkOverrideView(CViewSetup* setup);
	bool __fastcall hkFireEventClientSide(IGameEventManager2* thisptr, void* edx, IGameEvent* pEvent);
	bool __stdcall hkInPrediction();
	bool __stdcall hkSVCheats();
	bool __stdcall hkClientmodeCreateMove(float input_sample_frametime, CUserCmd* pCmd);
	bool __fastcall hkGetPlayerInfo(void* ecx, void* edx, int ent_num, player_info_t* pInfo);
	MDLHandle_t __stdcall hkFindMDL(char*);
}