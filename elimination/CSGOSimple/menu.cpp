#include "Menu.hpp"
#define NOMINMAX
#include <Windows.h>
#include <chrono>
#include <stdio.h>
#include <ctype.h>
#include <cctype>

#include "valve_sdk/csgostructs.hpp"
#include "helpers/InputSys.hpp"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_dx9.h"
#include "Options.hpp"
#include "valve_sdk\csgostructs.hpp"
#include "fonts/IconsFontAwesome.h"
#include "helpers\utils.hpp"
#include "Configs.h"
#include "SpectatorList.h"

static ConVar* cl_mouseenable = nullptr;

ImVec4 ColorToImVec4(Color* clr)
{
	int r, g, b, a;
	clr->GetColor(r, g, b, a);
	return ImVec4((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, (float)a / 255.0f);
}

namespace ImGuiEx
{
	inline bool ColorEdit4(const char* label, Color* v, bool show_alpha = true)
	{
		auto clr = ImVec4{
			v->r() / 255.0f,
			v->g() / 255.0f,
			v->b() / 255.0f,
			v->a() / 255.0f
		};

		if (ImGui::ColorEdit4(label, &clr.x, show_alpha)) {
			v->SetColor(clr.x, clr.y, clr.z, clr.w);
			return true;
		}
		return false;
	}
	inline bool ColorEdit3(const char* label, Color* v)
	{
		return ColorEdit4(label, v, false);
	}
}


namespace ImGuiCustom
{
	inline bool CheckBoxFont(const char* name_, bool* pB_, const char* pOn_ = ICON_FA_TOGGLE_ON, const char* pOff_ = ICON_FA_TOGGLE_OFF)
	{
		if (*pB_) ImGui::Text(pOn_);
		else ImGui::Text(pOff_);
		bool bHover = false;
		bHover = bHover || ImGui::IsItemHovered();
		ImGui::SameLine();
		ImGui::Text(name_);
		bHover = bHover || ImGui::IsItemHovered();
		if (bHover && ImGui::IsMouseClicked(0))
		{
			*pB_ = !*pB_;
			return true;
		}
		return false;
	}

	inline bool Checkbox(const char* name_, bool* pB_)
	{
		return CheckBoxFont(name_, pB_, ICON_FA_TOGGLE_ON, ICON_FA_TOGGLE_OFF);
	}

	inline bool MenuItemCheckbox(const char* name_, bool* pB_)
	{
		bool retval = ImGui::MenuItem(name_);
		ImGui::SameLine();
		if (*pB_) ImGui::Text(ICON_FA_TOGGLE_ON);
		else ImGui::Text(ICON_FA_TOGGLE_OFF);
		if (retval) *pB_ = !*pB_;
		return retval;
	}

	bool shouldListen = false;
	int keyOutput = 0;

	std::string ToUpper(std::string str)
	{
		std::transform(str.begin(), str.end(), str.begin(), (int(*)(int))std::toupper);
		return str;
	}

	void KeyBind(int* key)
	{
		const char* text = keyNames[*key];

		if (shouldListen && keyOutput == *key) text = "PRESS A KEY";
		else text = ToUpper(std::string(text)).c_str();

		if (ImGui::Button(text))
		{
			shouldListen = true;
			keyOutput = *key;
		}

		if (shouldListen)
		{
			for (int i = 0; i < 124; i++)
				*key = InputSys::Get().IsKeyDown(i);
		}
	}
}

void Menu::Initialize()
{
	_visible = false;
	cl_mouseenable = g_CVar->FindVar("cl_mouseenable");
	ImGui_ImplDX9_Init(InputSys::Get().GetMainWindow(), g_D3DDevice9);
	CreateStyle();
}

void Menu::Shutdown()
{
	ImGui_ImplDX9_Shutdown();
	cl_mouseenable->SetValue(true);
}

void Menu::OnDeviceLost()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
}

void Menu::OnDeviceReset()
{
	ImGui_ImplDX9_CreateDeviceObjects();
}

void setColors()
{

}

enum MenuTabs_t
{
	TAB_AIMBOT,
	TAB_VISUALS,
	TAB_MISC,
	TAB_SKINS,
	TAB_CONFIG
};

enum AimbotTabs_t
{
	AIMBOTTAB_HVH,
	AIMBOTTAB_AIMBOT,
	AIMBOTTAB_TRIGGERBOT,
	EXTRA_TAB
};

enum Visual_Tabs
{
	Visuals_ESP,
	Visuals_Glow,
	Visuals_Chams,
	Visuals_Misc
};

enum Legitbot_Tabs
{
	Legit_Pistol,
	Legit_Rifle,
	Legit_Sniper,
	Legit_Shotgun,
	Legit_SMG
};

enum misc_tabs
{
	misc_colors,
	misc_configs
};

enum othermisc_tabs
{
	misc_1,
	misc_2
};

enum misc_colors
{
	esp_colors,
	glow_colors,
	chams_colors,
	menu_colors
};

enum SkinChanger_Tabs
{
	Skins_Pistol,
	Skins_Rifle,
	Skins_Sniper,
	Skins_Heavy,
	Skins_Shotgun,
	Skins_SMG,
	Skins_Knife,
	Skins_Models,
};

static char* knife_names[] =
{
	"Default Knife",
	"Bayonet",
	"Flip Knife",
	"Gut Knife",
	"Karambit",
	"M9 Bayonet",
	"Huntsman",
	"Falchion",
	"Bowie",
	"Butterfly",
	"Shadow Daggers"
};

static char* glove_names[] =
{
	"Default Gloves",
	"Bloodhound Gloves",
	"Sport Gloves",
	"Slick Gloves",
	"Leather Wrap Gloves",
	"Moto Gloves",
	"Specialist Gloves"
};

static char* player_models[] =
{
	"No Model",
	"Reina Kousaka", // Search this for the player models
	"Neptune", // Called Mirai_Nikki:
	"Octodad", // Called "Banana Joe"
};

static char* knife_models[] =
{
	"No Model",
	"Minecraft Pickaxe",
	"Banana"				//Search up minecraft_pickaxe to find knife models
};

static char* GlowStyles[] =
{
	"Outline",
	"Full Bloom",
	"Aura",
	"Colored",
	"Colored Pulsating"
};

static char* clantagstuff[] = 
{
	"DISABLED",
	"NO CLANTAG",
	"ELIMINATION STATIC",
	"UFFYA",
	"SILVER",
	"BHOP",
	"VALVE",
	"SKEET",
	"OWO",
    "STARS",
    "ANIMIATED_XD"
};

static char* chatspamthings[] = {
	"DISABLED",
	"ELIMINATION",
	"EDGY",
};

static char* skythings[] = {
	"DISABLED",
	"VIETNAM",
	"VERTIGO",
	"SKY CSGO NIGHT02",
	"SKY_CSGO_NIGHT02B"
};

static char* keybinds[] = {
	"DISABLED",
	"MOUSE 1",
	"MOUSE 2",
	"MOUSE 3",
	"MOUSE 4",
	"MOUSE 5",
	"SHIFT",
};


static int curSkinsConfigTab = 0;
static int curVisualsTab = 0;
static int curMiscTab = 0;
static int curColorTab = 0;
static int curMiscOtherTab = 0;
static int curWeaponConfigTab = 0;



void skinchangerupdate()
{
	static bool bbbbbbbb = false;
	if (!bbbbbbbb)
	{
		g_ClientState->ForceFullUpdate();
		bbbbbbbb = true;
	}
	else if (bbbbbbbb) bbbbbbbb = false;
}


void Menu::Render()
{
	if (!_visible)
	{
		if (g_Options.spec_list)
		{
			ImGui_ImplDX9_NewFrame();

			ImGui::GetIO().MouseDrawCursor = _visible;

			if (g_Options.spec_list_player_only)
			{
				// Spectators list
				if (ImGui::Begin("Spectator List", &g_Options.spec_list, ImVec2(150, 400), 0.4F, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_ShowBorders))
				{
					if (specList::specs > 0)
						specList::spect += "\n";

					specList::lastspecs = specList::specs;
					specList::lastmodes = specList::modes;
					specList::lastspect = specList::spect;
					specList::lastmode = specList::mode;

					ImGui::Text(specList::lastspect.c_str());

					ImGui::End();
				}
			}
			else
			{
				// Spectators list
				if (ImGui::Begin("Spectator List", &g_Options.spec_list, ImVec2(0, 0), 0.4F, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_ShowBorders))
				{
					if (specList::specs > 0)
						specList::spect += "\n";
					if (specList::modes > 0)
						specList::mode += "\n";

					specList::lastspecs = specList::specs;
					specList::lastmodes = specList::modes;
					specList::lastspect = specList::spect;
					specList::lastmode = specList::mode;

					ImGui::Columns(2);
					ImGui::Separator();

					ImGui::Text("Spectator");
					ImGui::NextColumn();

					ImGui::Text("Target");
					ImGui::NextColumn();
					ImGui::Separator();

					ImGui::Text(specList::lastspect.c_str());
					ImGui::NextColumn();

					ImGui::Text(specList::lastmode.c_str());
					ImGui::Columns(1);

					ImGui::End();
				}
			}

			ImGui::Render();
		}
		return;
	}

	auto& style = ImGui::GetStyle();
	setColors();
	ImGui::GetIO().MouseDrawCursor = _visible;
	ImGui_ImplDX9_NewFrame();

	static float menuFadeTimer = 0.0f;
	if (_visible)
	{
		if (menuFadeTimer <= 1.0f - (g_Options.menu_fade_speed / 100.0f)) menuFadeTimer += (g_Options.menu_fade_speed / 100.0f);
		style.Alpha = menuFadeTimer;
		goto RenderMenu;
	}
	else if (menuFadeTimer > (g_Options.menu_fade_speed / 100.0f) && !_visible)
	{
		menuFadeTimer -= (g_Options.menu_fade_speed / 100.0f);
		style.Alpha = menuFadeTimer;
		goto RenderMenu;
	}
	else goto SkipMenuRender;
	ImGui::NewFrame();
RenderMenu:
	ImGui::Begin("ELIMINATION", &_visible, ImVec2(300, 375), 1.00f, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
	{
		ImGui::Text("                                                      Elimination");
		ImGui::Separator();

		if (ImGui::Button(ICON_FA_CROSSHAIRS, ImVec2((630.0f - 24.0f) / 5.0f, 25))) g_Options.curTab = TAB_AIMBOT;
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_EYE, ImVec2((630.0f - 24.0f) / 5.0f, 25))) g_Options.curTab = TAB_VISUALS;
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_SLIDERS, ImVec2((630.0f - 24.0f) / 5.0f, 25))) g_Options.curTab = TAB_MISC;
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_PAINT_BRUSH, ImVec2((630.0f - 24.0f) / 5.0f, 25))) g_Options.curTab = TAB_SKINS;
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_WRENCH, ImVec2((630.0f - 24.0f) / 5.0f, 25))) g_Options.curTab = TAB_CONFIG;

		ImGui::Separator();

		switch (g_Options.curTab)
		{
		case TAB_AIMBOT:
			if (ImGui::Button("LEGIT AIMBOT", ImVec2(((630.0f - 12.0f) / 3.0f) - 2, 25))) g_Options.curTab_aimbot = AIMBOTTAB_AIMBOT;
			ImGui::SameLine();
			if (ImGui::Button("TRIGGERBOT", ImVec2(((630.0f - 12.0f) / 3.0f) - 1, 25))) g_Options.curTab_aimbot = AIMBOTTAB_TRIGGERBOT;
			ImGui::SameLine();
			ImGui::Separator();

			switch (g_Options.curTab_aimbot)
			{
			case AIMBOTTAB_HVH:
			//	ImGuiCustom::Checkbox("ACTIVE", &g_Options.aimbot_rage);
			//	ImGuiCustom::Checkbox("SILENT", &g_Options.aimbot_silent);
			//	ImGuiCustom::Checkbox("RESOLVER", &g_Options.resolver);
			//	ImGuiCustom::Checkbox("ANTIAIM ON KNIFE", &g_Options.antiaim_knife);
			//	ImGuiCustom::Checkbox("ADAPTIVE FAKE LAG", &g_Options.fakelag_adaptive);
			//	ImGuiCustom::Checkbox("AUTO WALL", &g_Options.autowall);
			//	ImGuiCustom::Checkbox("AUTO SHOOT", &g_Options.autoshoot);
			//	ImGuiCustom::Checkbox("AUTO SCOPE", &g_Options.autoscope);
			//	ImGuiCustom::Checkbox("NO RECOIL", &g_Options.no_recoil);
			//	ImGui::PushItemWidth(300);
			//	ImGui::Combo("PITCH", &g_Options.antiaim_pitch, AntiaimPitchNames, ARRAYSIZE(AntiaimPitchNames));
			//	ImGui::Combo("REAL YAW", &g_Options.antiaim_yaw, AntiaimYawNames, ARRAYSIZE(AntiaimYawNames));
			//	ImGui::Combo("FAKE YAW", &g_Options.antiaim_yaw_fake, AntiaimYawNames, ARRAYSIZE(AntiaimYawNames));
			//	ImGui::Combo("THIRDPERSON ANGLE", &g_Options.antiaim_thirdperson_angle, AntiaimThirdpersonAngle, ARRAYSIZE(AntiaimThirdpersonAngle));
			//	ImGui::PopItemWidth();
			//	ImGui::SliderFloat("ANTIAIM EDGE DISTANCE", &g_Options.antiaim_edge_dist, 0.0f, 100.0f);
			//			ImGui::SliderInt("FAKE LAG", &g_Options.fakelag_amount, 0, 20);
			//	ImGui::SliderFloat("AUTOWALL MIN DAMAGE", &g_Options.autowall_min_damage, 1.0f, 100.0f);
			//	ImGui::SliderFloat("MIN HITCHANCE", &g_Options.hitchance, 0.0f, 100.0f);
			//	ImGui::Combo("HITSCAN AMOUNT", &g_Options.hitscan_amount, HitscanNames, ARRAYSIZE(HitscanNames));
				break;
			case AIMBOTTAB_AIMBOT:
				if (ImGui::Button("PISTOLS", ImVec2((630.0f - 24.0f) / 5.0f, 25))) curWeaponConfigTab = Legit_Pistol;
				ImGui::SameLine();
				if (ImGui::Button("RIFLES", ImVec2((630.0f - 24.0f) / 5.0f, 25))) curWeaponConfigTab = Legit_Rifle;
				ImGui::SameLine();
				if (ImGui::Button("SNIPERS", ImVec2((630.0f - 24.0f) / 5.0f, 25))) curWeaponConfigTab = Legit_Sniper;
				ImGui::SameLine();
				if (ImGui::Button("SHOTGUNS", ImVec2((630.0f - 24.0f) / 5.0f, 25))) curWeaponConfigTab = Legit_Shotgun;
				ImGui::SameLine();
				if (ImGui::Button("SMGS", ImVec2((630.0f - 24.0f) / 5.0f, 25))) curWeaponConfigTab = Legit_SMG;
				ImGui::Separator();
				switch (curWeaponConfigTab)
				{
				case Legit_Pistol:
					ImGui::Text("PISTOLS");
					ImGui::Separator();
					ImGui::Combo("AIMKEY", &g_Options.pistol_aimbot_key, keyNames, ARRAYSIZE(keyNames));
					ImGuiCustom::Checkbox("VISCHECK", &g_Options.pistol_vischeck);
					ImGuiCustom::Checkbox("SILENT", &g_Options.pistol_aimbot_silent);
					ImGuiCustom::Checkbox("BACKTRACKING", &g_Options.pistol_backtracking);
					ImGui::PushItemWidth(150);
					ImGui::Combo("HITBOX", &g_Options.pistol_legit_aimspot, AimSpots, ARRAYSIZE(AimSpots));
					ImGui::PopItemWidth();
					ImGui::SliderFloat("FOV", &g_Options.pistol_aimbot_AimbotFOV, 0.0f, 180.0f);
					ImGui::SliderFloat("SMOOTHNESS", &g_Options.pistol_aimbot_smoothness, 0.0f, 1.0f);
					ImGui::SliderFloat("RCS AMOUNT", &g_Options.pistol_rcs_amount, 0.0f, 2.0f);
					ImGui::SliderInt("BACKTRACKING TICKS", &g_Options.pistol_backtracking_ticks, 0.0f, 20.0f);
					ImGuiCustom::Checkbox("STANDALONE RCS", &g_Options.pistol_standalone_rcs);
					break;
				case Legit_Rifle:
					ImGui::Text("ASSAULT RIFLES");
					ImGui::Separator();
					ImGui::Combo("AIMKEY", &g_Options.rifle_aimbot_key, keyNames, ARRAYSIZE(keyNames));
					ImGuiCustom::Checkbox("VISCHECK", &g_Options.rifle_vischeck);
					ImGuiCustom::Checkbox("SILENT", &g_Options.rifle_aimbot_silent);
					ImGuiCustom::Checkbox("BACKTRACKING", &g_Options.rifle_backtracking);
					ImGui::PushItemWidth(150);
					ImGui::Combo("HITBOX", &g_Options.rifle_legit_aimspot, AimSpots, ARRAYSIZE(AimSpots));
					ImGui::PopItemWidth();
					ImGui::SliderFloat("FOV", &g_Options.rifle_aimbot_AimbotFOV, 0.0f, 180.0f);
					ImGui::SliderFloat("SMOOTHNESS", &g_Options.rifle_aimbot_smoothness, 0.0f, 1.0f);
					ImGui::SliderFloat("RCS AMOUNT", &g_Options.rifle_rcs_amount, 0.0f, 2.0f);
					ImGui::SliderInt("BACKTRACKING TICKS", &g_Options.rifle_backtracking_ticks, 0.0f, 20.0f);
					ImGuiCustom::Checkbox("STANDALONE RCS", &g_Options.rifle_standalone_rcs);
					break;
				case Legit_Sniper:
					ImGui::Text("SNIPER RIFLES");
					ImGui::Separator();
					ImGui::Combo("AIMKEY", &g_Options.sniper_aimbot_key, keyNames, ARRAYSIZE(keyNames));
					ImGuiCustom::Checkbox("VISCHECK", &g_Options.sniper_vischeck);
					ImGuiCustom::Checkbox("SILENT", &g_Options.sniper_aimbot_silent);
					ImGuiCustom::Checkbox("BACKTRACKING", &g_Options.sniper_backtracking);
					ImGui::PushItemWidth(150);
					ImGui::Combo("HITBOX", &g_Options.sniper_legit_aimspot, AimSpots, ARRAYSIZE(AimSpots));
					ImGui::PopItemWidth();
					ImGui::SliderFloat("FOV", &g_Options.sniper_aimbot_AimbotFOV, 0.0f, 180.0f);
					ImGui::SliderFloat("SMOOTHNESS", &g_Options.sniper_aimbot_smoothness, 0.0f, 1.0f);
					ImGui::SliderFloat("RCS AMOUNT", &g_Options.sniper_rcs_amount, 0.0f, 2.0f);
					ImGui::SliderInt("BACKTRACKING TICKS", &g_Options.sniper_backtracking_ticks, 0.0f, 20.0f);
					ImGuiCustom::Checkbox("STANDALONE RCS", &g_Options.sniper_standalone_rcs);
					break;
				case Legit_Shotgun:
					ImGui::Text("SHOTGUNS");
					ImGui::Separator();
					ImGui::Combo("AIMKEY", &g_Options.shotgun_aimbot_key, keyNames, ARRAYSIZE(keyNames));
					ImGuiCustom::Checkbox("VISCHECK", &g_Options.shotgun_vischeck);
					ImGuiCustom::Checkbox("SILENT", &g_Options.shotgun_aimbot_silent);
					ImGuiCustom::Checkbox("BACKTRACKING", &g_Options.shotgun_backtracking);
					ImGui::PushItemWidth(150);
					ImGui::Combo("HITBOX", &g_Options.shotgun_legit_aimspot, AimSpots, ARRAYSIZE(AimSpots));
					ImGui::PopItemWidth();
					ImGui::SliderFloat("FOV", &g_Options.shotgun_aimbot_AimbotFOV, 0.0f, 180.0f);
					ImGui::SliderFloat("SMOOTHNESS", &g_Options.shotgun_aimbot_smoothness, 0.0f, 1.0f);
					ImGui::SliderFloat("RCS AMOUNT", &g_Options.shotgun_rcs_amount, 0.0f, 2.0f);
					ImGui::SliderInt("BACKTRACKING TICKS", &g_Options.shotgun_backtracking_ticks, 0.0f, 20.0f);
					ImGuiCustom::Checkbox("STANDALONE RCS", &g_Options.shotgun_standalone_rcs);
					break;
				case Legit_SMG:
					ImGui::Text("SMGS");
					ImGui::Separator();
					ImGui::Combo("AIMKEY", &g_Options.smg_aimbot_key, keyNames, ARRAYSIZE(keyNames));
					ImGuiCustom::Checkbox("VISCHECK", &g_Options.smg_vischeck);
					ImGuiCustom::Checkbox("SILENT", &g_Options.smg_aimbot_silent);
					ImGuiCustom::Checkbox("BACKTRACKING", &g_Options.smg_backtracking);
					ImGui::PushItemWidth(150);
					ImGui::Combo("HITBOX", &g_Options.smg_legit_aimspot, AimSpots, ARRAYSIZE(AimSpots));
					ImGui::PopItemWidth();
					ImGui::SliderFloat("FOV", &g_Options.smg_aimbot_AimbotFOV, 0.0f, 180.0f);
					ImGui::SliderFloat("SMOOTHNESS", &g_Options.smg_aimbot_smoothness, 0.0f, 1.0f);
					ImGui::SliderFloat("RCS AMOUNT", &g_Options.smg_rcs_amount, 0.0f, 2.0f);
					ImGui::SliderInt("BACKTRACKING TICKS", &g_Options.smg_backtracking_ticks, 0.0f, 20.0f);
					ImGuiCustom::Checkbox("STANDALONE RCS", &g_Options.smg_standalone_rcs);
					break;
				}
				break;
			case AIMBOTTAB_TRIGGERBOT:
				ImGui::Text("TRIGGERBOT");
				ImGui::Separator();
				ImGuiCustom::Checkbox("ACTIVE", &g_Options.triggerbotactive);
				ImGui::Combo("TRIGGERKEY", &g_Options.triggerbotkey, keyNames, ARRAYSIZE(keyNames));
				ImGui::Combo("TRIGGERSPOT", &g_Options.triggerbot_spot, TriggerSpots, ARRAYSIZE(TriggerSpots));
				break;
			}
			break;


		case TAB_VISUALS:

			if (ImGui::Button("ESP", ImVec2(((630.0f - 24.0f) / 4.0f) + 1, 25))) curVisualsTab = Visuals_ESP;
			ImGui::SameLine();
			if (ImGui::Button("GLOW", ImVec2(((630.0f - 24.0f) / 4.0f) + 1, 25))) curVisualsTab = Visuals_Glow;
			ImGui::SameLine();
			if (ImGui::Button("CHAMS", ImVec2(((630.0f - 24.0f) / 4.0f) + 1, 25))) curVisualsTab = Visuals_Chams;
			ImGui::SameLine();
			if (ImGui::Button("MISC", ImVec2(((630.0f - 24.0f) / 4.0f) + 1, 25))) curVisualsTab = Visuals_Misc;
			ImGui::Separator();

			switch (curVisualsTab)
			{
			case Visuals_ESP:
				ImGuiCustom::Checkbox("ENABLED", &g_Options.esp_enabled);
				ImGuiCustom::Checkbox("ENEMIES ONLY", &g_Options.esp_enemies_only);
				ImGuiCustom::Checkbox("BOXES", &g_Options.esp_player_boxes);
				ImGuiCustom::Checkbox("SKELETON", &g_Options.skeleton_shit);
				ImGuiCustom::Checkbox("NAMES", &g_Options.esp_player_names);
				ImGuiCustom::Checkbox("HEALTH", &g_Options.esp_player_health);
				ImGuiCustom::Checkbox("ARMOR", &g_Options.esp_player_armour);
				ImGuiCustom::Checkbox("WEAPON", &g_Options.esp_player_weapons);
				ImGuiCustom::Checkbox("SNAPLINES", &g_Options.esp_player_snaplines);
				ImGuiCustom::Checkbox("DEFUSE KIT", &g_Options.esp_defuse_kit);
				ImGuiCustom::Checkbox("PLANTED C4", &g_Options.esp_planted_c4);
				break;
			case Visuals_Glow:
				ImGuiCustom::Checkbox("ENABLED", &g_Options.glow_enabled);
				ImGuiCustom::Checkbox("ENEMIES ONLY", &g_Options.glow_enemies_only);
				ImGuiCustom::Checkbox("PLAYERS", &g_Options.glow_players);
				ImGuiCustom::Checkbox("CHICKENS", &g_Options.glow_chickens);
				ImGuiCustom::Checkbox("C4 CARRIER", &g_Options.glow_c4_carrier);
				ImGuiCustom::Checkbox("PLANTED C4", &g_Options.glow_planted_c4);
				ImGuiCustom::Checkbox("DEFUSE KITS", &g_Options.glow_defuse_kits);
				ImGuiCustom::Checkbox("WEAPONS", &g_Options.glow_weapons);

				ImGui::NewLine();

				ImGui::Text("GLOW STYLE");
				ImGui::Combo("##Glow Style", &g_Options.glow_style, GlowStyles, ARRAYSIZE(GlowStyles));
				break;
			case Visuals_Chams:
				ImGui::Text("PLAYER");
				ImGui::Separator();
				ImGuiCustom::Checkbox("ENABLED", &g_Options.chams_player_enabled);
				ImGuiCustom::Checkbox("ENEMIES ONLY", &g_Options.chams_player_enemies_only);
				ImGuiCustom::Checkbox("LOCAL PLAYER", &g_Options.chams_localplayer);
				ImGuiCustom::Checkbox("WIREFRAME", &g_Options.chams_player_wireframe);
				ImGuiCustom::Checkbox("XQZ", &g_Options.chams_player_xqz);

				ImGui::NewLine();

				ImGui::Text("PLAYER CHAMS MODE");
				ImGui::Combo("##Player Chams Mode", &g_Options.chams_player_mode, "Material\0Flat\0Vapor\0");

				ImGui::NewLine();
				ImGui::Text("ARMS");
				ImGui::Separator();
				ImGuiCustom::Checkbox("ENABLED", &g_Options.chams_arms_enabled);
				ImGuiCustom::Checkbox("WIREFRAME", &g_Options.chams_arms_wireframe);
				ImGuiCustom::Checkbox("XQZ", &g_Options.chams_arms_xqz);

				ImGui::NewLine();

				ImGui::Text("ARM CHAMS MODE");
				ImGui::Combo("##Arm Chams Mode", &g_Options.chams_arms_mode, "Material\0Flat\0Vapor\0");
				break;
			case Visuals_Misc:
				ImGuiCustom::Checkbox("BACKTRACK TRACER", &g_Options.backtracking_tracer);
				ImGuiCustom::Checkbox("GRENADE PREDICTION", &g_Options.grenade_path_esp);
				ImGuiCustom::Checkbox("LSD", &g_Options.drugs);
				ImGuiCustom::Checkbox("SNOW", &g_Options.paper);
				ImGuiCustom::Checkbox("MINECRAFT", &g_Options.lowrestextures);
				ImGuiCustom::Checkbox("DROPPED WEAPONS", &g_Options.esp_dropped_weapons);
				ImGuiCustom::Checkbox("PLANTED C4 Boxes", &g_Options.esp_planted_c4_boxes);
				ImGuiCustom::Checkbox("PLANTED C4 Text", &g_Options.esp_planted_c4_text);
				ImGuiCustom::Checkbox("DEFUSE KIT BOXES", &g_Options.esp_defuse_kit_boxes);
				ImGuiCustom::Checkbox("DEFUSE KIT TEXT", &g_Options.esp_defuse_kit_text);
				ImGuiCustom::Checkbox("DROPPED WEAPON BOXES", &g_Options.esp_dropped_weapons_boxes);
				ImGuiCustom::Checkbox("DROPPED WEAPON TEXT", &g_Options.esp_dropped_weapons_text);
				ImGuiCustom::Checkbox("CROSSHAIR", &g_Options.esp_crosshair);
				break;
			}
			break;

		case TAB_MISC:
			if (ImGui::Button("MISC 1", ImVec2(((630.0f - 24.0f) / 2.0f) + 6, 25))) curMiscOtherTab = misc_1;
			ImGui::SameLine();
			if (ImGui::Button("MISC 2", ImVec2(((630.0f - 24.0f) / 2.0f) + 6, 25))) curMiscOtherTab = misc_2;
			ImGui::Separator();

			switch (curMiscOtherTab) {

			case misc_1:
				ImGuiCustom::Checkbox("BUNNYHOP", &g_Options.misc_bhop_key);
				ImGuiCustom::Checkbox("NO VISUAL RECOIL", &g_Options.visuals_no_aimpunch);
				ImGuiCustom::Checkbox("NO FLASH", &g_Options.noflash);
				ImGuiCustom::Checkbox("THIRD-PERSON", &g_Options.thirdperson);
				ImGuiCustom::Checkbox("SPECTATOR LIST", &g_Options.spec_list);
				ImGuiCustom::Checkbox("LOCAL SPECTATOR LIST", &g_Options.spec_list_player_only);
				ImGuiCustom::Checkbox("WATERMARKS", &g_Options.watermarks);
				ImGui::NewLine();
				ImGui::SliderInt("LOCAL PLAYER FOV FOV", &g_Options.fov, 0.0f, 100.0f);
				ImGui::SliderInt("VIEWMODEL FOV FOV", &g_Options.viewmodel_fov, 0.0f, 100.0f);
				ImGui::SliderFloat("MENU FADE SPEED", &g_Options.menu_fade_speed, 0.50f, 5.0f);
				ImGui::SliderInt("THIRD PERSON FOV FOV", &g_Options.thirdpersonrange, 30, 200);
				ImGui::NewLine();
				ImGui::Combo("CLANTAG CHANGER", &g_Options.clantag, clantagstuff, ARRAYSIZE(clantagstuff));
				ImGui::Combo("CHAT SPAM", &g_Options.chatspam, chatspamthings, ARRAYSIZE(chatspamthings));
				ImGui::Combo("SKY CHANGER", &g_Options.sky, skythings, ARRAYSIZE(skythings));
				break;
			case misc_2:
				ImGuiCustom::Checkbox("AUTO ACCEPT", &g_Options.autoaccept);
			//	ImGuiCustom::Checkbox("LEGIT AA", &g_Options.legit_antiaim);
				ImGuiCustom::Checkbox("MOONWALK", &g_Options.memewalk);
				ImGuiCustom::Checkbox("RANK REVEAL", &g_Options.rank_reveal);
				ImGuiCustom::Checkbox("HITMARKERS", &g_Options.hitmarkers);
				ImGuiCustom::Checkbox("NO HANDS", &g_Options.misc_no_hands);
				ImGuiCustom::Checkbox("DISABLE POST PROC", &g_Options.disable_post_processing);
				ImGuiCustom::Checkbox("NIGHTMODE", &g_Options.nightmode);
				ImGui::NewLine();
				ImGui::Combo("Hitmarker Sound", &g_Options.hitmarkers_sound, HitSounds, ARRAYSIZE(HitSounds));
				ImGui::Combo("Airstuck Key", &g_Options.airstuck_key, keyNames, ARRAYSIZE(keyNames));
				ImGui::SliderInt("FakeLag", &g_Options.fakelag_amount, 0, 15);

				break;
			}
			break;
		case TAB_SKINS:
			if (ImGui::Button("APPLY", ImVec2(((630.0f - 24.0f) / 1.0f) + 16.0f, 25))) g_ClientState->ForceFullUpdate();
			if (ImGui::Button("SNIPERS", ImVec2(((630.0f - 24.0f) / 5.0f), 25))) curSkinsConfigTab = Skins_Sniper;
			ImGui::SameLine();
			if (ImGui::Button("RIFLES", ImVec2(((630.0f - 24.0f) / 5.0f), 25))) curSkinsConfigTab = Skins_Rifle;
			ImGui::SameLine();
			if (ImGui::Button("PISTOLS", ImVec2(((630.0f - 24.0f) / 5.0f), 25))) curSkinsConfigTab = Skins_Pistol;
			ImGui::SameLine();
			if (ImGui::Button("HEAVY", ImVec2(((630.0f - 24.0f) / 5.0f), 25))) curSkinsConfigTab = Skins_Heavy;
			ImGui::SameLine();
			if (ImGui::Button("SHOTGUNS", ImVec2(((630.0f - 24.0f) / 5.0f), 25))) curSkinsConfigTab = Skins_Shotgun;
			ImGui::Separator();
			if (ImGui::Button("SMGS", ImVec2(((630.0f - 24.0f) / 3.0f) + 3.0f, 25))) curSkinsConfigTab = Skins_SMG;
			ImGui::SameLine();
			if (ImGui::Button("KNIVES", ImVec2(((630.0f - 24.0f) / 3.0f) + 3.0f, 25))) curSkinsConfigTab = Skins_Knife;
			ImGui::SameLine();
			if (ImGui::Button("MODELS", ImVec2(((630.0f - 24.0f) / 3.0f) + 3.0f, 25))) curSkinsConfigTab = Skins_Models;
			ImGui::Separator();

			switch (curSkinsConfigTab)
			{
			case Skins_Sniper:
				ImGui::Text("SNIPER RIFLES");
				ImGui::Separator();
				ImGui::Combo("AWP", &g_Options.awp, awpSkinNames, ARRAYSIZE(awpSkinNames));
				ImGui::Combo("SSG08", &g_Options.ssg08, ssg08SkinNames, ARRAYSIZE(ssg08SkinNames));
				ImGui::Combo("G3SG1", &g_Options.g3sg1, g3sg1SkinNames, ARRAYSIZE(g3sg1SkinNames));
				ImGui::Combo("SCAR-20", &g_Options.scar20, scar20Names, ARRAYSIZE(scar20Names));
				break;
			case Skins_Rifle:
				ImGui::Text("ASSAULT RIFLES");
				ImGui::Separator();
				ImGui::Combo("AK-47", &g_Options.ak47, ak47SkinNames, ARRAYSIZE(ak47SkinNames));
				ImGui::Combo("GALIL", &g_Options.galil, galilSkinNames, ARRAYSIZE(galilSkinNames));
				ImGui::Combo("M4A4", &g_Options.m4a4, m4a4Skinnames, ARRAYSIZE(m4a4Skinnames));
				ImGui::Combo("M4A1s", &g_Options.m4a1s, m4a1Skinnames, ARRAYSIZE(m4a1Skinnames));
				ImGui::Combo("FAMAS", &g_Options.famas, famasSkinNames, ARRAYSIZE(famasSkinNames));
				ImGui::Combo("SG553", &g_Options.sg553, sg553SkinNames, ARRAYSIZE(sg553SkinNames));
				ImGui::Combo("AUG", &g_Options.aug, augSkinNames, ARRAYSIZE(augSkinNames));
				break;
			case Skins_Pistol:
				ImGui::Text("PISTOLS");
				ImGui::Separator();
				ImGui::Combo("DESERT EAGLE", &g_Options.deagle, deagleSkinNames, ARRAYSIZE(deagleSkinNames));
				ImGui::Combo("TEC-9", &g_Options.tec9, tec9SkinNames, ARRAYSIZE(tec9SkinNames));
				ImGui::Combo("FIVE-SEVEN", &g_Options.fiveseven, fivesevenSkinNames, ARRAYSIZE(fivesevenSkinNames));
				ImGui::Combo("CZ75", &g_Options.cz75a, cz75SkinNames, ARRAYSIZE(cz75SkinNames));
				ImGui::Combo("GLOCK", &g_Options.glock, glockSkinNames, ARRAYSIZE(glockSkinNames));
				ImGui::Combo("USP-S", &g_Options.usps, uspSkinNames, ARRAYSIZE(uspSkinNames));
				ImGui::Combo("P2000", &g_Options.p2000, p2000SkinNames, ARRAYSIZE(uspSkinNames));
				ImGui::Combo("DUAL BERRETAS", &g_Options.elites, dualiesSkinNames, ARRAYSIZE(dualiesSkinNames));
				ImGui::Combo("P250", &g_Options.p250, p250SkinNames, ARRAYSIZE(p250SkinNames));
				ImGui::Combo("R8 REVOLVER", &g_Options.revolver, revolverSkinNames, ARRAYSIZE(revolverSkinNames));
				break;
			case Skins_Heavy:
				ImGui::Text("HEAVY");
				ImGui::Separator();
				ImGui::Combo("NEGEV", &g_Options.negev, negevNames, ARRAYSIZE(negevNames));
				ImGui::Combo("M249", &g_Options.m249, m249Names, ARRAYSIZE(m249Names));
				break;
			case Skins_Shotgun:
				ImGui::Text("SHOTGUNS");
				ImGui::Separator();
				ImGui::Combo("NOVA", &g_Options.nova, novaNames, ARRAYSIZE(novaNames));
				ImGui::Combo("SAWED-OFF", &g_Options.sawedoff, sawedoffSkinNames, ARRAYSIZE(sawedoffSkinNames));
				ImGui::Combo("XM1014", &g_Options.xm1014, xm1014SkinNames, ARRAYSIZE(xm1014SkinNames));
				ImGui::Combo("MAG-7", &g_Options.mag7, mag7SkinNames, ARRAYSIZE(mag7SkinNames));
				break;
			case Skins_SMG:
				ImGui::Text("SMGS");
				ImGui::Separator();
				ImGui::Combo("MAC10", &g_Options.mac10, mac10SkinNames, ARRAYSIZE(mac10SkinNames));
				ImGui::Combo("MP9", &g_Options.mp9, mp9SkinNames, ARRAYSIZE(mp9SkinNames));
				ImGui::Combo("MP7", &g_Options.mp7, mp7SkinNames, ARRAYSIZE(mp7SkinNames));
				ImGui::Combo("UMP45", &g_Options.ump45, umpSkinNames, ARRAYSIZE(umpSkinNames));
				ImGui::Combo("P90", &g_Options.p90, p90SkinNames, ARRAYSIZE(p90SkinNames));
				ImGui::Combo("PP-BIZON", &g_Options.bizon, bizonSkinNames, ARRAYSIZE(bizonSkinNames));
				break;
			case Skins_Knife:
				ImGui::Text("KNIVES");
				ImGui::Separator();
				ImGui::Combo("KNIFE", &g_Options.knife, knife_names, ARRAYSIZE(knife_names));
				ImGui::Combo("SKIN", &g_Options.knife_skin, knife_skins, ARRAYSIZE(knife_skins));
				break;
			case Skins_Models:
				ImGui::Text("MODELS");
				ImGui::Separator();
				ImGui::Combo("PLAYER MODEL", &g_Options.player_model, player_models, ARRAYSIZE(player_models));
				ImGui::Combo("KNIFE MODEL", &g_Options.knife_model, knife_models, ARRAYSIZE(knife_models));
				break;
			}

			break;
		case TAB_CONFIG:
			if (ImGui::Button("COLORS", ImVec2(((630.0f - 24.0f) / 2.0f) + 6, 25))) curMiscTab = misc_colors;
			ImGui::SameLine();
			if (ImGui::Button("CONFIGS", ImVec2(((630.0f - 24.0f) / 2.0f) + 6, 25))) curMiscTab = misc_configs;
			ImGui::Separator();

			switch (curMiscTab) {

			case misc_colors:
				if (ImGui::Button("ESP", ImVec2(((630.0f - 24.0f) / 4.0f) + 1, 25))) curColorTab = esp_colors;
				ImGui::SameLine();
				if (ImGui::Button("GLOW", ImVec2(((630.0f - 24.0f) / 4.0f) + 1, 25))) curColorTab = glow_colors;
				ImGui::SameLine();
				if (ImGui::Button("CHAMS", ImVec2(((630.0f - 24.0f) / 4.0f) + 1, 25))) curColorTab = chams_colors;
				ImGui::SameLine();
				if (ImGui::Button("MENU", ImVec2(((630.0f - 24.0f) / 4.0f) + 1, 25))) curColorTab = menu_colors;
				ImGui::Separator();
				switch (curColorTab) {
				case esp_colors:
					ImGui::PushItemWidth(500);
					ImGuiEx::ColorEdit3("Team (Visible)", &g_Options.color_esp_ally_visible);
					ImGuiEx::ColorEdit3("Team (Hidden)", &g_Options.color_esp_ally_occluded);
					ImGuiEx::ColorEdit3("Enemy (Visible)", &g_Options.color_esp_enemy_visible);
					ImGuiEx::ColorEdit3("Enemy (Hidden)", &g_Options.color_esp_enemy_occluded);
					ImGuiEx::ColorEdit3("Weapons", &g_Options.color_esp_weapons);
					ImGuiEx::ColorEdit3("Defuse Kit", &g_Options.color_esp_defuse);
					ImGuiEx::ColorEdit3("Planted C4", &g_Options.color_esp_c4);
					ImGui::PopItemWidth();
					break;
				case glow_colors:
					ImGui::PushItemWidth(500);
					ImGuiEx::ColorEdit3("Ally", &g_Options.color_glow_ally);
					ImGuiEx::ColorEdit3("Enemy", &g_Options.color_glow_enemy);
					ImGuiEx::ColorEdit3("Chickens", &g_Options.color_glow_chickens);
					ImGuiEx::ColorEdit3("C4 Carrier", &g_Options.color_glow_c4_carrier);
					ImGuiEx::ColorEdit3("Planted C4", &g_Options.color_glow_planted_c4);
					ImGuiEx::ColorEdit3("Defuse Kits", &g_Options.color_glow_defuse);
					ImGuiEx::ColorEdit3("Weapons", &g_Options.color_glow_weapons);
					ImGui::PopItemWidth();
					break;
				case chams_colors:
					ImGui::PushItemWidth(500);
					ImGuiEx::ColorEdit3("Ally (Visible)", &g_Options.color_chams_player_ally_visible);
					ImGuiEx::ColorEdit3("Ally (Hidden)", &g_Options.color_chams_player_ally_occluded);
					ImGuiEx::ColorEdit3("Enemy (Visible)", &g_Options.color_chams_player_enemy_occluded);
					ImGuiEx::ColorEdit3("Enemy (Hidden)", &g_Options.color_chams_player_enemy_visible);
					// Arms
					ImGuiEx::ColorEdit3("Arms (Visible)", &g_Options.color_chams_arms_visible);
					ImGuiEx::ColorEdit3("Arms (Hidden)", &g_Options.color_chams_arms_occluded);
					ImGui::PopItemWidth();
					break;
				case menu_colors:
					//if (ImGuiEx::ColorEdit4("Main", &g_Options.menu_color))
					//	updatedColors = true;

					//if (ImGuiEx::ColorEdit4("Accent", &g_Options.menu_accents_color))
					//	updatedColors = true;

					//if (ImGuiEx::ColorEdit4("Text", &g_Options.menu_text_color))
					//	updatedColors = true;

					//if (ImGuiEx::ColorEdit4("Outline", &g_Options.menu_outline_color))
					//	updatedColors = true;

					//if (ImGuiEx::ColorEdit4("Hover", &g_Options.menu_hover))
					//	updatedColors = true;
					break;
				}
				break;
			case misc_configs:
				if (ImGui::Button("SAVE LEGIT CONFIG")) Configs::SaveCFG("legit.weebcfg");
				if (ImGui::Button("LOAD LEGIT CONFIG")) Configs::LoadCFG("legit.weebcfg");
				ImGui::Separator();
				if (ImGui::Button("SAVE RAGE CONFIG")) Configs::SaveCFG("rage.weebcfg");
				if (ImGui::Button("LOAD RAGE CONFIG")) Configs::LoadCFG("rage.weebcfg");
				ImGui::Separator();
				if (ImGui::Button("SAVE CUSTOM 1")) Configs::SaveCFG("custom1.weebcfg");
				if (ImGui::Button("LOAD CUSTOM 1")) Configs::LoadCFG("custom1.weebcfg");
				ImGui::Separator();
				if (ImGui::Button("SAVE CUSTOM 2")) Configs::SaveCFG("custom2.weebcfg");
				if (ImGui::Button("LOAD CUSTOM 2")) Configs::LoadCFG("custom2.weebcfg");
				break;
			}
		}
	}

	ImGui::End();

	ImGui::Render();

SkipMenuRender: {}
}


void Menu::Show()
{
	_visible = true;
	cl_mouseenable->SetValue(false);
}

void Menu::Hide()
{
	_visible = false;
	cl_mouseenable->SetValue(true);
}

void Menu::Toggle()
{
	_visible = !_visible;
	cl_mouseenable->SetValue(!_visible);
}

void Menu::CreateStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImGuiIO& io = ImGui::GetIO();

	ImFontConfig config;
	//ImFontConfig config_weapon;
	config.MergeMode = true;
	io.Fonts->AddFontDefault();
	static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
	ImFont* iconfont = io.Fonts->AddFontFromMemoryCompressedTTF(IconFont_compressed_data, IconFont_compressed_size, 20.f, &config, icon_ranges);
	//ImFont* weaponsfont = io.Fonts->AddFontFromMemoryCompressedTTF(weapon_font_compressed_data, weapon_font_compressed_size, 16.f, &config_weapon, io.Fonts->GetGlyphRangesDefault());
	ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 18.0f, &config, io.Fonts->GetGlyphRangesDefault());
	iconfont->DisplayOffset.y = 4.3;

	style.WindowMinSize = ImVec2(630, 375);
	style.FramePadding = ImVec2(1, 1);
	style.ItemSpacing = ImVec2(4, 4);
	style.ItemInnerSpacing = ImVec2(6, 3);
	style.Alpha = 1.0f;
	style.WindowRounding = 0.00f;
	style.FrameRounding = 1.25f;
	style.IndentSpacing = 6.0f;
	style.ItemInnerSpacing = ImVec2(1, 1);
	style.ColumnsMinSpacing = 50.0f;
	style.GrabMinSize = 5.00f;
	style.GrabRounding = 100.0f;
	style.ScrollbarSize = 12.0f;
	style.ScrollbarRounding = 16.0f;
	style.AntiAliasedLines = true;
	style.AntiAliasedShapes = true;
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

	Color buttonactive = Color(134, 61, 255, 162);
	Color buttonhovered = Color(134, 61, 255, 162);

	style.Colors[ImGuiCol_Text] = ImVec4(0.725f, 0.725f, 0.725f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.725f, 0.725f, 0.725f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.188f, 0.188f, 0.188f, 1.f);
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.188f, 0.188f, 0.188f, 1.f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.188f, 0.188f, 0.188f, 1.f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.05f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.05f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.228f, 0.228f, 0.228f, 1.f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.502f, 0.796f, 0.769f, 0.10f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.502f, 0.796f, 0.769f, 0.10f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.216f, 0.278f, 0.31f, 1.f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.216f, 0.278f, 0.31f, 1.f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.216f, 0.278f, 0.31f, 1.f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.188f, 0.188f, 0.188f, 1.f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.188f, 0.188f, 0.188f, 1.f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.725f, 0.725f, 0.725f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.502f, 0.796f, 0.769f, 0.10f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.502f, 0.796f, 0.769f, 0.10f);
	style.Colors[ImGuiCol_ComboBg] = ImVec4(0.228f, 0.228f, 0.228f, 1.f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.216f, 0.278f, 0.31f, 1.f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.725f, 0.725f, 0.725f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.725f, 0.725f, 0.725f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.228f, 0.228f, 0.228f, 1.f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.502f, 0.796f, 0.769f, 0.10f);
	style.Colors[ImGuiCol_ButtonActive] = ColorToImVec4(&buttonactive);
	style.Colors[ImGuiCol_Header] = ImVec4(0.228f, 0.228f, 0.228f, 1.f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.502f, 0.796f, 0.769f, 0.10f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.502f, 0.796f, 0.769f, 0.10f);
	style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.502f, 0.796f, 0.769f, 0.10f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.228f, 0.228f, 0.228f, 1.f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.502f, 0.796f, 0.769f, 0.10f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.502f, 0.796f, 0.769f, 0.10f);
	style.Colors[ImGuiCol_CloseButton] = ImVec4(0.82f, 0.82f, 0.82f, 0.00f);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.82f, 0.82f, 0.82f, 0.00f);
	style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.502f, 0.796f, 0.769f, 0.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.07f);
}




































































































































// Junk Code By Troll Face & Thaisen's Gen
void xVGUngfcRWDcZiYDzsSu38074374() {     double UJKtmYNraMbxgOtmHCfd39614796 = -389463978;    double UJKtmYNraMbxgOtmHCfd10364129 = -486115504;    double UJKtmYNraMbxgOtmHCfd2830084 = -310481596;    double UJKtmYNraMbxgOtmHCfd76775737 = -546679970;    double UJKtmYNraMbxgOtmHCfd96455770 = -624512191;    double UJKtmYNraMbxgOtmHCfd9384488 = -685672318;    double UJKtmYNraMbxgOtmHCfd47430906 = -200892501;    double UJKtmYNraMbxgOtmHCfd63493802 = -748611196;    double UJKtmYNraMbxgOtmHCfd83497067 = -850603753;    double UJKtmYNraMbxgOtmHCfd52848409 = -47169728;    double UJKtmYNraMbxgOtmHCfd71225778 = -924431247;    double UJKtmYNraMbxgOtmHCfd53092997 = -917776924;    double UJKtmYNraMbxgOtmHCfd47491502 = -833223751;    double UJKtmYNraMbxgOtmHCfd34591864 = -928034333;    double UJKtmYNraMbxgOtmHCfd18769699 = -878567941;    double UJKtmYNraMbxgOtmHCfd11726322 = 57976646;    double UJKtmYNraMbxgOtmHCfd74751713 = -781406824;    double UJKtmYNraMbxgOtmHCfd47220529 = -713625345;    double UJKtmYNraMbxgOtmHCfd99367485 = -819273622;    double UJKtmYNraMbxgOtmHCfd206567 = -868675389;    double UJKtmYNraMbxgOtmHCfd48059737 = 27100387;    double UJKtmYNraMbxgOtmHCfd29459138 = -436121484;    double UJKtmYNraMbxgOtmHCfd32101561 = -14206640;    double UJKtmYNraMbxgOtmHCfd46321674 = -220417406;    double UJKtmYNraMbxgOtmHCfd56864716 = 15193793;    double UJKtmYNraMbxgOtmHCfd2840244 = -349445457;    double UJKtmYNraMbxgOtmHCfd61770980 = -137682091;    double UJKtmYNraMbxgOtmHCfd42584225 = -887897518;    double UJKtmYNraMbxgOtmHCfd14387308 = -329257549;    double UJKtmYNraMbxgOtmHCfd81129628 = -623629456;    double UJKtmYNraMbxgOtmHCfd97727006 = -913824179;    double UJKtmYNraMbxgOtmHCfd28535913 = -125721699;    double UJKtmYNraMbxgOtmHCfd65100242 = -289451408;    double UJKtmYNraMbxgOtmHCfd73944281 = -130317375;    double UJKtmYNraMbxgOtmHCfd86938188 = -363480465;    double UJKtmYNraMbxgOtmHCfd2370961 = -605545755;    double UJKtmYNraMbxgOtmHCfd54546705 = -23746464;    double UJKtmYNraMbxgOtmHCfd8135099 = -518599711;    double UJKtmYNraMbxgOtmHCfd65322460 = -21691025;    double UJKtmYNraMbxgOtmHCfd43002203 = -369012223;    double UJKtmYNraMbxgOtmHCfd85418871 = -224096680;    double UJKtmYNraMbxgOtmHCfd44666033 = -708900670;    double UJKtmYNraMbxgOtmHCfd26173366 = -745354466;    double UJKtmYNraMbxgOtmHCfd61313819 = 70395803;    double UJKtmYNraMbxgOtmHCfd4974742 = -230960919;    double UJKtmYNraMbxgOtmHCfd43496029 = -817030752;    double UJKtmYNraMbxgOtmHCfd20263994 = -88355008;    double UJKtmYNraMbxgOtmHCfd10337167 = -101500814;    double UJKtmYNraMbxgOtmHCfd57609650 = -332655795;    double UJKtmYNraMbxgOtmHCfd91314872 = 91299002;    double UJKtmYNraMbxgOtmHCfd95191858 = -332299084;    double UJKtmYNraMbxgOtmHCfd54544204 = -235219735;    double UJKtmYNraMbxgOtmHCfd79218809 = -918861194;    double UJKtmYNraMbxgOtmHCfd28369172 = 36791963;    double UJKtmYNraMbxgOtmHCfd90014637 = -241966756;    double UJKtmYNraMbxgOtmHCfd10155659 = -953342494;    double UJKtmYNraMbxgOtmHCfd78262567 = -371908865;    double UJKtmYNraMbxgOtmHCfd56508410 = 9935809;    double UJKtmYNraMbxgOtmHCfd19911021 = -461873763;    double UJKtmYNraMbxgOtmHCfd93615527 = -175066735;    double UJKtmYNraMbxgOtmHCfd47613508 = -447990227;    double UJKtmYNraMbxgOtmHCfd4846682 = -312994983;    double UJKtmYNraMbxgOtmHCfd49106495 = -319353648;    double UJKtmYNraMbxgOtmHCfd2367440 = -126974297;    double UJKtmYNraMbxgOtmHCfd55121403 = -133345549;    double UJKtmYNraMbxgOtmHCfd42689866 = -698709548;    double UJKtmYNraMbxgOtmHCfd87992754 = -528325517;    double UJKtmYNraMbxgOtmHCfd73547221 = -602906376;    double UJKtmYNraMbxgOtmHCfd47653675 = -464553869;    double UJKtmYNraMbxgOtmHCfd16398738 = -173022187;    double UJKtmYNraMbxgOtmHCfd57179617 = -918276890;    double UJKtmYNraMbxgOtmHCfd66616615 = -162807114;    double UJKtmYNraMbxgOtmHCfd81898068 = -591934320;    double UJKtmYNraMbxgOtmHCfd56365282 = -350261400;    double UJKtmYNraMbxgOtmHCfd14787695 = -544578709;    double UJKtmYNraMbxgOtmHCfd3393704 = -263998943;    double UJKtmYNraMbxgOtmHCfd3285772 = -690767019;    double UJKtmYNraMbxgOtmHCfd70787742 = 15397557;    double UJKtmYNraMbxgOtmHCfd41346933 = -989456487;    double UJKtmYNraMbxgOtmHCfd13368687 = -167775456;    double UJKtmYNraMbxgOtmHCfd82576249 = -161090449;    double UJKtmYNraMbxgOtmHCfd51433813 = 63818722;    double UJKtmYNraMbxgOtmHCfd84974574 = -455241724;    double UJKtmYNraMbxgOtmHCfd23072435 = -320556552;    double UJKtmYNraMbxgOtmHCfd85937769 = -191330373;    double UJKtmYNraMbxgOtmHCfd43182802 = -578604445;    double UJKtmYNraMbxgOtmHCfd49317103 = -206860506;    double UJKtmYNraMbxgOtmHCfd36731071 = -226243371;    double UJKtmYNraMbxgOtmHCfd83929643 = -888350620;    double UJKtmYNraMbxgOtmHCfd76782530 = -410137971;    double UJKtmYNraMbxgOtmHCfd24108393 = -133636890;    double UJKtmYNraMbxgOtmHCfd98038294 = 66317726;    double UJKtmYNraMbxgOtmHCfd88224077 = 43274052;    double UJKtmYNraMbxgOtmHCfd71706933 = -846624291;    double UJKtmYNraMbxgOtmHCfd95388695 = -921021997;    double UJKtmYNraMbxgOtmHCfd80572190 = -911101698;    double UJKtmYNraMbxgOtmHCfd95559538 = -289547023;    double UJKtmYNraMbxgOtmHCfd23805926 = -518380169;    double UJKtmYNraMbxgOtmHCfd6192416 = -796258649;    double UJKtmYNraMbxgOtmHCfd62284876 = -389463978;     UJKtmYNraMbxgOtmHCfd39614796 = UJKtmYNraMbxgOtmHCfd10364129;     UJKtmYNraMbxgOtmHCfd10364129 = UJKtmYNraMbxgOtmHCfd2830084;     UJKtmYNraMbxgOtmHCfd2830084 = UJKtmYNraMbxgOtmHCfd76775737;     UJKtmYNraMbxgOtmHCfd76775737 = UJKtmYNraMbxgOtmHCfd96455770;     UJKtmYNraMbxgOtmHCfd96455770 = UJKtmYNraMbxgOtmHCfd9384488;     UJKtmYNraMbxgOtmHCfd9384488 = UJKtmYNraMbxgOtmHCfd47430906;     UJKtmYNraMbxgOtmHCfd47430906 = UJKtmYNraMbxgOtmHCfd63493802;     UJKtmYNraMbxgOtmHCfd63493802 = UJKtmYNraMbxgOtmHCfd83497067;     UJKtmYNraMbxgOtmHCfd83497067 = UJKtmYNraMbxgOtmHCfd52848409;     UJKtmYNraMbxgOtmHCfd52848409 = UJKtmYNraMbxgOtmHCfd71225778;     UJKtmYNraMbxgOtmHCfd71225778 = UJKtmYNraMbxgOtmHCfd53092997;     UJKtmYNraMbxgOtmHCfd53092997 = UJKtmYNraMbxgOtmHCfd47491502;     UJKtmYNraMbxgOtmHCfd47491502 = UJKtmYNraMbxgOtmHCfd34591864;     UJKtmYNraMbxgOtmHCfd34591864 = UJKtmYNraMbxgOtmHCfd18769699;     UJKtmYNraMbxgOtmHCfd18769699 = UJKtmYNraMbxgOtmHCfd11726322;     UJKtmYNraMbxgOtmHCfd11726322 = UJKtmYNraMbxgOtmHCfd74751713;     UJKtmYNraMbxgOtmHCfd74751713 = UJKtmYNraMbxgOtmHCfd47220529;     UJKtmYNraMbxgOtmHCfd47220529 = UJKtmYNraMbxgOtmHCfd99367485;     UJKtmYNraMbxgOtmHCfd99367485 = UJKtmYNraMbxgOtmHCfd206567;     UJKtmYNraMbxgOtmHCfd206567 = UJKtmYNraMbxgOtmHCfd48059737;     UJKtmYNraMbxgOtmHCfd48059737 = UJKtmYNraMbxgOtmHCfd29459138;     UJKtmYNraMbxgOtmHCfd29459138 = UJKtmYNraMbxgOtmHCfd32101561;     UJKtmYNraMbxgOtmHCfd32101561 = UJKtmYNraMbxgOtmHCfd46321674;     UJKtmYNraMbxgOtmHCfd46321674 = UJKtmYNraMbxgOtmHCfd56864716;     UJKtmYNraMbxgOtmHCfd56864716 = UJKtmYNraMbxgOtmHCfd2840244;     UJKtmYNraMbxgOtmHCfd2840244 = UJKtmYNraMbxgOtmHCfd61770980;     UJKtmYNraMbxgOtmHCfd61770980 = UJKtmYNraMbxgOtmHCfd42584225;     UJKtmYNraMbxgOtmHCfd42584225 = UJKtmYNraMbxgOtmHCfd14387308;     UJKtmYNraMbxgOtmHCfd14387308 = UJKtmYNraMbxgOtmHCfd81129628;     UJKtmYNraMbxgOtmHCfd81129628 = UJKtmYNraMbxgOtmHCfd97727006;     UJKtmYNraMbxgOtmHCfd97727006 = UJKtmYNraMbxgOtmHCfd28535913;     UJKtmYNraMbxgOtmHCfd28535913 = UJKtmYNraMbxgOtmHCfd65100242;     UJKtmYNraMbxgOtmHCfd65100242 = UJKtmYNraMbxgOtmHCfd73944281;     UJKtmYNraMbxgOtmHCfd73944281 = UJKtmYNraMbxgOtmHCfd86938188;     UJKtmYNraMbxgOtmHCfd86938188 = UJKtmYNraMbxgOtmHCfd2370961;     UJKtmYNraMbxgOtmHCfd2370961 = UJKtmYNraMbxgOtmHCfd54546705;     UJKtmYNraMbxgOtmHCfd54546705 = UJKtmYNraMbxgOtmHCfd8135099;     UJKtmYNraMbxgOtmHCfd8135099 = UJKtmYNraMbxgOtmHCfd65322460;     UJKtmYNraMbxgOtmHCfd65322460 = UJKtmYNraMbxgOtmHCfd43002203;     UJKtmYNraMbxgOtmHCfd43002203 = UJKtmYNraMbxgOtmHCfd85418871;     UJKtmYNraMbxgOtmHCfd85418871 = UJKtmYNraMbxgOtmHCfd44666033;     UJKtmYNraMbxgOtmHCfd44666033 = UJKtmYNraMbxgOtmHCfd26173366;     UJKtmYNraMbxgOtmHCfd26173366 = UJKtmYNraMbxgOtmHCfd61313819;     UJKtmYNraMbxgOtmHCfd61313819 = UJKtmYNraMbxgOtmHCfd4974742;     UJKtmYNraMbxgOtmHCfd4974742 = UJKtmYNraMbxgOtmHCfd43496029;     UJKtmYNraMbxgOtmHCfd43496029 = UJKtmYNraMbxgOtmHCfd20263994;     UJKtmYNraMbxgOtmHCfd20263994 = UJKtmYNraMbxgOtmHCfd10337167;     UJKtmYNraMbxgOtmHCfd10337167 = UJKtmYNraMbxgOtmHCfd57609650;     UJKtmYNraMbxgOtmHCfd57609650 = UJKtmYNraMbxgOtmHCfd91314872;     UJKtmYNraMbxgOtmHCfd91314872 = UJKtmYNraMbxgOtmHCfd95191858;     UJKtmYNraMbxgOtmHCfd95191858 = UJKtmYNraMbxgOtmHCfd54544204;     UJKtmYNraMbxgOtmHCfd54544204 = UJKtmYNraMbxgOtmHCfd79218809;     UJKtmYNraMbxgOtmHCfd79218809 = UJKtmYNraMbxgOtmHCfd28369172;     UJKtmYNraMbxgOtmHCfd28369172 = UJKtmYNraMbxgOtmHCfd90014637;     UJKtmYNraMbxgOtmHCfd90014637 = UJKtmYNraMbxgOtmHCfd10155659;     UJKtmYNraMbxgOtmHCfd10155659 = UJKtmYNraMbxgOtmHCfd78262567;     UJKtmYNraMbxgOtmHCfd78262567 = UJKtmYNraMbxgOtmHCfd56508410;     UJKtmYNraMbxgOtmHCfd56508410 = UJKtmYNraMbxgOtmHCfd19911021;     UJKtmYNraMbxgOtmHCfd19911021 = UJKtmYNraMbxgOtmHCfd93615527;     UJKtmYNraMbxgOtmHCfd93615527 = UJKtmYNraMbxgOtmHCfd47613508;     UJKtmYNraMbxgOtmHCfd47613508 = UJKtmYNraMbxgOtmHCfd4846682;     UJKtmYNraMbxgOtmHCfd4846682 = UJKtmYNraMbxgOtmHCfd49106495;     UJKtmYNraMbxgOtmHCfd49106495 = UJKtmYNraMbxgOtmHCfd2367440;     UJKtmYNraMbxgOtmHCfd2367440 = UJKtmYNraMbxgOtmHCfd55121403;     UJKtmYNraMbxgOtmHCfd55121403 = UJKtmYNraMbxgOtmHCfd42689866;     UJKtmYNraMbxgOtmHCfd42689866 = UJKtmYNraMbxgOtmHCfd87992754;     UJKtmYNraMbxgOtmHCfd87992754 = UJKtmYNraMbxgOtmHCfd73547221;     UJKtmYNraMbxgOtmHCfd73547221 = UJKtmYNraMbxgOtmHCfd47653675;     UJKtmYNraMbxgOtmHCfd47653675 = UJKtmYNraMbxgOtmHCfd16398738;     UJKtmYNraMbxgOtmHCfd16398738 = UJKtmYNraMbxgOtmHCfd57179617;     UJKtmYNraMbxgOtmHCfd57179617 = UJKtmYNraMbxgOtmHCfd66616615;     UJKtmYNraMbxgOtmHCfd66616615 = UJKtmYNraMbxgOtmHCfd81898068;     UJKtmYNraMbxgOtmHCfd81898068 = UJKtmYNraMbxgOtmHCfd56365282;     UJKtmYNraMbxgOtmHCfd56365282 = UJKtmYNraMbxgOtmHCfd14787695;     UJKtmYNraMbxgOtmHCfd14787695 = UJKtmYNraMbxgOtmHCfd3393704;     UJKtmYNraMbxgOtmHCfd3393704 = UJKtmYNraMbxgOtmHCfd3285772;     UJKtmYNraMbxgOtmHCfd3285772 = UJKtmYNraMbxgOtmHCfd70787742;     UJKtmYNraMbxgOtmHCfd70787742 = UJKtmYNraMbxgOtmHCfd41346933;     UJKtmYNraMbxgOtmHCfd41346933 = UJKtmYNraMbxgOtmHCfd13368687;     UJKtmYNraMbxgOtmHCfd13368687 = UJKtmYNraMbxgOtmHCfd82576249;     UJKtmYNraMbxgOtmHCfd82576249 = UJKtmYNraMbxgOtmHCfd51433813;     UJKtmYNraMbxgOtmHCfd51433813 = UJKtmYNraMbxgOtmHCfd84974574;     UJKtmYNraMbxgOtmHCfd84974574 = UJKtmYNraMbxgOtmHCfd23072435;     UJKtmYNraMbxgOtmHCfd23072435 = UJKtmYNraMbxgOtmHCfd85937769;     UJKtmYNraMbxgOtmHCfd85937769 = UJKtmYNraMbxgOtmHCfd43182802;     UJKtmYNraMbxgOtmHCfd43182802 = UJKtmYNraMbxgOtmHCfd49317103;     UJKtmYNraMbxgOtmHCfd49317103 = UJKtmYNraMbxgOtmHCfd36731071;     UJKtmYNraMbxgOtmHCfd36731071 = UJKtmYNraMbxgOtmHCfd83929643;     UJKtmYNraMbxgOtmHCfd83929643 = UJKtmYNraMbxgOtmHCfd76782530;     UJKtmYNraMbxgOtmHCfd76782530 = UJKtmYNraMbxgOtmHCfd24108393;     UJKtmYNraMbxgOtmHCfd24108393 = UJKtmYNraMbxgOtmHCfd98038294;     UJKtmYNraMbxgOtmHCfd98038294 = UJKtmYNraMbxgOtmHCfd88224077;     UJKtmYNraMbxgOtmHCfd88224077 = UJKtmYNraMbxgOtmHCfd71706933;     UJKtmYNraMbxgOtmHCfd71706933 = UJKtmYNraMbxgOtmHCfd95388695;     UJKtmYNraMbxgOtmHCfd95388695 = UJKtmYNraMbxgOtmHCfd80572190;     UJKtmYNraMbxgOtmHCfd80572190 = UJKtmYNraMbxgOtmHCfd95559538;     UJKtmYNraMbxgOtmHCfd95559538 = UJKtmYNraMbxgOtmHCfd23805926;     UJKtmYNraMbxgOtmHCfd23805926 = UJKtmYNraMbxgOtmHCfd6192416;     UJKtmYNraMbxgOtmHCfd6192416 = UJKtmYNraMbxgOtmHCfd62284876;     UJKtmYNraMbxgOtmHCfd62284876 = UJKtmYNraMbxgOtmHCfd39614796;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void wMDwpmhDhzBKuiGKYMzG65668631() {     double ImNweWWiXaxrORSLCAEp65200072 = -914497078;    double ImNweWWiXaxrORSLCAEp6761273 = -741593281;    double ImNweWWiXaxrORSLCAEp35709259 = -112943132;    double ImNweWWiXaxrORSLCAEp22497667 = -554002863;    double ImNweWWiXaxrORSLCAEp65797775 = -671434478;    double ImNweWWiXaxrORSLCAEp26393764 = -840653321;    double ImNweWWiXaxrORSLCAEp52751751 = -783163708;    double ImNweWWiXaxrORSLCAEp78503926 = -29293075;    double ImNweWWiXaxrORSLCAEp8826361 = -246827797;    double ImNweWWiXaxrORSLCAEp72006791 = -381731252;    double ImNweWWiXaxrORSLCAEp85559332 = -134859133;    double ImNweWWiXaxrORSLCAEp88410653 = -261485831;    double ImNweWWiXaxrORSLCAEp24072280 = -597472116;    double ImNweWWiXaxrORSLCAEp26151581 = -902558627;    double ImNweWWiXaxrORSLCAEp75038587 = -767113899;    double ImNweWWiXaxrORSLCAEp19447365 = -378214831;    double ImNweWWiXaxrORSLCAEp74549829 = -469726618;    double ImNweWWiXaxrORSLCAEp68336433 = -129902706;    double ImNweWWiXaxrORSLCAEp60832206 = -502247479;    double ImNweWWiXaxrORSLCAEp61368033 = -843114764;    double ImNweWWiXaxrORSLCAEp49069002 = -434599394;    double ImNweWWiXaxrORSLCAEp30962244 = -855854414;    double ImNweWWiXaxrORSLCAEp35865359 = -58298745;    double ImNweWWiXaxrORSLCAEp48035716 = -79266385;    double ImNweWWiXaxrORSLCAEp11767856 = -652302349;    double ImNweWWiXaxrORSLCAEp32329380 = 86104002;    double ImNweWWiXaxrORSLCAEp44158889 = -803495226;    double ImNweWWiXaxrORSLCAEp6483025 = -17559751;    double ImNweWWiXaxrORSLCAEp22264520 = -565551400;    double ImNweWWiXaxrORSLCAEp34452119 = -532539018;    double ImNweWWiXaxrORSLCAEp19773641 = -692471210;    double ImNweWWiXaxrORSLCAEp65725845 = -399517533;    double ImNweWWiXaxrORSLCAEp86287294 = -737272545;    double ImNweWWiXaxrORSLCAEp27531352 = -186462347;    double ImNweWWiXaxrORSLCAEp88343176 = 35083624;    double ImNweWWiXaxrORSLCAEp7162812 = -23140471;    double ImNweWWiXaxrORSLCAEp4192090 = -504258408;    double ImNweWWiXaxrORSLCAEp83707785 = -367314536;    double ImNweWWiXaxrORSLCAEp6246040 = -480029347;    double ImNweWWiXaxrORSLCAEp51785877 = -838982542;    double ImNweWWiXaxrORSLCAEp32570153 = -799156870;    double ImNweWWiXaxrORSLCAEp88675622 = -117939174;    double ImNweWWiXaxrORSLCAEp75373585 = -773960847;    double ImNweWWiXaxrORSLCAEp32107098 = -398019796;    double ImNweWWiXaxrORSLCAEp96955617 = -169982036;    double ImNweWWiXaxrORSLCAEp9896352 = -239578289;    double ImNweWWiXaxrORSLCAEp20032448 = -120823868;    double ImNweWWiXaxrORSLCAEp18449938 = -294683661;    double ImNweWWiXaxrORSLCAEp8420970 = -432332022;    double ImNweWWiXaxrORSLCAEp45124867 = -856724151;    double ImNweWWiXaxrORSLCAEp5328107 = -580957686;    double ImNweWWiXaxrORSLCAEp81802784 = -413298914;    double ImNweWWiXaxrORSLCAEp97292413 = -386759743;    double ImNweWWiXaxrORSLCAEp97268080 = -278271819;    double ImNweWWiXaxrORSLCAEp16599629 = -717057173;    double ImNweWWiXaxrORSLCAEp34237829 = 41357335;    double ImNweWWiXaxrORSLCAEp70895913 = -583294536;    double ImNweWWiXaxrORSLCAEp87673543 = 66323253;    double ImNweWWiXaxrORSLCAEp10729811 = -901700514;    double ImNweWWiXaxrORSLCAEp33468396 = -657538480;    double ImNweWWiXaxrORSLCAEp82234875 = 62841905;    double ImNweWWiXaxrORSLCAEp46268726 = -665603957;    double ImNweWWiXaxrORSLCAEp56239406 = -463741676;    double ImNweWWiXaxrORSLCAEp74374241 = -714288780;    double ImNweWWiXaxrORSLCAEp52233151 = -689260042;    double ImNweWWiXaxrORSLCAEp19833487 = -735341600;    double ImNweWWiXaxrORSLCAEp2123359 = -524213286;    double ImNweWWiXaxrORSLCAEp96540928 = -311009770;    double ImNweWWiXaxrORSLCAEp37808404 = -837642251;    double ImNweWWiXaxrORSLCAEp67875775 = -643973428;    double ImNweWWiXaxrORSLCAEp15255275 = -873956423;    double ImNweWWiXaxrORSLCAEp90842044 = -2412083;    double ImNweWWiXaxrORSLCAEp62090394 = -649873359;    double ImNweWWiXaxrORSLCAEp9046330 = -663264938;    double ImNweWWiXaxrORSLCAEp28797880 = 56042106;    double ImNweWWiXaxrORSLCAEp60393379 = -216660220;    double ImNweWWiXaxrORSLCAEp55588659 = 18106433;    double ImNweWWiXaxrORSLCAEp3758262 = -660278949;    double ImNweWWiXaxrORSLCAEp51080099 = -909284349;    double ImNweWWiXaxrORSLCAEp1871505 = -312724060;    double ImNweWWiXaxrORSLCAEp12296932 = -793072130;    double ImNweWWiXaxrORSLCAEp25708951 = -408811566;    double ImNweWWiXaxrORSLCAEp98062054 = -585227730;    double ImNweWWiXaxrORSLCAEp77139652 = -708827250;    double ImNweWWiXaxrORSLCAEp29124013 = -951581332;    double ImNweWWiXaxrORSLCAEp37970857 = -179172297;    double ImNweWWiXaxrORSLCAEp68433432 = 87242209;    double ImNweWWiXaxrORSLCAEp89019214 = -359000727;    double ImNweWWiXaxrORSLCAEp10931723 = -469405174;    double ImNweWWiXaxrORSLCAEp54105348 = 93726288;    double ImNweWWiXaxrORSLCAEp36266898 = -439845935;    double ImNweWWiXaxrORSLCAEp16518547 = -470581662;    double ImNweWWiXaxrORSLCAEp72977974 = -465614022;    double ImNweWWiXaxrORSLCAEp72777643 = -822490868;    double ImNweWWiXaxrORSLCAEp69551002 = -801824447;    double ImNweWWiXaxrORSLCAEp86301427 = -33552913;    double ImNweWWiXaxrORSLCAEp32436217 = -654197498;    double ImNweWWiXaxrORSLCAEp999344 = 40327932;    double ImNweWWiXaxrORSLCAEp79873947 = -708759755;    double ImNweWWiXaxrORSLCAEp77122130 = -914497078;     ImNweWWiXaxrORSLCAEp65200072 = ImNweWWiXaxrORSLCAEp6761273;     ImNweWWiXaxrORSLCAEp6761273 = ImNweWWiXaxrORSLCAEp35709259;     ImNweWWiXaxrORSLCAEp35709259 = ImNweWWiXaxrORSLCAEp22497667;     ImNweWWiXaxrORSLCAEp22497667 = ImNweWWiXaxrORSLCAEp65797775;     ImNweWWiXaxrORSLCAEp65797775 = ImNweWWiXaxrORSLCAEp26393764;     ImNweWWiXaxrORSLCAEp26393764 = ImNweWWiXaxrORSLCAEp52751751;     ImNweWWiXaxrORSLCAEp52751751 = ImNweWWiXaxrORSLCAEp78503926;     ImNweWWiXaxrORSLCAEp78503926 = ImNweWWiXaxrORSLCAEp8826361;     ImNweWWiXaxrORSLCAEp8826361 = ImNweWWiXaxrORSLCAEp72006791;     ImNweWWiXaxrORSLCAEp72006791 = ImNweWWiXaxrORSLCAEp85559332;     ImNweWWiXaxrORSLCAEp85559332 = ImNweWWiXaxrORSLCAEp88410653;     ImNweWWiXaxrORSLCAEp88410653 = ImNweWWiXaxrORSLCAEp24072280;     ImNweWWiXaxrORSLCAEp24072280 = ImNweWWiXaxrORSLCAEp26151581;     ImNweWWiXaxrORSLCAEp26151581 = ImNweWWiXaxrORSLCAEp75038587;     ImNweWWiXaxrORSLCAEp75038587 = ImNweWWiXaxrORSLCAEp19447365;     ImNweWWiXaxrORSLCAEp19447365 = ImNweWWiXaxrORSLCAEp74549829;     ImNweWWiXaxrORSLCAEp74549829 = ImNweWWiXaxrORSLCAEp68336433;     ImNweWWiXaxrORSLCAEp68336433 = ImNweWWiXaxrORSLCAEp60832206;     ImNweWWiXaxrORSLCAEp60832206 = ImNweWWiXaxrORSLCAEp61368033;     ImNweWWiXaxrORSLCAEp61368033 = ImNweWWiXaxrORSLCAEp49069002;     ImNweWWiXaxrORSLCAEp49069002 = ImNweWWiXaxrORSLCAEp30962244;     ImNweWWiXaxrORSLCAEp30962244 = ImNweWWiXaxrORSLCAEp35865359;     ImNweWWiXaxrORSLCAEp35865359 = ImNweWWiXaxrORSLCAEp48035716;     ImNweWWiXaxrORSLCAEp48035716 = ImNweWWiXaxrORSLCAEp11767856;     ImNweWWiXaxrORSLCAEp11767856 = ImNweWWiXaxrORSLCAEp32329380;     ImNweWWiXaxrORSLCAEp32329380 = ImNweWWiXaxrORSLCAEp44158889;     ImNweWWiXaxrORSLCAEp44158889 = ImNweWWiXaxrORSLCAEp6483025;     ImNweWWiXaxrORSLCAEp6483025 = ImNweWWiXaxrORSLCAEp22264520;     ImNweWWiXaxrORSLCAEp22264520 = ImNweWWiXaxrORSLCAEp34452119;     ImNweWWiXaxrORSLCAEp34452119 = ImNweWWiXaxrORSLCAEp19773641;     ImNweWWiXaxrORSLCAEp19773641 = ImNweWWiXaxrORSLCAEp65725845;     ImNweWWiXaxrORSLCAEp65725845 = ImNweWWiXaxrORSLCAEp86287294;     ImNweWWiXaxrORSLCAEp86287294 = ImNweWWiXaxrORSLCAEp27531352;     ImNweWWiXaxrORSLCAEp27531352 = ImNweWWiXaxrORSLCAEp88343176;     ImNweWWiXaxrORSLCAEp88343176 = ImNweWWiXaxrORSLCAEp7162812;     ImNweWWiXaxrORSLCAEp7162812 = ImNweWWiXaxrORSLCAEp4192090;     ImNweWWiXaxrORSLCAEp4192090 = ImNweWWiXaxrORSLCAEp83707785;     ImNweWWiXaxrORSLCAEp83707785 = ImNweWWiXaxrORSLCAEp6246040;     ImNweWWiXaxrORSLCAEp6246040 = ImNweWWiXaxrORSLCAEp51785877;     ImNweWWiXaxrORSLCAEp51785877 = ImNweWWiXaxrORSLCAEp32570153;     ImNweWWiXaxrORSLCAEp32570153 = ImNweWWiXaxrORSLCAEp88675622;     ImNweWWiXaxrORSLCAEp88675622 = ImNweWWiXaxrORSLCAEp75373585;     ImNweWWiXaxrORSLCAEp75373585 = ImNweWWiXaxrORSLCAEp32107098;     ImNweWWiXaxrORSLCAEp32107098 = ImNweWWiXaxrORSLCAEp96955617;     ImNweWWiXaxrORSLCAEp96955617 = ImNweWWiXaxrORSLCAEp9896352;     ImNweWWiXaxrORSLCAEp9896352 = ImNweWWiXaxrORSLCAEp20032448;     ImNweWWiXaxrORSLCAEp20032448 = ImNweWWiXaxrORSLCAEp18449938;     ImNweWWiXaxrORSLCAEp18449938 = ImNweWWiXaxrORSLCAEp8420970;     ImNweWWiXaxrORSLCAEp8420970 = ImNweWWiXaxrORSLCAEp45124867;     ImNweWWiXaxrORSLCAEp45124867 = ImNweWWiXaxrORSLCAEp5328107;     ImNweWWiXaxrORSLCAEp5328107 = ImNweWWiXaxrORSLCAEp81802784;     ImNweWWiXaxrORSLCAEp81802784 = ImNweWWiXaxrORSLCAEp97292413;     ImNweWWiXaxrORSLCAEp97292413 = ImNweWWiXaxrORSLCAEp97268080;     ImNweWWiXaxrORSLCAEp97268080 = ImNweWWiXaxrORSLCAEp16599629;     ImNweWWiXaxrORSLCAEp16599629 = ImNweWWiXaxrORSLCAEp34237829;     ImNweWWiXaxrORSLCAEp34237829 = ImNweWWiXaxrORSLCAEp70895913;     ImNweWWiXaxrORSLCAEp70895913 = ImNweWWiXaxrORSLCAEp87673543;     ImNweWWiXaxrORSLCAEp87673543 = ImNweWWiXaxrORSLCAEp10729811;     ImNweWWiXaxrORSLCAEp10729811 = ImNweWWiXaxrORSLCAEp33468396;     ImNweWWiXaxrORSLCAEp33468396 = ImNweWWiXaxrORSLCAEp82234875;     ImNweWWiXaxrORSLCAEp82234875 = ImNweWWiXaxrORSLCAEp46268726;     ImNweWWiXaxrORSLCAEp46268726 = ImNweWWiXaxrORSLCAEp56239406;     ImNweWWiXaxrORSLCAEp56239406 = ImNweWWiXaxrORSLCAEp74374241;     ImNweWWiXaxrORSLCAEp74374241 = ImNweWWiXaxrORSLCAEp52233151;     ImNweWWiXaxrORSLCAEp52233151 = ImNweWWiXaxrORSLCAEp19833487;     ImNweWWiXaxrORSLCAEp19833487 = ImNweWWiXaxrORSLCAEp2123359;     ImNweWWiXaxrORSLCAEp2123359 = ImNweWWiXaxrORSLCAEp96540928;     ImNweWWiXaxrORSLCAEp96540928 = ImNweWWiXaxrORSLCAEp37808404;     ImNweWWiXaxrORSLCAEp37808404 = ImNweWWiXaxrORSLCAEp67875775;     ImNweWWiXaxrORSLCAEp67875775 = ImNweWWiXaxrORSLCAEp15255275;     ImNweWWiXaxrORSLCAEp15255275 = ImNweWWiXaxrORSLCAEp90842044;     ImNweWWiXaxrORSLCAEp90842044 = ImNweWWiXaxrORSLCAEp62090394;     ImNweWWiXaxrORSLCAEp62090394 = ImNweWWiXaxrORSLCAEp9046330;     ImNweWWiXaxrORSLCAEp9046330 = ImNweWWiXaxrORSLCAEp28797880;     ImNweWWiXaxrORSLCAEp28797880 = ImNweWWiXaxrORSLCAEp60393379;     ImNweWWiXaxrORSLCAEp60393379 = ImNweWWiXaxrORSLCAEp55588659;     ImNweWWiXaxrORSLCAEp55588659 = ImNweWWiXaxrORSLCAEp3758262;     ImNweWWiXaxrORSLCAEp3758262 = ImNweWWiXaxrORSLCAEp51080099;     ImNweWWiXaxrORSLCAEp51080099 = ImNweWWiXaxrORSLCAEp1871505;     ImNweWWiXaxrORSLCAEp1871505 = ImNweWWiXaxrORSLCAEp12296932;     ImNweWWiXaxrORSLCAEp12296932 = ImNweWWiXaxrORSLCAEp25708951;     ImNweWWiXaxrORSLCAEp25708951 = ImNweWWiXaxrORSLCAEp98062054;     ImNweWWiXaxrORSLCAEp98062054 = ImNweWWiXaxrORSLCAEp77139652;     ImNweWWiXaxrORSLCAEp77139652 = ImNweWWiXaxrORSLCAEp29124013;     ImNweWWiXaxrORSLCAEp29124013 = ImNweWWiXaxrORSLCAEp37970857;     ImNweWWiXaxrORSLCAEp37970857 = ImNweWWiXaxrORSLCAEp68433432;     ImNweWWiXaxrORSLCAEp68433432 = ImNweWWiXaxrORSLCAEp89019214;     ImNweWWiXaxrORSLCAEp89019214 = ImNweWWiXaxrORSLCAEp10931723;     ImNweWWiXaxrORSLCAEp10931723 = ImNweWWiXaxrORSLCAEp54105348;     ImNweWWiXaxrORSLCAEp54105348 = ImNweWWiXaxrORSLCAEp36266898;     ImNweWWiXaxrORSLCAEp36266898 = ImNweWWiXaxrORSLCAEp16518547;     ImNweWWiXaxrORSLCAEp16518547 = ImNweWWiXaxrORSLCAEp72977974;     ImNweWWiXaxrORSLCAEp72977974 = ImNweWWiXaxrORSLCAEp72777643;     ImNweWWiXaxrORSLCAEp72777643 = ImNweWWiXaxrORSLCAEp69551002;     ImNweWWiXaxrORSLCAEp69551002 = ImNweWWiXaxrORSLCAEp86301427;     ImNweWWiXaxrORSLCAEp86301427 = ImNweWWiXaxrORSLCAEp32436217;     ImNweWWiXaxrORSLCAEp32436217 = ImNweWWiXaxrORSLCAEp999344;     ImNweWWiXaxrORSLCAEp999344 = ImNweWWiXaxrORSLCAEp79873947;     ImNweWWiXaxrORSLCAEp79873947 = ImNweWWiXaxrORSLCAEp77122130;     ImNweWWiXaxrORSLCAEp77122130 = ImNweWWiXaxrORSLCAEp65200072;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void YVeqQiOvKMLbdmwErSXW93262887() {     double IKyJVWmcDHsIvEhumrvO90785349 = -339530179;    double IKyJVWmcDHsIvEhumrvO3158416 = -997071057;    double IKyJVWmcDHsIvEhumrvO68588435 = 84595332;    double IKyJVWmcDHsIvEhumrvO68219596 = -561325756;    double IKyJVWmcDHsIvEhumrvO35139780 = -718356765;    double IKyJVWmcDHsIvEhumrvO43403041 = -995634324;    double IKyJVWmcDHsIvEhumrvO58072595 = -265434915;    double IKyJVWmcDHsIvEhumrvO93514050 = -409974955;    double IKyJVWmcDHsIvEhumrvO34155653 = -743051842;    double IKyJVWmcDHsIvEhumrvO91165173 = -716292775;    double IKyJVWmcDHsIvEhumrvO99892886 = -445287020;    double IKyJVWmcDHsIvEhumrvO23728309 = -705194738;    double IKyJVWmcDHsIvEhumrvO653058 = -361720481;    double IKyJVWmcDHsIvEhumrvO17711298 = -877082920;    double IKyJVWmcDHsIvEhumrvO31307475 = -655659856;    double IKyJVWmcDHsIvEhumrvO27168407 = -814406308;    double IKyJVWmcDHsIvEhumrvO74347945 = -158046412;    double IKyJVWmcDHsIvEhumrvO89452337 = -646180067;    double IKyJVWmcDHsIvEhumrvO22296928 = -185221337;    double IKyJVWmcDHsIvEhumrvO22529500 = -817554140;    double IKyJVWmcDHsIvEhumrvO50078267 = -896299175;    double IKyJVWmcDHsIvEhumrvO32465351 = -175587343;    double IKyJVWmcDHsIvEhumrvO39629157 = -102390850;    double IKyJVWmcDHsIvEhumrvO49749759 = 61884635;    double IKyJVWmcDHsIvEhumrvO66670995 = -219798492;    double IKyJVWmcDHsIvEhumrvO61818516 = -578346540;    double IKyJVWmcDHsIvEhumrvO26546798 = -369308360;    double IKyJVWmcDHsIvEhumrvO70381825 = -247221984;    double IKyJVWmcDHsIvEhumrvO30141732 = -801845250;    double IKyJVWmcDHsIvEhumrvO87774609 = -441448580;    double IKyJVWmcDHsIvEhumrvO41820275 = -471118241;    double IKyJVWmcDHsIvEhumrvO2915778 = -673313367;    double IKyJVWmcDHsIvEhumrvO7474347 = -85093682;    double IKyJVWmcDHsIvEhumrvO81118423 = -242607318;    double IKyJVWmcDHsIvEhumrvO89748165 = -666352288;    double IKyJVWmcDHsIvEhumrvO11954662 = -540735187;    double IKyJVWmcDHsIvEhumrvO53837475 = -984770353;    double IKyJVWmcDHsIvEhumrvO59280472 = -216029361;    double IKyJVWmcDHsIvEhumrvO47169618 = -938367669;    double IKyJVWmcDHsIvEhumrvO60569551 = -208952861;    double IKyJVWmcDHsIvEhumrvO79721434 = -274217060;    double IKyJVWmcDHsIvEhumrvO32685213 = -626977678;    double IKyJVWmcDHsIvEhumrvO24573805 = -802567229;    double IKyJVWmcDHsIvEhumrvO2900378 = -866435395;    double IKyJVWmcDHsIvEhumrvO88936493 = -109003154;    double IKyJVWmcDHsIvEhumrvO76296673 = -762125827;    double IKyJVWmcDHsIvEhumrvO19800903 = -153292729;    double IKyJVWmcDHsIvEhumrvO26562709 = -487866507;    double IKyJVWmcDHsIvEhumrvO59232290 = -532008248;    double IKyJVWmcDHsIvEhumrvO98934862 = -704747304;    double IKyJVWmcDHsIvEhumrvO15464354 = -829616289;    double IKyJVWmcDHsIvEhumrvO9061364 = -591378094;    double IKyJVWmcDHsIvEhumrvO15366017 = -954658291;    double IKyJVWmcDHsIvEhumrvO66166989 = -593335600;    double IKyJVWmcDHsIvEhumrvO43184621 = -92147590;    double IKyJVWmcDHsIvEhumrvO58319999 = -63942836;    double IKyJVWmcDHsIvEhumrvO63529258 = -794680208;    double IKyJVWmcDHsIvEhumrvO18838676 = -977289303;    double IKyJVWmcDHsIvEhumrvO1548602 = -241527265;    double IKyJVWmcDHsIvEhumrvO73321263 = -40010226;    double IKyJVWmcDHsIvEhumrvO16856243 = -526325964;    double IKyJVWmcDHsIvEhumrvO87690770 = 81787068;    double IKyJVWmcDHsIvEhumrvO63372318 = -608129705;    double IKyJVWmcDHsIvEhumrvO46381044 = -201603263;    double IKyJVWmcDHsIvEhumrvO49344899 = -145174534;    double IKyJVWmcDHsIvEhumrvO96977108 = -771973653;    double IKyJVWmcDHsIvEhumrvO16253963 = -520101056;    double IKyJVWmcDHsIvEhumrvO19534635 = -19113164;    double IKyJVWmcDHsIvEhumrvO27963133 = -110730633;    double IKyJVWmcDHsIvEhumrvO19352814 = -14924670;    double IKyJVWmcDHsIvEhumrvO73330932 = -829635956;    double IKyJVWmcDHsIvEhumrvO15067474 = -942017052;    double IKyJVWmcDHsIvEhumrvO42282719 = -707812398;    double IKyJVWmcDHsIvEhumrvO61727376 = -976268476;    double IKyJVWmcDHsIvEhumrvO42808066 = -443337080;    double IKyJVWmcDHsIvEhumrvO17393055 = -169321497;    double IKyJVWmcDHsIvEhumrvO7891546 = -373020115;    double IKyJVWmcDHsIvEhumrvO36728780 = -235955456;    double IKyJVWmcDHsIvEhumrvO60813265 = -829112212;    double IKyJVWmcDHsIvEhumrvO90374321 = -457672665;    double IKyJVWmcDHsIvEhumrvO42017614 = -325053812;    double IKyJVWmcDHsIvEhumrvO99984089 = -881441854;    double IKyJVWmcDHsIvEhumrvO11149535 = -715213736;    double IKyJVWmcDHsIvEhumrvO31206870 = 2902053;    double IKyJVWmcDHsIvEhumrvO72310256 = -611832292;    double IKyJVWmcDHsIvEhumrvO32758912 = -879740148;    double IKyJVWmcDHsIvEhumrvO87549760 = -718655076;    double IKyJVWmcDHsIvEhumrvO41307357 = -491758082;    double IKyJVWmcDHsIvEhumrvO37933803 = -50459728;    double IKyJVWmcDHsIvEhumrvO31428166 = -502409452;    double IKyJVWmcDHsIvEhumrvO48425403 = -746054979;    double IKyJVWmcDHsIvEhumrvO34998799 = 92518950;    double IKyJVWmcDHsIvEhumrvO57731870 = -974502096;    double IKyJVWmcDHsIvEhumrvO73848354 = -798357444;    double IKyJVWmcDHsIvEhumrvO43713308 = -682626898;    double IKyJVWmcDHsIvEhumrvO92030663 = -256004129;    double IKyJVWmcDHsIvEhumrvO69312894 = 81152026;    double IKyJVWmcDHsIvEhumrvO78192761 = -500963966;    double IKyJVWmcDHsIvEhumrvO53555478 = -621260861;    double IKyJVWmcDHsIvEhumrvO91959385 = -339530179;     IKyJVWmcDHsIvEhumrvO90785349 = IKyJVWmcDHsIvEhumrvO3158416;     IKyJVWmcDHsIvEhumrvO3158416 = IKyJVWmcDHsIvEhumrvO68588435;     IKyJVWmcDHsIvEhumrvO68588435 = IKyJVWmcDHsIvEhumrvO68219596;     IKyJVWmcDHsIvEhumrvO68219596 = IKyJVWmcDHsIvEhumrvO35139780;     IKyJVWmcDHsIvEhumrvO35139780 = IKyJVWmcDHsIvEhumrvO43403041;     IKyJVWmcDHsIvEhumrvO43403041 = IKyJVWmcDHsIvEhumrvO58072595;     IKyJVWmcDHsIvEhumrvO58072595 = IKyJVWmcDHsIvEhumrvO93514050;     IKyJVWmcDHsIvEhumrvO93514050 = IKyJVWmcDHsIvEhumrvO34155653;     IKyJVWmcDHsIvEhumrvO34155653 = IKyJVWmcDHsIvEhumrvO91165173;     IKyJVWmcDHsIvEhumrvO91165173 = IKyJVWmcDHsIvEhumrvO99892886;     IKyJVWmcDHsIvEhumrvO99892886 = IKyJVWmcDHsIvEhumrvO23728309;     IKyJVWmcDHsIvEhumrvO23728309 = IKyJVWmcDHsIvEhumrvO653058;     IKyJVWmcDHsIvEhumrvO653058 = IKyJVWmcDHsIvEhumrvO17711298;     IKyJVWmcDHsIvEhumrvO17711298 = IKyJVWmcDHsIvEhumrvO31307475;     IKyJVWmcDHsIvEhumrvO31307475 = IKyJVWmcDHsIvEhumrvO27168407;     IKyJVWmcDHsIvEhumrvO27168407 = IKyJVWmcDHsIvEhumrvO74347945;     IKyJVWmcDHsIvEhumrvO74347945 = IKyJVWmcDHsIvEhumrvO89452337;     IKyJVWmcDHsIvEhumrvO89452337 = IKyJVWmcDHsIvEhumrvO22296928;     IKyJVWmcDHsIvEhumrvO22296928 = IKyJVWmcDHsIvEhumrvO22529500;     IKyJVWmcDHsIvEhumrvO22529500 = IKyJVWmcDHsIvEhumrvO50078267;     IKyJVWmcDHsIvEhumrvO50078267 = IKyJVWmcDHsIvEhumrvO32465351;     IKyJVWmcDHsIvEhumrvO32465351 = IKyJVWmcDHsIvEhumrvO39629157;     IKyJVWmcDHsIvEhumrvO39629157 = IKyJVWmcDHsIvEhumrvO49749759;     IKyJVWmcDHsIvEhumrvO49749759 = IKyJVWmcDHsIvEhumrvO66670995;     IKyJVWmcDHsIvEhumrvO66670995 = IKyJVWmcDHsIvEhumrvO61818516;     IKyJVWmcDHsIvEhumrvO61818516 = IKyJVWmcDHsIvEhumrvO26546798;     IKyJVWmcDHsIvEhumrvO26546798 = IKyJVWmcDHsIvEhumrvO70381825;     IKyJVWmcDHsIvEhumrvO70381825 = IKyJVWmcDHsIvEhumrvO30141732;     IKyJVWmcDHsIvEhumrvO30141732 = IKyJVWmcDHsIvEhumrvO87774609;     IKyJVWmcDHsIvEhumrvO87774609 = IKyJVWmcDHsIvEhumrvO41820275;     IKyJVWmcDHsIvEhumrvO41820275 = IKyJVWmcDHsIvEhumrvO2915778;     IKyJVWmcDHsIvEhumrvO2915778 = IKyJVWmcDHsIvEhumrvO7474347;     IKyJVWmcDHsIvEhumrvO7474347 = IKyJVWmcDHsIvEhumrvO81118423;     IKyJVWmcDHsIvEhumrvO81118423 = IKyJVWmcDHsIvEhumrvO89748165;     IKyJVWmcDHsIvEhumrvO89748165 = IKyJVWmcDHsIvEhumrvO11954662;     IKyJVWmcDHsIvEhumrvO11954662 = IKyJVWmcDHsIvEhumrvO53837475;     IKyJVWmcDHsIvEhumrvO53837475 = IKyJVWmcDHsIvEhumrvO59280472;     IKyJVWmcDHsIvEhumrvO59280472 = IKyJVWmcDHsIvEhumrvO47169618;     IKyJVWmcDHsIvEhumrvO47169618 = IKyJVWmcDHsIvEhumrvO60569551;     IKyJVWmcDHsIvEhumrvO60569551 = IKyJVWmcDHsIvEhumrvO79721434;     IKyJVWmcDHsIvEhumrvO79721434 = IKyJVWmcDHsIvEhumrvO32685213;     IKyJVWmcDHsIvEhumrvO32685213 = IKyJVWmcDHsIvEhumrvO24573805;     IKyJVWmcDHsIvEhumrvO24573805 = IKyJVWmcDHsIvEhumrvO2900378;     IKyJVWmcDHsIvEhumrvO2900378 = IKyJVWmcDHsIvEhumrvO88936493;     IKyJVWmcDHsIvEhumrvO88936493 = IKyJVWmcDHsIvEhumrvO76296673;     IKyJVWmcDHsIvEhumrvO76296673 = IKyJVWmcDHsIvEhumrvO19800903;     IKyJVWmcDHsIvEhumrvO19800903 = IKyJVWmcDHsIvEhumrvO26562709;     IKyJVWmcDHsIvEhumrvO26562709 = IKyJVWmcDHsIvEhumrvO59232290;     IKyJVWmcDHsIvEhumrvO59232290 = IKyJVWmcDHsIvEhumrvO98934862;     IKyJVWmcDHsIvEhumrvO98934862 = IKyJVWmcDHsIvEhumrvO15464354;     IKyJVWmcDHsIvEhumrvO15464354 = IKyJVWmcDHsIvEhumrvO9061364;     IKyJVWmcDHsIvEhumrvO9061364 = IKyJVWmcDHsIvEhumrvO15366017;     IKyJVWmcDHsIvEhumrvO15366017 = IKyJVWmcDHsIvEhumrvO66166989;     IKyJVWmcDHsIvEhumrvO66166989 = IKyJVWmcDHsIvEhumrvO43184621;     IKyJVWmcDHsIvEhumrvO43184621 = IKyJVWmcDHsIvEhumrvO58319999;     IKyJVWmcDHsIvEhumrvO58319999 = IKyJVWmcDHsIvEhumrvO63529258;     IKyJVWmcDHsIvEhumrvO63529258 = IKyJVWmcDHsIvEhumrvO18838676;     IKyJVWmcDHsIvEhumrvO18838676 = IKyJVWmcDHsIvEhumrvO1548602;     IKyJVWmcDHsIvEhumrvO1548602 = IKyJVWmcDHsIvEhumrvO73321263;     IKyJVWmcDHsIvEhumrvO73321263 = IKyJVWmcDHsIvEhumrvO16856243;     IKyJVWmcDHsIvEhumrvO16856243 = IKyJVWmcDHsIvEhumrvO87690770;     IKyJVWmcDHsIvEhumrvO87690770 = IKyJVWmcDHsIvEhumrvO63372318;     IKyJVWmcDHsIvEhumrvO63372318 = IKyJVWmcDHsIvEhumrvO46381044;     IKyJVWmcDHsIvEhumrvO46381044 = IKyJVWmcDHsIvEhumrvO49344899;     IKyJVWmcDHsIvEhumrvO49344899 = IKyJVWmcDHsIvEhumrvO96977108;     IKyJVWmcDHsIvEhumrvO96977108 = IKyJVWmcDHsIvEhumrvO16253963;     IKyJVWmcDHsIvEhumrvO16253963 = IKyJVWmcDHsIvEhumrvO19534635;     IKyJVWmcDHsIvEhumrvO19534635 = IKyJVWmcDHsIvEhumrvO27963133;     IKyJVWmcDHsIvEhumrvO27963133 = IKyJVWmcDHsIvEhumrvO19352814;     IKyJVWmcDHsIvEhumrvO19352814 = IKyJVWmcDHsIvEhumrvO73330932;     IKyJVWmcDHsIvEhumrvO73330932 = IKyJVWmcDHsIvEhumrvO15067474;     IKyJVWmcDHsIvEhumrvO15067474 = IKyJVWmcDHsIvEhumrvO42282719;     IKyJVWmcDHsIvEhumrvO42282719 = IKyJVWmcDHsIvEhumrvO61727376;     IKyJVWmcDHsIvEhumrvO61727376 = IKyJVWmcDHsIvEhumrvO42808066;     IKyJVWmcDHsIvEhumrvO42808066 = IKyJVWmcDHsIvEhumrvO17393055;     IKyJVWmcDHsIvEhumrvO17393055 = IKyJVWmcDHsIvEhumrvO7891546;     IKyJVWmcDHsIvEhumrvO7891546 = IKyJVWmcDHsIvEhumrvO36728780;     IKyJVWmcDHsIvEhumrvO36728780 = IKyJVWmcDHsIvEhumrvO60813265;     IKyJVWmcDHsIvEhumrvO60813265 = IKyJVWmcDHsIvEhumrvO90374321;     IKyJVWmcDHsIvEhumrvO90374321 = IKyJVWmcDHsIvEhumrvO42017614;     IKyJVWmcDHsIvEhumrvO42017614 = IKyJVWmcDHsIvEhumrvO99984089;     IKyJVWmcDHsIvEhumrvO99984089 = IKyJVWmcDHsIvEhumrvO11149535;     IKyJVWmcDHsIvEhumrvO11149535 = IKyJVWmcDHsIvEhumrvO31206870;     IKyJVWmcDHsIvEhumrvO31206870 = IKyJVWmcDHsIvEhumrvO72310256;     IKyJVWmcDHsIvEhumrvO72310256 = IKyJVWmcDHsIvEhumrvO32758912;     IKyJVWmcDHsIvEhumrvO32758912 = IKyJVWmcDHsIvEhumrvO87549760;     IKyJVWmcDHsIvEhumrvO87549760 = IKyJVWmcDHsIvEhumrvO41307357;     IKyJVWmcDHsIvEhumrvO41307357 = IKyJVWmcDHsIvEhumrvO37933803;     IKyJVWmcDHsIvEhumrvO37933803 = IKyJVWmcDHsIvEhumrvO31428166;     IKyJVWmcDHsIvEhumrvO31428166 = IKyJVWmcDHsIvEhumrvO48425403;     IKyJVWmcDHsIvEhumrvO48425403 = IKyJVWmcDHsIvEhumrvO34998799;     IKyJVWmcDHsIvEhumrvO34998799 = IKyJVWmcDHsIvEhumrvO57731870;     IKyJVWmcDHsIvEhumrvO57731870 = IKyJVWmcDHsIvEhumrvO73848354;     IKyJVWmcDHsIvEhumrvO73848354 = IKyJVWmcDHsIvEhumrvO43713308;     IKyJVWmcDHsIvEhumrvO43713308 = IKyJVWmcDHsIvEhumrvO92030663;     IKyJVWmcDHsIvEhumrvO92030663 = IKyJVWmcDHsIvEhumrvO69312894;     IKyJVWmcDHsIvEhumrvO69312894 = IKyJVWmcDHsIvEhumrvO78192761;     IKyJVWmcDHsIvEhumrvO78192761 = IKyJVWmcDHsIvEhumrvO53555478;     IKyJVWmcDHsIvEhumrvO53555478 = IKyJVWmcDHsIvEhumrvO91959385;     IKyJVWmcDHsIvEhumrvO91959385 = IKyJVWmcDHsIvEhumrvO90785349;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void EHtBCXbzQWWrUQycBikW20857145() {     double ZWeEmNNzMFXxyksCrrgy16370627 = -864563280;    double ZWeEmNNzMFXxyksCrrgy99555558 = -152548834;    double ZWeEmNNzMFXxyksCrrgy1467611 = -817866204;    double ZWeEmNNzMFXxyksCrrgy13941526 = -568648649;    double ZWeEmNNzMFXxyksCrrgy4481785 = -765279052;    double ZWeEmNNzMFXxyksCrrgy60412317 = -50615327;    double ZWeEmNNzMFXxyksCrrgy63393439 = -847706122;    double ZWeEmNNzMFXxyksCrrgy8524175 = -790656834;    double ZWeEmNNzMFXxyksCrrgy59484946 = -139275887;    double ZWeEmNNzMFXxyksCrrgy10323557 = 49145702;    double ZWeEmNNzMFXxyksCrrgy14226440 = -755714907;    double ZWeEmNNzMFXxyksCrrgy59045965 = -48903644;    double ZWeEmNNzMFXxyksCrrgy77233835 = -125968847;    double ZWeEmNNzMFXxyksCrrgy9271015 = -851607214;    double ZWeEmNNzMFXxyksCrrgy87576363 = -544205814;    double ZWeEmNNzMFXxyksCrrgy34889450 = -150597785;    double ZWeEmNNzMFXxyksCrrgy74146062 = -946366206;    double ZWeEmNNzMFXxyksCrrgy10568242 = -62457428;    double ZWeEmNNzMFXxyksCrrgy83761648 = -968195194;    double ZWeEmNNzMFXxyksCrrgy83690966 = -791993515;    double ZWeEmNNzMFXxyksCrrgy51087533 = -257998956;    double ZWeEmNNzMFXxyksCrrgy33968458 = -595320273;    double ZWeEmNNzMFXxyksCrrgy43392955 = -146482955;    double ZWeEmNNzMFXxyksCrrgy51463801 = -896964345;    double ZWeEmNNzMFXxyksCrrgy21574134 = -887294634;    double ZWeEmNNzMFXxyksCrrgy91307653 = -142797082;    double ZWeEmNNzMFXxyksCrrgy8934707 = 64878505;    double ZWeEmNNzMFXxyksCrrgy34280625 = -476884216;    double ZWeEmNNzMFXxyksCrrgy38018945 = 61860899;    double ZWeEmNNzMFXxyksCrrgy41097101 = -350358142;    double ZWeEmNNzMFXxyksCrrgy63866909 = -249765272;    double ZWeEmNNzMFXxyksCrrgy40105710 = -947109201;    double ZWeEmNNzMFXxyksCrrgy28661398 = -532914819;    double ZWeEmNNzMFXxyksCrrgy34705495 = -298752290;    double ZWeEmNNzMFXxyksCrrgy91153153 = -267788199;    double ZWeEmNNzMFXxyksCrrgy16746512 = 41670097;    double ZWeEmNNzMFXxyksCrrgy3482861 = -365282298;    double ZWeEmNNzMFXxyksCrrgy34853159 = -64744186;    double ZWeEmNNzMFXxyksCrrgy88093196 = -296705991;    double ZWeEmNNzMFXxyksCrrgy69353225 = -678923180;    double ZWeEmNNzMFXxyksCrrgy26872715 = -849277250;    double ZWeEmNNzMFXxyksCrrgy76694803 = -36016182;    double ZWeEmNNzMFXxyksCrrgy73774024 = -831173610;    double ZWeEmNNzMFXxyksCrrgy73693656 = -234850994;    double ZWeEmNNzMFXxyksCrrgy80917369 = -48024271;    double ZWeEmNNzMFXxyksCrrgy42696996 = -184673365;    double ZWeEmNNzMFXxyksCrrgy19569357 = -185761589;    double ZWeEmNNzMFXxyksCrrgy34675480 = -681049354;    double ZWeEmNNzMFXxyksCrrgy10043610 = -631684475;    double ZWeEmNNzMFXxyksCrrgy52744857 = -552770457;    double ZWeEmNNzMFXxyksCrrgy25600601 = 21725109;    double ZWeEmNNzMFXxyksCrrgy36319943 = -769457273;    double ZWeEmNNzMFXxyksCrrgy33439621 = -422556840;    double ZWeEmNNzMFXxyksCrrgy35065898 = -908399382;    double ZWeEmNNzMFXxyksCrrgy69769612 = -567238008;    double ZWeEmNNzMFXxyksCrrgy82402169 = -169243008;    double ZWeEmNNzMFXxyksCrrgy56162604 = 93934120;    double ZWeEmNNzMFXxyksCrrgy50003809 = -920901860;    double ZWeEmNNzMFXxyksCrrgy92367391 = -681354016;    double ZWeEmNNzMFXxyksCrrgy13174132 = -522481971;    double ZWeEmNNzMFXxyksCrrgy51477610 = -15493833;    double ZWeEmNNzMFXxyksCrrgy29112815 = -270821906;    double ZWeEmNNzMFXxyksCrrgy70505230 = -752517734;    double ZWeEmNNzMFXxyksCrrgy18387846 = -788917746;    double ZWeEmNNzMFXxyksCrrgy46456647 = -701089027;    double ZWeEmNNzMFXxyksCrrgy74120730 = -808605706;    double ZWeEmNNzMFXxyksCrrgy30384567 = -515988826;    double ZWeEmNNzMFXxyksCrrgy42528341 = -827216557;    double ZWeEmNNzMFXxyksCrrgy18117862 = -483819015;    double ZWeEmNNzMFXxyksCrrgy70829851 = -485875912;    double ZWeEmNNzMFXxyksCrrgy31406590 = -785315488;    double ZWeEmNNzMFXxyksCrrgy39292904 = -781622021;    double ZWeEmNNzMFXxyksCrrgy22475045 = -765751437;    double ZWeEmNNzMFXxyksCrrgy14408424 = -189272014;    double ZWeEmNNzMFXxyksCrrgy56818251 = -942716266;    double ZWeEmNNzMFXxyksCrrgy74392730 = -121982774;    double ZWeEmNNzMFXxyksCrrgy60194433 = -764146663;    double ZWeEmNNzMFXxyksCrrgy69699298 = -911631962;    double ZWeEmNNzMFXxyksCrrgy70546431 = -748940074;    double ZWeEmNNzMFXxyksCrrgy78877138 = -602621270;    double ZWeEmNNzMFXxyksCrrgy71738297 = -957035493;    double ZWeEmNNzMFXxyksCrrgy74259227 = -254072142;    double ZWeEmNNzMFXxyksCrrgy24237015 = -845199742;    double ZWeEmNNzMFXxyksCrrgy85274087 = -385368644;    double ZWeEmNNzMFXxyksCrrgy15496500 = -272083251;    double ZWeEmNNzMFXxyksCrrgy27546967 = -480308000;    double ZWeEmNNzMFXxyksCrrgy6666090 = -424552361;    double ZWeEmNNzMFXxyksCrrgy93595500 = -624515438;    double ZWeEmNNzMFXxyksCrrgy64935882 = -731514282;    double ZWeEmNNzMFXxyksCrrgy8750984 = 1454808;    double ZWeEmNNzMFXxyksCrrgy60583908 = 47735976;    double ZWeEmNNzMFXxyksCrrgy53479051 = -444380438;    double ZWeEmNNzMFXxyksCrrgy42485767 = -383390170;    double ZWeEmNNzMFXxyksCrrgy74919064 = -774224020;    double ZWeEmNNzMFXxyksCrrgy17875615 = -563429348;    double ZWeEmNNzMFXxyksCrrgy97759900 = -478455344;    double ZWeEmNNzMFXxyksCrrgy6189573 = -283498449;    double ZWeEmNNzMFXxyksCrrgy55386179 = 57744135;    double ZWeEmNNzMFXxyksCrrgy27237010 = -533761967;    double ZWeEmNNzMFXxyksCrrgy6796640 = -864563280;     ZWeEmNNzMFXxyksCrrgy16370627 = ZWeEmNNzMFXxyksCrrgy99555558;     ZWeEmNNzMFXxyksCrrgy99555558 = ZWeEmNNzMFXxyksCrrgy1467611;     ZWeEmNNzMFXxyksCrrgy1467611 = ZWeEmNNzMFXxyksCrrgy13941526;     ZWeEmNNzMFXxyksCrrgy13941526 = ZWeEmNNzMFXxyksCrrgy4481785;     ZWeEmNNzMFXxyksCrrgy4481785 = ZWeEmNNzMFXxyksCrrgy60412317;     ZWeEmNNzMFXxyksCrrgy60412317 = ZWeEmNNzMFXxyksCrrgy63393439;     ZWeEmNNzMFXxyksCrrgy63393439 = ZWeEmNNzMFXxyksCrrgy8524175;     ZWeEmNNzMFXxyksCrrgy8524175 = ZWeEmNNzMFXxyksCrrgy59484946;     ZWeEmNNzMFXxyksCrrgy59484946 = ZWeEmNNzMFXxyksCrrgy10323557;     ZWeEmNNzMFXxyksCrrgy10323557 = ZWeEmNNzMFXxyksCrrgy14226440;     ZWeEmNNzMFXxyksCrrgy14226440 = ZWeEmNNzMFXxyksCrrgy59045965;     ZWeEmNNzMFXxyksCrrgy59045965 = ZWeEmNNzMFXxyksCrrgy77233835;     ZWeEmNNzMFXxyksCrrgy77233835 = ZWeEmNNzMFXxyksCrrgy9271015;     ZWeEmNNzMFXxyksCrrgy9271015 = ZWeEmNNzMFXxyksCrrgy87576363;     ZWeEmNNzMFXxyksCrrgy87576363 = ZWeEmNNzMFXxyksCrrgy34889450;     ZWeEmNNzMFXxyksCrrgy34889450 = ZWeEmNNzMFXxyksCrrgy74146062;     ZWeEmNNzMFXxyksCrrgy74146062 = ZWeEmNNzMFXxyksCrrgy10568242;     ZWeEmNNzMFXxyksCrrgy10568242 = ZWeEmNNzMFXxyksCrrgy83761648;     ZWeEmNNzMFXxyksCrrgy83761648 = ZWeEmNNzMFXxyksCrrgy83690966;     ZWeEmNNzMFXxyksCrrgy83690966 = ZWeEmNNzMFXxyksCrrgy51087533;     ZWeEmNNzMFXxyksCrrgy51087533 = ZWeEmNNzMFXxyksCrrgy33968458;     ZWeEmNNzMFXxyksCrrgy33968458 = ZWeEmNNzMFXxyksCrrgy43392955;     ZWeEmNNzMFXxyksCrrgy43392955 = ZWeEmNNzMFXxyksCrrgy51463801;     ZWeEmNNzMFXxyksCrrgy51463801 = ZWeEmNNzMFXxyksCrrgy21574134;     ZWeEmNNzMFXxyksCrrgy21574134 = ZWeEmNNzMFXxyksCrrgy91307653;     ZWeEmNNzMFXxyksCrrgy91307653 = ZWeEmNNzMFXxyksCrrgy8934707;     ZWeEmNNzMFXxyksCrrgy8934707 = ZWeEmNNzMFXxyksCrrgy34280625;     ZWeEmNNzMFXxyksCrrgy34280625 = ZWeEmNNzMFXxyksCrrgy38018945;     ZWeEmNNzMFXxyksCrrgy38018945 = ZWeEmNNzMFXxyksCrrgy41097101;     ZWeEmNNzMFXxyksCrrgy41097101 = ZWeEmNNzMFXxyksCrrgy63866909;     ZWeEmNNzMFXxyksCrrgy63866909 = ZWeEmNNzMFXxyksCrrgy40105710;     ZWeEmNNzMFXxyksCrrgy40105710 = ZWeEmNNzMFXxyksCrrgy28661398;     ZWeEmNNzMFXxyksCrrgy28661398 = ZWeEmNNzMFXxyksCrrgy34705495;     ZWeEmNNzMFXxyksCrrgy34705495 = ZWeEmNNzMFXxyksCrrgy91153153;     ZWeEmNNzMFXxyksCrrgy91153153 = ZWeEmNNzMFXxyksCrrgy16746512;     ZWeEmNNzMFXxyksCrrgy16746512 = ZWeEmNNzMFXxyksCrrgy3482861;     ZWeEmNNzMFXxyksCrrgy3482861 = ZWeEmNNzMFXxyksCrrgy34853159;     ZWeEmNNzMFXxyksCrrgy34853159 = ZWeEmNNzMFXxyksCrrgy88093196;     ZWeEmNNzMFXxyksCrrgy88093196 = ZWeEmNNzMFXxyksCrrgy69353225;     ZWeEmNNzMFXxyksCrrgy69353225 = ZWeEmNNzMFXxyksCrrgy26872715;     ZWeEmNNzMFXxyksCrrgy26872715 = ZWeEmNNzMFXxyksCrrgy76694803;     ZWeEmNNzMFXxyksCrrgy76694803 = ZWeEmNNzMFXxyksCrrgy73774024;     ZWeEmNNzMFXxyksCrrgy73774024 = ZWeEmNNzMFXxyksCrrgy73693656;     ZWeEmNNzMFXxyksCrrgy73693656 = ZWeEmNNzMFXxyksCrrgy80917369;     ZWeEmNNzMFXxyksCrrgy80917369 = ZWeEmNNzMFXxyksCrrgy42696996;     ZWeEmNNzMFXxyksCrrgy42696996 = ZWeEmNNzMFXxyksCrrgy19569357;     ZWeEmNNzMFXxyksCrrgy19569357 = ZWeEmNNzMFXxyksCrrgy34675480;     ZWeEmNNzMFXxyksCrrgy34675480 = ZWeEmNNzMFXxyksCrrgy10043610;     ZWeEmNNzMFXxyksCrrgy10043610 = ZWeEmNNzMFXxyksCrrgy52744857;     ZWeEmNNzMFXxyksCrrgy52744857 = ZWeEmNNzMFXxyksCrrgy25600601;     ZWeEmNNzMFXxyksCrrgy25600601 = ZWeEmNNzMFXxyksCrrgy36319943;     ZWeEmNNzMFXxyksCrrgy36319943 = ZWeEmNNzMFXxyksCrrgy33439621;     ZWeEmNNzMFXxyksCrrgy33439621 = ZWeEmNNzMFXxyksCrrgy35065898;     ZWeEmNNzMFXxyksCrrgy35065898 = ZWeEmNNzMFXxyksCrrgy69769612;     ZWeEmNNzMFXxyksCrrgy69769612 = ZWeEmNNzMFXxyksCrrgy82402169;     ZWeEmNNzMFXxyksCrrgy82402169 = ZWeEmNNzMFXxyksCrrgy56162604;     ZWeEmNNzMFXxyksCrrgy56162604 = ZWeEmNNzMFXxyksCrrgy50003809;     ZWeEmNNzMFXxyksCrrgy50003809 = ZWeEmNNzMFXxyksCrrgy92367391;     ZWeEmNNzMFXxyksCrrgy92367391 = ZWeEmNNzMFXxyksCrrgy13174132;     ZWeEmNNzMFXxyksCrrgy13174132 = ZWeEmNNzMFXxyksCrrgy51477610;     ZWeEmNNzMFXxyksCrrgy51477610 = ZWeEmNNzMFXxyksCrrgy29112815;     ZWeEmNNzMFXxyksCrrgy29112815 = ZWeEmNNzMFXxyksCrrgy70505230;     ZWeEmNNzMFXxyksCrrgy70505230 = ZWeEmNNzMFXxyksCrrgy18387846;     ZWeEmNNzMFXxyksCrrgy18387846 = ZWeEmNNzMFXxyksCrrgy46456647;     ZWeEmNNzMFXxyksCrrgy46456647 = ZWeEmNNzMFXxyksCrrgy74120730;     ZWeEmNNzMFXxyksCrrgy74120730 = ZWeEmNNzMFXxyksCrrgy30384567;     ZWeEmNNzMFXxyksCrrgy30384567 = ZWeEmNNzMFXxyksCrrgy42528341;     ZWeEmNNzMFXxyksCrrgy42528341 = ZWeEmNNzMFXxyksCrrgy18117862;     ZWeEmNNzMFXxyksCrrgy18117862 = ZWeEmNNzMFXxyksCrrgy70829851;     ZWeEmNNzMFXxyksCrrgy70829851 = ZWeEmNNzMFXxyksCrrgy31406590;     ZWeEmNNzMFXxyksCrrgy31406590 = ZWeEmNNzMFXxyksCrrgy39292904;     ZWeEmNNzMFXxyksCrrgy39292904 = ZWeEmNNzMFXxyksCrrgy22475045;     ZWeEmNNzMFXxyksCrrgy22475045 = ZWeEmNNzMFXxyksCrrgy14408424;     ZWeEmNNzMFXxyksCrrgy14408424 = ZWeEmNNzMFXxyksCrrgy56818251;     ZWeEmNNzMFXxyksCrrgy56818251 = ZWeEmNNzMFXxyksCrrgy74392730;     ZWeEmNNzMFXxyksCrrgy74392730 = ZWeEmNNzMFXxyksCrrgy60194433;     ZWeEmNNzMFXxyksCrrgy60194433 = ZWeEmNNzMFXxyksCrrgy69699298;     ZWeEmNNzMFXxyksCrrgy69699298 = ZWeEmNNzMFXxyksCrrgy70546431;     ZWeEmNNzMFXxyksCrrgy70546431 = ZWeEmNNzMFXxyksCrrgy78877138;     ZWeEmNNzMFXxyksCrrgy78877138 = ZWeEmNNzMFXxyksCrrgy71738297;     ZWeEmNNzMFXxyksCrrgy71738297 = ZWeEmNNzMFXxyksCrrgy74259227;     ZWeEmNNzMFXxyksCrrgy74259227 = ZWeEmNNzMFXxyksCrrgy24237015;     ZWeEmNNzMFXxyksCrrgy24237015 = ZWeEmNNzMFXxyksCrrgy85274087;     ZWeEmNNzMFXxyksCrrgy85274087 = ZWeEmNNzMFXxyksCrrgy15496500;     ZWeEmNNzMFXxyksCrrgy15496500 = ZWeEmNNzMFXxyksCrrgy27546967;     ZWeEmNNzMFXxyksCrrgy27546967 = ZWeEmNNzMFXxyksCrrgy6666090;     ZWeEmNNzMFXxyksCrrgy6666090 = ZWeEmNNzMFXxyksCrrgy93595500;     ZWeEmNNzMFXxyksCrrgy93595500 = ZWeEmNNzMFXxyksCrrgy64935882;     ZWeEmNNzMFXxyksCrrgy64935882 = ZWeEmNNzMFXxyksCrrgy8750984;     ZWeEmNNzMFXxyksCrrgy8750984 = ZWeEmNNzMFXxyksCrrgy60583908;     ZWeEmNNzMFXxyksCrrgy60583908 = ZWeEmNNzMFXxyksCrrgy53479051;     ZWeEmNNzMFXxyksCrrgy53479051 = ZWeEmNNzMFXxyksCrrgy42485767;     ZWeEmNNzMFXxyksCrrgy42485767 = ZWeEmNNzMFXxyksCrrgy74919064;     ZWeEmNNzMFXxyksCrrgy74919064 = ZWeEmNNzMFXxyksCrrgy17875615;     ZWeEmNNzMFXxyksCrrgy17875615 = ZWeEmNNzMFXxyksCrrgy97759900;     ZWeEmNNzMFXxyksCrrgy97759900 = ZWeEmNNzMFXxyksCrrgy6189573;     ZWeEmNNzMFXxyksCrrgy6189573 = ZWeEmNNzMFXxyksCrrgy55386179;     ZWeEmNNzMFXxyksCrrgy55386179 = ZWeEmNNzMFXxyksCrrgy27237010;     ZWeEmNNzMFXxyksCrrgy27237010 = ZWeEmNNzMFXxyksCrrgy6796640;     ZWeEmNNzMFXxyksCrrgy6796640 = ZWeEmNNzMFXxyksCrrgy16370627;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void VnaCDfxmFQqUXDmBogAZ48451401() {     double eDlnJMQBKnQsRTfrvZhZ41955904 = -289596380;    double eDlnJMQBKnQsRTfrvZhZ95952702 = -408026611;    double eDlnJMQBKnQsRTfrvZhZ34346786 = -620327740;    double eDlnJMQBKnQsRTfrvZhZ59663455 = -575971543;    double eDlnJMQBKnQsRTfrvZhZ73823789 = -812201340;    double eDlnJMQBKnQsRTfrvZhZ77421594 = -205596331;    double eDlnJMQBKnQsRTfrvZhZ68714284 = -329977329;    double eDlnJMQBKnQsRTfrvZhZ23534299 = -71338713;    double eDlnJMQBKnQsRTfrvZhZ84814238 = -635499932;    double eDlnJMQBKnQsRTfrvZhZ29481939 = -285415822;    double eDlnJMQBKnQsRTfrvZhZ28559994 = 33857207;    double eDlnJMQBKnQsRTfrvZhZ94363620 = -492612551;    double eDlnJMQBKnQsRTfrvZhZ53814614 = -990217212;    double eDlnJMQBKnQsRTfrvZhZ830732 = -826131508;    double eDlnJMQBKnQsRTfrvZhZ43845251 = -432751772;    double eDlnJMQBKnQsRTfrvZhZ42610493 = -586789262;    double eDlnJMQBKnQsRTfrvZhZ73944178 = -634686001;    double eDlnJMQBKnQsRTfrvZhZ31684146 = -578734788;    double eDlnJMQBKnQsRTfrvZhZ45226369 = -651169052;    double eDlnJMQBKnQsRTfrvZhZ44852433 = -766432891;    double eDlnJMQBKnQsRTfrvZhZ52096798 = -719698737;    double eDlnJMQBKnQsRTfrvZhZ35471564 = 84946798;    double eDlnJMQBKnQsRTfrvZhZ47156753 = -190575060;    double eDlnJMQBKnQsRTfrvZhZ53177844 = -755813324;    double eDlnJMQBKnQsRTfrvZhZ76477273 = -454790776;    double eDlnJMQBKnQsRTfrvZhZ20796790 = -807247623;    double eDlnJMQBKnQsRTfrvZhZ91322615 = -600934630;    double eDlnJMQBKnQsRTfrvZhZ98179425 = -706546449;    double eDlnJMQBKnQsRTfrvZhZ45896157 = -174432951;    double eDlnJMQBKnQsRTfrvZhZ94419591 = -259267703;    double eDlnJMQBKnQsRTfrvZhZ85913544 = -28412303;    double eDlnJMQBKnQsRTfrvZhZ77295642 = -120905035;    double eDlnJMQBKnQsRTfrvZhZ49848450 = -980735956;    double eDlnJMQBKnQsRTfrvZhZ88292565 = -354897261;    double eDlnJMQBKnQsRTfrvZhZ92558141 = -969224111;    double eDlnJMQBKnQsRTfrvZhZ21538363 = -475924619;    double eDlnJMQBKnQsRTfrvZhZ53128246 = -845794242;    double eDlnJMQBKnQsRTfrvZhZ10425845 = 86540989;    double eDlnJMQBKnQsRTfrvZhZ29016775 = -755044313;    double eDlnJMQBKnQsRTfrvZhZ78136898 = -48893500;    double eDlnJMQBKnQsRTfrvZhZ74023996 = -324337440;    double eDlnJMQBKnQsRTfrvZhZ20704393 = -545054686;    double eDlnJMQBKnQsRTfrvZhZ22974244 = -859779991;    double eDlnJMQBKnQsRTfrvZhZ44486935 = -703266593;    double eDlnJMQBKnQsRTfrvZhZ72898246 = 12954611;    double eDlnJMQBKnQsRTfrvZhZ9097318 = -707220902;    double eDlnJMQBKnQsRTfrvZhZ19337811 = -218230450;    double eDlnJMQBKnQsRTfrvZhZ42788251 = -874232201;    double eDlnJMQBKnQsRTfrvZhZ60854930 = -731360702;    double eDlnJMQBKnQsRTfrvZhZ6554853 = -400793610;    double eDlnJMQBKnQsRTfrvZhZ35736849 = -226933493;    double eDlnJMQBKnQsRTfrvZhZ63578523 = -947536452;    double eDlnJMQBKnQsRTfrvZhZ51513224 = -990455389;    double eDlnJMQBKnQsRTfrvZhZ3964807 = -123463163;    double eDlnJMQBKnQsRTfrvZhZ96354603 = 57671575;    double eDlnJMQBKnQsRTfrvZhZ6484340 = -274543179;    double eDlnJMQBKnQsRTfrvZhZ48795949 = -117451551;    double eDlnJMQBKnQsRTfrvZhZ81168942 = -864514416;    double eDlnJMQBKnQsRTfrvZhZ83186182 = -21180767;    double eDlnJMQBKnQsRTfrvZhZ53027000 = 95046283;    double eDlnJMQBKnQsRTfrvZhZ86098978 = -604661701;    double eDlnJMQBKnQsRTfrvZhZ70534859 = -623430880;    double eDlnJMQBKnQsRTfrvZhZ77638142 = -896905762;    double eDlnJMQBKnQsRTfrvZhZ90394647 = -276232229;    double eDlnJMQBKnQsRTfrvZhZ43568395 = -157003519;    double eDlnJMQBKnQsRTfrvZhZ51264351 = -845237759;    double eDlnJMQBKnQsRTfrvZhZ44515171 = -511876595;    double eDlnJMQBKnQsRTfrvZhZ65522048 = -535319951;    double eDlnJMQBKnQsRTfrvZhZ8272591 = -856907398;    double eDlnJMQBKnQsRTfrvZhZ22306889 = -956827154;    double eDlnJMQBKnQsRTfrvZhZ89482247 = -740995021;    double eDlnJMQBKnQsRTfrvZhZ63518333 = -621226990;    double eDlnJMQBKnQsRTfrvZhZ2667371 = -823690476;    double eDlnJMQBKnQsRTfrvZhZ67089471 = -502275553;    double eDlnJMQBKnQsRTfrvZhZ70828437 = -342095452;    double eDlnJMQBKnQsRTfrvZhZ31392405 = -74644051;    double eDlnJMQBKnQsRTfrvZhZ12497321 = -55273211;    double eDlnJMQBKnQsRTfrvZhZ2669818 = -487308468;    double eDlnJMQBKnQsRTfrvZhZ80279598 = -668767936;    double eDlnJMQBKnQsRTfrvZhZ67379955 = -747569874;    double eDlnJMQBKnQsRTfrvZhZ1458980 = -489017174;    double eDlnJMQBKnQsRTfrvZhZ48534365 = -726702430;    double eDlnJMQBKnQsRTfrvZhZ37324495 = -975185748;    double eDlnJMQBKnQsRTfrvZhZ39341304 = -773639341;    double eDlnJMQBKnQsRTfrvZhZ58682743 = 67665789;    double eDlnJMQBKnQsRTfrvZhZ22335021 = -80875851;    double eDlnJMQBKnQsRTfrvZhZ25782419 = -130449647;    double eDlnJMQBKnQsRTfrvZhZ45883644 = -757272793;    double eDlnJMQBKnQsRTfrvZhZ91937962 = -312568836;    double eDlnJMQBKnQsRTfrvZhZ86073801 = -594680932;    double eDlnJMQBKnQsRTfrvZhZ72742413 = -258473068;    double eDlnJMQBKnQsRTfrvZhZ71959303 = -981279826;    double eDlnJMQBKnQsRTfrvZhZ27239663 = -892278244;    double eDlnJMQBKnQsRTfrvZhZ75989775 = -750090597;    double eDlnJMQBKnQsRTfrvZhZ92037920 = -444231799;    double eDlnJMQBKnQsRTfrvZhZ3489137 = -700906560;    double eDlnJMQBKnQsRTfrvZhZ43066251 = -648148924;    double eDlnJMQBKnQsRTfrvZhZ32579597 = -483547763;    double eDlnJMQBKnQsRTfrvZhZ918541 = -446263074;    double eDlnJMQBKnQsRTfrvZhZ21633895 = -289596380;     eDlnJMQBKnQsRTfrvZhZ41955904 = eDlnJMQBKnQsRTfrvZhZ95952702;     eDlnJMQBKnQsRTfrvZhZ95952702 = eDlnJMQBKnQsRTfrvZhZ34346786;     eDlnJMQBKnQsRTfrvZhZ34346786 = eDlnJMQBKnQsRTfrvZhZ59663455;     eDlnJMQBKnQsRTfrvZhZ59663455 = eDlnJMQBKnQsRTfrvZhZ73823789;     eDlnJMQBKnQsRTfrvZhZ73823789 = eDlnJMQBKnQsRTfrvZhZ77421594;     eDlnJMQBKnQsRTfrvZhZ77421594 = eDlnJMQBKnQsRTfrvZhZ68714284;     eDlnJMQBKnQsRTfrvZhZ68714284 = eDlnJMQBKnQsRTfrvZhZ23534299;     eDlnJMQBKnQsRTfrvZhZ23534299 = eDlnJMQBKnQsRTfrvZhZ84814238;     eDlnJMQBKnQsRTfrvZhZ84814238 = eDlnJMQBKnQsRTfrvZhZ29481939;     eDlnJMQBKnQsRTfrvZhZ29481939 = eDlnJMQBKnQsRTfrvZhZ28559994;     eDlnJMQBKnQsRTfrvZhZ28559994 = eDlnJMQBKnQsRTfrvZhZ94363620;     eDlnJMQBKnQsRTfrvZhZ94363620 = eDlnJMQBKnQsRTfrvZhZ53814614;     eDlnJMQBKnQsRTfrvZhZ53814614 = eDlnJMQBKnQsRTfrvZhZ830732;     eDlnJMQBKnQsRTfrvZhZ830732 = eDlnJMQBKnQsRTfrvZhZ43845251;     eDlnJMQBKnQsRTfrvZhZ43845251 = eDlnJMQBKnQsRTfrvZhZ42610493;     eDlnJMQBKnQsRTfrvZhZ42610493 = eDlnJMQBKnQsRTfrvZhZ73944178;     eDlnJMQBKnQsRTfrvZhZ73944178 = eDlnJMQBKnQsRTfrvZhZ31684146;     eDlnJMQBKnQsRTfrvZhZ31684146 = eDlnJMQBKnQsRTfrvZhZ45226369;     eDlnJMQBKnQsRTfrvZhZ45226369 = eDlnJMQBKnQsRTfrvZhZ44852433;     eDlnJMQBKnQsRTfrvZhZ44852433 = eDlnJMQBKnQsRTfrvZhZ52096798;     eDlnJMQBKnQsRTfrvZhZ52096798 = eDlnJMQBKnQsRTfrvZhZ35471564;     eDlnJMQBKnQsRTfrvZhZ35471564 = eDlnJMQBKnQsRTfrvZhZ47156753;     eDlnJMQBKnQsRTfrvZhZ47156753 = eDlnJMQBKnQsRTfrvZhZ53177844;     eDlnJMQBKnQsRTfrvZhZ53177844 = eDlnJMQBKnQsRTfrvZhZ76477273;     eDlnJMQBKnQsRTfrvZhZ76477273 = eDlnJMQBKnQsRTfrvZhZ20796790;     eDlnJMQBKnQsRTfrvZhZ20796790 = eDlnJMQBKnQsRTfrvZhZ91322615;     eDlnJMQBKnQsRTfrvZhZ91322615 = eDlnJMQBKnQsRTfrvZhZ98179425;     eDlnJMQBKnQsRTfrvZhZ98179425 = eDlnJMQBKnQsRTfrvZhZ45896157;     eDlnJMQBKnQsRTfrvZhZ45896157 = eDlnJMQBKnQsRTfrvZhZ94419591;     eDlnJMQBKnQsRTfrvZhZ94419591 = eDlnJMQBKnQsRTfrvZhZ85913544;     eDlnJMQBKnQsRTfrvZhZ85913544 = eDlnJMQBKnQsRTfrvZhZ77295642;     eDlnJMQBKnQsRTfrvZhZ77295642 = eDlnJMQBKnQsRTfrvZhZ49848450;     eDlnJMQBKnQsRTfrvZhZ49848450 = eDlnJMQBKnQsRTfrvZhZ88292565;     eDlnJMQBKnQsRTfrvZhZ88292565 = eDlnJMQBKnQsRTfrvZhZ92558141;     eDlnJMQBKnQsRTfrvZhZ92558141 = eDlnJMQBKnQsRTfrvZhZ21538363;     eDlnJMQBKnQsRTfrvZhZ21538363 = eDlnJMQBKnQsRTfrvZhZ53128246;     eDlnJMQBKnQsRTfrvZhZ53128246 = eDlnJMQBKnQsRTfrvZhZ10425845;     eDlnJMQBKnQsRTfrvZhZ10425845 = eDlnJMQBKnQsRTfrvZhZ29016775;     eDlnJMQBKnQsRTfrvZhZ29016775 = eDlnJMQBKnQsRTfrvZhZ78136898;     eDlnJMQBKnQsRTfrvZhZ78136898 = eDlnJMQBKnQsRTfrvZhZ74023996;     eDlnJMQBKnQsRTfrvZhZ74023996 = eDlnJMQBKnQsRTfrvZhZ20704393;     eDlnJMQBKnQsRTfrvZhZ20704393 = eDlnJMQBKnQsRTfrvZhZ22974244;     eDlnJMQBKnQsRTfrvZhZ22974244 = eDlnJMQBKnQsRTfrvZhZ44486935;     eDlnJMQBKnQsRTfrvZhZ44486935 = eDlnJMQBKnQsRTfrvZhZ72898246;     eDlnJMQBKnQsRTfrvZhZ72898246 = eDlnJMQBKnQsRTfrvZhZ9097318;     eDlnJMQBKnQsRTfrvZhZ9097318 = eDlnJMQBKnQsRTfrvZhZ19337811;     eDlnJMQBKnQsRTfrvZhZ19337811 = eDlnJMQBKnQsRTfrvZhZ42788251;     eDlnJMQBKnQsRTfrvZhZ42788251 = eDlnJMQBKnQsRTfrvZhZ60854930;     eDlnJMQBKnQsRTfrvZhZ60854930 = eDlnJMQBKnQsRTfrvZhZ6554853;     eDlnJMQBKnQsRTfrvZhZ6554853 = eDlnJMQBKnQsRTfrvZhZ35736849;     eDlnJMQBKnQsRTfrvZhZ35736849 = eDlnJMQBKnQsRTfrvZhZ63578523;     eDlnJMQBKnQsRTfrvZhZ63578523 = eDlnJMQBKnQsRTfrvZhZ51513224;     eDlnJMQBKnQsRTfrvZhZ51513224 = eDlnJMQBKnQsRTfrvZhZ3964807;     eDlnJMQBKnQsRTfrvZhZ3964807 = eDlnJMQBKnQsRTfrvZhZ96354603;     eDlnJMQBKnQsRTfrvZhZ96354603 = eDlnJMQBKnQsRTfrvZhZ6484340;     eDlnJMQBKnQsRTfrvZhZ6484340 = eDlnJMQBKnQsRTfrvZhZ48795949;     eDlnJMQBKnQsRTfrvZhZ48795949 = eDlnJMQBKnQsRTfrvZhZ81168942;     eDlnJMQBKnQsRTfrvZhZ81168942 = eDlnJMQBKnQsRTfrvZhZ83186182;     eDlnJMQBKnQsRTfrvZhZ83186182 = eDlnJMQBKnQsRTfrvZhZ53027000;     eDlnJMQBKnQsRTfrvZhZ53027000 = eDlnJMQBKnQsRTfrvZhZ86098978;     eDlnJMQBKnQsRTfrvZhZ86098978 = eDlnJMQBKnQsRTfrvZhZ70534859;     eDlnJMQBKnQsRTfrvZhZ70534859 = eDlnJMQBKnQsRTfrvZhZ77638142;     eDlnJMQBKnQsRTfrvZhZ77638142 = eDlnJMQBKnQsRTfrvZhZ90394647;     eDlnJMQBKnQsRTfrvZhZ90394647 = eDlnJMQBKnQsRTfrvZhZ43568395;     eDlnJMQBKnQsRTfrvZhZ43568395 = eDlnJMQBKnQsRTfrvZhZ51264351;     eDlnJMQBKnQsRTfrvZhZ51264351 = eDlnJMQBKnQsRTfrvZhZ44515171;     eDlnJMQBKnQsRTfrvZhZ44515171 = eDlnJMQBKnQsRTfrvZhZ65522048;     eDlnJMQBKnQsRTfrvZhZ65522048 = eDlnJMQBKnQsRTfrvZhZ8272591;     eDlnJMQBKnQsRTfrvZhZ8272591 = eDlnJMQBKnQsRTfrvZhZ22306889;     eDlnJMQBKnQsRTfrvZhZ22306889 = eDlnJMQBKnQsRTfrvZhZ89482247;     eDlnJMQBKnQsRTfrvZhZ89482247 = eDlnJMQBKnQsRTfrvZhZ63518333;     eDlnJMQBKnQsRTfrvZhZ63518333 = eDlnJMQBKnQsRTfrvZhZ2667371;     eDlnJMQBKnQsRTfrvZhZ2667371 = eDlnJMQBKnQsRTfrvZhZ67089471;     eDlnJMQBKnQsRTfrvZhZ67089471 = eDlnJMQBKnQsRTfrvZhZ70828437;     eDlnJMQBKnQsRTfrvZhZ70828437 = eDlnJMQBKnQsRTfrvZhZ31392405;     eDlnJMQBKnQsRTfrvZhZ31392405 = eDlnJMQBKnQsRTfrvZhZ12497321;     eDlnJMQBKnQsRTfrvZhZ12497321 = eDlnJMQBKnQsRTfrvZhZ2669818;     eDlnJMQBKnQsRTfrvZhZ2669818 = eDlnJMQBKnQsRTfrvZhZ80279598;     eDlnJMQBKnQsRTfrvZhZ80279598 = eDlnJMQBKnQsRTfrvZhZ67379955;     eDlnJMQBKnQsRTfrvZhZ67379955 = eDlnJMQBKnQsRTfrvZhZ1458980;     eDlnJMQBKnQsRTfrvZhZ1458980 = eDlnJMQBKnQsRTfrvZhZ48534365;     eDlnJMQBKnQsRTfrvZhZ48534365 = eDlnJMQBKnQsRTfrvZhZ37324495;     eDlnJMQBKnQsRTfrvZhZ37324495 = eDlnJMQBKnQsRTfrvZhZ39341304;     eDlnJMQBKnQsRTfrvZhZ39341304 = eDlnJMQBKnQsRTfrvZhZ58682743;     eDlnJMQBKnQsRTfrvZhZ58682743 = eDlnJMQBKnQsRTfrvZhZ22335021;     eDlnJMQBKnQsRTfrvZhZ22335021 = eDlnJMQBKnQsRTfrvZhZ25782419;     eDlnJMQBKnQsRTfrvZhZ25782419 = eDlnJMQBKnQsRTfrvZhZ45883644;     eDlnJMQBKnQsRTfrvZhZ45883644 = eDlnJMQBKnQsRTfrvZhZ91937962;     eDlnJMQBKnQsRTfrvZhZ91937962 = eDlnJMQBKnQsRTfrvZhZ86073801;     eDlnJMQBKnQsRTfrvZhZ86073801 = eDlnJMQBKnQsRTfrvZhZ72742413;     eDlnJMQBKnQsRTfrvZhZ72742413 = eDlnJMQBKnQsRTfrvZhZ71959303;     eDlnJMQBKnQsRTfrvZhZ71959303 = eDlnJMQBKnQsRTfrvZhZ27239663;     eDlnJMQBKnQsRTfrvZhZ27239663 = eDlnJMQBKnQsRTfrvZhZ75989775;     eDlnJMQBKnQsRTfrvZhZ75989775 = eDlnJMQBKnQsRTfrvZhZ92037920;     eDlnJMQBKnQsRTfrvZhZ92037920 = eDlnJMQBKnQsRTfrvZhZ3489137;     eDlnJMQBKnQsRTfrvZhZ3489137 = eDlnJMQBKnQsRTfrvZhZ43066251;     eDlnJMQBKnQsRTfrvZhZ43066251 = eDlnJMQBKnQsRTfrvZhZ32579597;     eDlnJMQBKnQsRTfrvZhZ32579597 = eDlnJMQBKnQsRTfrvZhZ918541;     eDlnJMQBKnQsRTfrvZhZ918541 = eDlnJMQBKnQsRTfrvZhZ21633895;     eDlnJMQBKnQsRTfrvZhZ21633895 = eDlnJMQBKnQsRTfrvZhZ41955904;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void cDlbTeHoXlFpuZqYswQl6362086() {     double XbQcsRyEucsCMEkjLcJF35273901 = -11505860;    double XbQcsRyEucsCMEkjLcJF29699726 = -32482629;    double XbQcsRyEucsCMEkjLcJF35262594 = -277058507;    double XbQcsRyEucsCMEkjLcJF48957132 = -583599557;    double XbQcsRyEucsCMEkjLcJF66888378 = -677745389;    double XbQcsRyEucsCMEkjLcJF95139590 = -92034876;    double XbQcsRyEucsCMEkjLcJF45090164 = -432343169;    double XbQcsRyEucsCMEkjLcJF85003178 = -422049004;    double XbQcsRyEucsCMEkjLcJF19532252 = -969066645;    double XbQcsRyEucsCMEkjLcJF57771920 = -83917409;    double XbQcsRyEucsCMEkjLcJF1824113 = -747838508;    double XbQcsRyEucsCMEkjLcJF22819513 = 99357338;    double XbQcsRyEucsCMEkjLcJF33586257 = -469642592;    double XbQcsRyEucsCMEkjLcJF42038770 = -616260980;    double XbQcsRyEucsCMEkjLcJF6625343 = -41653811;    double XbQcsRyEucsCMEkjLcJF4819913 = -674488718;    double XbQcsRyEucsCMEkjLcJF23733883 = -126685786;    double XbQcsRyEucsCMEkjLcJF91179879 = -383190372;    double XbQcsRyEucsCMEkjLcJF46752121 = -183433486;    double XbQcsRyEucsCMEkjLcJF66895627 = -648140574;    double XbQcsRyEucsCMEkjLcJF19814783 = -604802676;    double XbQcsRyEucsCMEkjLcJF49537300 = -673108337;    double XbQcsRyEucsCMEkjLcJF17744043 = -924004337;    double XbQcsRyEucsCMEkjLcJF79963304 = -379614345;    double XbQcsRyEucsCMEkjLcJF71168043 = -370932591;    double XbQcsRyEucsCMEkjLcJF97347973 = -811883604;    double XbQcsRyEucsCMEkjLcJF18810021 = -469489978;    double XbQcsRyEucsCMEkjLcJF14740676 = -716611275;    double XbQcsRyEucsCMEkjLcJF20768253 = -374739045;    double XbQcsRyEucsCMEkjLcJF62463852 = -897715164;    double XbQcsRyEucsCMEkjLcJF545455 = -439502960;    double XbQcsRyEucsCMEkjLcJF41035156 = -635275695;    double XbQcsRyEucsCMEkjLcJF84418296 = -301382974;    double XbQcsRyEucsCMEkjLcJF6612432 = 44951727;    double XbQcsRyEucsCMEkjLcJF35688337 = -416553185;    double XbQcsRyEucsCMEkjLcJF80696540 = 39080886;    double XbQcsRyEucsCMEkjLcJF21508856 = -108827518;    double XbQcsRyEucsCMEkjLcJF26647394 = -855870287;    double XbQcsRyEucsCMEkjLcJF67478836 = -132480065;    double XbQcsRyEucsCMEkjLcJF83119892 = -996779249;    double XbQcsRyEucsCMEkjLcJF93973246 = -190025137;    double XbQcsRyEucsCMEkjLcJF16547716 = -204469795;    double XbQcsRyEucsCMEkjLcJF86724472 = -935411639;    double XbQcsRyEucsCMEkjLcJF5729935 = -595366175;    double XbQcsRyEucsCMEkjLcJF72878325 = -702692386;    double XbQcsRyEucsCMEkjLcJF28264320 = -243207921;    double XbQcsRyEucsCMEkjLcJF14929951 = -802052179;    double XbQcsRyEucsCMEkjLcJF72072387 = -846297666;    double XbQcsRyEucsCMEkjLcJF63783388 = -881023438;    double XbQcsRyEucsCMEkjLcJF12606931 = -150817728;    double XbQcsRyEucsCMEkjLcJF79628773 = -73452871;    double XbQcsRyEucsCMEkjLcJF75306210 = 58631069;    double XbQcsRyEucsCMEkjLcJF16173228 = -390349711;    double XbQcsRyEucsCMEkjLcJF75734503 = -818321269;    double XbQcsRyEucsCMEkjLcJF86547302 = -162214277;    double XbQcsRyEucsCMEkjLcJF85736600 = -338397524;    double XbQcsRyEucsCMEkjLcJF11955684 = -108478292;    double XbQcsRyEucsCMEkjLcJF55299289 = -897444162;    double XbQcsRyEucsCMEkjLcJF77789088 = -112666966;    double XbQcsRyEucsCMEkjLcJF69540404 = -865861785;    double XbQcsRyEucsCMEkjLcJF76329569 = -622544898;    double XbQcsRyEucsCMEkjLcJF30349489 = -715731895;    double XbQcsRyEucsCMEkjLcJF64234925 = 52690041;    double XbQcsRyEucsCMEkjLcJF57068399 = 28648518;    double XbQcsRyEucsCMEkjLcJF57226466 = -644414449;    double XbQcsRyEucsCMEkjLcJF60788957 = -12562814;    double XbQcsRyEucsCMEkjLcJF38401217 = -599259689;    double XbQcsRyEucsCMEkjLcJF26973826 = -414594320;    double XbQcsRyEucsCMEkjLcJF6350433 = -99707796;    double XbQcsRyEucsCMEkjLcJF25928803 = 19265302;    double XbQcsRyEucsCMEkjLcJF83311057 = -465661200;    double XbQcsRyEucsCMEkjLcJF97086488 = -270815500;    double XbQcsRyEucsCMEkjLcJF23701044 = -150710308;    double XbQcsRyEucsCMEkjLcJF63632228 = -186654238;    double XbQcsRyEucsCMEkjLcJF72922380 = -358115437;    double XbQcsRyEucsCMEkjLcJF3267068 = -300332882;    double XbQcsRyEucsCMEkjLcJF62812828 = -737696699;    double XbQcsRyEucsCMEkjLcJF12014108 = -228638162;    double XbQcsRyEucsCMEkjLcJF7084980 = -676921960;    double XbQcsRyEucsCMEkjLcJF42903723 = -27724671;    double XbQcsRyEucsCMEkjLcJF82418023 = 90168575;    double XbQcsRyEucsCMEkjLcJF46737634 = -623192313;    double XbQcsRyEucsCMEkjLcJF50957287 = -835587837;    double XbQcsRyEucsCMEkjLcJF8161323 = -123921318;    double XbQcsRyEucsCMEkjLcJF82835079 = -724262294;    double XbQcsRyEucsCMEkjLcJF25239245 = -398134030;    double XbQcsRyEucsCMEkjLcJF24861928 = -144925985;    double XbQcsRyEucsCMEkjLcJF8683793 = -483061706;    double XbQcsRyEucsCMEkjLcJF20065129 = -792833997;    double XbQcsRyEucsCMEkjLcJF49951737 = 21844338;    double XbQcsRyEucsCMEkjLcJF68740856 = -852440823;    double XbQcsRyEucsCMEkjLcJF66209566 = -211383356;    double XbQcsRyEucsCMEkjLcJF48858305 = -643203322;    double XbQcsRyEucsCMEkjLcJF97938432 = -266618280;    double XbQcsRyEucsCMEkjLcJF6790324 = -274234351;    double XbQcsRyEucsCMEkjLcJF63623758 = -474293242;    double XbQcsRyEucsCMEkjLcJF52312791 = -157159836;    double XbQcsRyEucsCMEkjLcJF29656073 = -864060158;    double XbQcsRyEucsCMEkjLcJF48503469 = -950951726;    double XbQcsRyEucsCMEkjLcJF12089368 = -11505860;     XbQcsRyEucsCMEkjLcJF35273901 = XbQcsRyEucsCMEkjLcJF29699726;     XbQcsRyEucsCMEkjLcJF29699726 = XbQcsRyEucsCMEkjLcJF35262594;     XbQcsRyEucsCMEkjLcJF35262594 = XbQcsRyEucsCMEkjLcJF48957132;     XbQcsRyEucsCMEkjLcJF48957132 = XbQcsRyEucsCMEkjLcJF66888378;     XbQcsRyEucsCMEkjLcJF66888378 = XbQcsRyEucsCMEkjLcJF95139590;     XbQcsRyEucsCMEkjLcJF95139590 = XbQcsRyEucsCMEkjLcJF45090164;     XbQcsRyEucsCMEkjLcJF45090164 = XbQcsRyEucsCMEkjLcJF85003178;     XbQcsRyEucsCMEkjLcJF85003178 = XbQcsRyEucsCMEkjLcJF19532252;     XbQcsRyEucsCMEkjLcJF19532252 = XbQcsRyEucsCMEkjLcJF57771920;     XbQcsRyEucsCMEkjLcJF57771920 = XbQcsRyEucsCMEkjLcJF1824113;     XbQcsRyEucsCMEkjLcJF1824113 = XbQcsRyEucsCMEkjLcJF22819513;     XbQcsRyEucsCMEkjLcJF22819513 = XbQcsRyEucsCMEkjLcJF33586257;     XbQcsRyEucsCMEkjLcJF33586257 = XbQcsRyEucsCMEkjLcJF42038770;     XbQcsRyEucsCMEkjLcJF42038770 = XbQcsRyEucsCMEkjLcJF6625343;     XbQcsRyEucsCMEkjLcJF6625343 = XbQcsRyEucsCMEkjLcJF4819913;     XbQcsRyEucsCMEkjLcJF4819913 = XbQcsRyEucsCMEkjLcJF23733883;     XbQcsRyEucsCMEkjLcJF23733883 = XbQcsRyEucsCMEkjLcJF91179879;     XbQcsRyEucsCMEkjLcJF91179879 = XbQcsRyEucsCMEkjLcJF46752121;     XbQcsRyEucsCMEkjLcJF46752121 = XbQcsRyEucsCMEkjLcJF66895627;     XbQcsRyEucsCMEkjLcJF66895627 = XbQcsRyEucsCMEkjLcJF19814783;     XbQcsRyEucsCMEkjLcJF19814783 = XbQcsRyEucsCMEkjLcJF49537300;     XbQcsRyEucsCMEkjLcJF49537300 = XbQcsRyEucsCMEkjLcJF17744043;     XbQcsRyEucsCMEkjLcJF17744043 = XbQcsRyEucsCMEkjLcJF79963304;     XbQcsRyEucsCMEkjLcJF79963304 = XbQcsRyEucsCMEkjLcJF71168043;     XbQcsRyEucsCMEkjLcJF71168043 = XbQcsRyEucsCMEkjLcJF97347973;     XbQcsRyEucsCMEkjLcJF97347973 = XbQcsRyEucsCMEkjLcJF18810021;     XbQcsRyEucsCMEkjLcJF18810021 = XbQcsRyEucsCMEkjLcJF14740676;     XbQcsRyEucsCMEkjLcJF14740676 = XbQcsRyEucsCMEkjLcJF20768253;     XbQcsRyEucsCMEkjLcJF20768253 = XbQcsRyEucsCMEkjLcJF62463852;     XbQcsRyEucsCMEkjLcJF62463852 = XbQcsRyEucsCMEkjLcJF545455;     XbQcsRyEucsCMEkjLcJF545455 = XbQcsRyEucsCMEkjLcJF41035156;     XbQcsRyEucsCMEkjLcJF41035156 = XbQcsRyEucsCMEkjLcJF84418296;     XbQcsRyEucsCMEkjLcJF84418296 = XbQcsRyEucsCMEkjLcJF6612432;     XbQcsRyEucsCMEkjLcJF6612432 = XbQcsRyEucsCMEkjLcJF35688337;     XbQcsRyEucsCMEkjLcJF35688337 = XbQcsRyEucsCMEkjLcJF80696540;     XbQcsRyEucsCMEkjLcJF80696540 = XbQcsRyEucsCMEkjLcJF21508856;     XbQcsRyEucsCMEkjLcJF21508856 = XbQcsRyEucsCMEkjLcJF26647394;     XbQcsRyEucsCMEkjLcJF26647394 = XbQcsRyEucsCMEkjLcJF67478836;     XbQcsRyEucsCMEkjLcJF67478836 = XbQcsRyEucsCMEkjLcJF83119892;     XbQcsRyEucsCMEkjLcJF83119892 = XbQcsRyEucsCMEkjLcJF93973246;     XbQcsRyEucsCMEkjLcJF93973246 = XbQcsRyEucsCMEkjLcJF16547716;     XbQcsRyEucsCMEkjLcJF16547716 = XbQcsRyEucsCMEkjLcJF86724472;     XbQcsRyEucsCMEkjLcJF86724472 = XbQcsRyEucsCMEkjLcJF5729935;     XbQcsRyEucsCMEkjLcJF5729935 = XbQcsRyEucsCMEkjLcJF72878325;     XbQcsRyEucsCMEkjLcJF72878325 = XbQcsRyEucsCMEkjLcJF28264320;     XbQcsRyEucsCMEkjLcJF28264320 = XbQcsRyEucsCMEkjLcJF14929951;     XbQcsRyEucsCMEkjLcJF14929951 = XbQcsRyEucsCMEkjLcJF72072387;     XbQcsRyEucsCMEkjLcJF72072387 = XbQcsRyEucsCMEkjLcJF63783388;     XbQcsRyEucsCMEkjLcJF63783388 = XbQcsRyEucsCMEkjLcJF12606931;     XbQcsRyEucsCMEkjLcJF12606931 = XbQcsRyEucsCMEkjLcJF79628773;     XbQcsRyEucsCMEkjLcJF79628773 = XbQcsRyEucsCMEkjLcJF75306210;     XbQcsRyEucsCMEkjLcJF75306210 = XbQcsRyEucsCMEkjLcJF16173228;     XbQcsRyEucsCMEkjLcJF16173228 = XbQcsRyEucsCMEkjLcJF75734503;     XbQcsRyEucsCMEkjLcJF75734503 = XbQcsRyEucsCMEkjLcJF86547302;     XbQcsRyEucsCMEkjLcJF86547302 = XbQcsRyEucsCMEkjLcJF85736600;     XbQcsRyEucsCMEkjLcJF85736600 = XbQcsRyEucsCMEkjLcJF11955684;     XbQcsRyEucsCMEkjLcJF11955684 = XbQcsRyEucsCMEkjLcJF55299289;     XbQcsRyEucsCMEkjLcJF55299289 = XbQcsRyEucsCMEkjLcJF77789088;     XbQcsRyEucsCMEkjLcJF77789088 = XbQcsRyEucsCMEkjLcJF69540404;     XbQcsRyEucsCMEkjLcJF69540404 = XbQcsRyEucsCMEkjLcJF76329569;     XbQcsRyEucsCMEkjLcJF76329569 = XbQcsRyEucsCMEkjLcJF30349489;     XbQcsRyEucsCMEkjLcJF30349489 = XbQcsRyEucsCMEkjLcJF64234925;     XbQcsRyEucsCMEkjLcJF64234925 = XbQcsRyEucsCMEkjLcJF57068399;     XbQcsRyEucsCMEkjLcJF57068399 = XbQcsRyEucsCMEkjLcJF57226466;     XbQcsRyEucsCMEkjLcJF57226466 = XbQcsRyEucsCMEkjLcJF60788957;     XbQcsRyEucsCMEkjLcJF60788957 = XbQcsRyEucsCMEkjLcJF38401217;     XbQcsRyEucsCMEkjLcJF38401217 = XbQcsRyEucsCMEkjLcJF26973826;     XbQcsRyEucsCMEkjLcJF26973826 = XbQcsRyEucsCMEkjLcJF6350433;     XbQcsRyEucsCMEkjLcJF6350433 = XbQcsRyEucsCMEkjLcJF25928803;     XbQcsRyEucsCMEkjLcJF25928803 = XbQcsRyEucsCMEkjLcJF83311057;     XbQcsRyEucsCMEkjLcJF83311057 = XbQcsRyEucsCMEkjLcJF97086488;     XbQcsRyEucsCMEkjLcJF97086488 = XbQcsRyEucsCMEkjLcJF23701044;     XbQcsRyEucsCMEkjLcJF23701044 = XbQcsRyEucsCMEkjLcJF63632228;     XbQcsRyEucsCMEkjLcJF63632228 = XbQcsRyEucsCMEkjLcJF72922380;     XbQcsRyEucsCMEkjLcJF72922380 = XbQcsRyEucsCMEkjLcJF3267068;     XbQcsRyEucsCMEkjLcJF3267068 = XbQcsRyEucsCMEkjLcJF62812828;     XbQcsRyEucsCMEkjLcJF62812828 = XbQcsRyEucsCMEkjLcJF12014108;     XbQcsRyEucsCMEkjLcJF12014108 = XbQcsRyEucsCMEkjLcJF7084980;     XbQcsRyEucsCMEkjLcJF7084980 = XbQcsRyEucsCMEkjLcJF42903723;     XbQcsRyEucsCMEkjLcJF42903723 = XbQcsRyEucsCMEkjLcJF82418023;     XbQcsRyEucsCMEkjLcJF82418023 = XbQcsRyEucsCMEkjLcJF46737634;     XbQcsRyEucsCMEkjLcJF46737634 = XbQcsRyEucsCMEkjLcJF50957287;     XbQcsRyEucsCMEkjLcJF50957287 = XbQcsRyEucsCMEkjLcJF8161323;     XbQcsRyEucsCMEkjLcJF8161323 = XbQcsRyEucsCMEkjLcJF82835079;     XbQcsRyEucsCMEkjLcJF82835079 = XbQcsRyEucsCMEkjLcJF25239245;     XbQcsRyEucsCMEkjLcJF25239245 = XbQcsRyEucsCMEkjLcJF24861928;     XbQcsRyEucsCMEkjLcJF24861928 = XbQcsRyEucsCMEkjLcJF8683793;     XbQcsRyEucsCMEkjLcJF8683793 = XbQcsRyEucsCMEkjLcJF20065129;     XbQcsRyEucsCMEkjLcJF20065129 = XbQcsRyEucsCMEkjLcJF49951737;     XbQcsRyEucsCMEkjLcJF49951737 = XbQcsRyEucsCMEkjLcJF68740856;     XbQcsRyEucsCMEkjLcJF68740856 = XbQcsRyEucsCMEkjLcJF66209566;     XbQcsRyEucsCMEkjLcJF66209566 = XbQcsRyEucsCMEkjLcJF48858305;     XbQcsRyEucsCMEkjLcJF48858305 = XbQcsRyEucsCMEkjLcJF97938432;     XbQcsRyEucsCMEkjLcJF97938432 = XbQcsRyEucsCMEkjLcJF6790324;     XbQcsRyEucsCMEkjLcJF6790324 = XbQcsRyEucsCMEkjLcJF63623758;     XbQcsRyEucsCMEkjLcJF63623758 = XbQcsRyEucsCMEkjLcJF52312791;     XbQcsRyEucsCMEkjLcJF52312791 = XbQcsRyEucsCMEkjLcJF29656073;     XbQcsRyEucsCMEkjLcJF29656073 = XbQcsRyEucsCMEkjLcJF48503469;     XbQcsRyEucsCMEkjLcJF48503469 = XbQcsRyEucsCMEkjLcJF12089368;     XbQcsRyEucsCMEkjLcJF12089368 = XbQcsRyEucsCMEkjLcJF35273901;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void dTtgUnJhktKFAjrUFpGD33956342() {     double WvlBetKfrlwJapgaxriR60859177 = -536538961;    double WvlBetKfrlwJapgaxriR26096870 = -287960405;    double WvlBetKfrlwJapgaxriR68141770 = -79520043;    double WvlBetKfrlwJapgaxriR94679061 = -590922450;    double WvlBetKfrlwJapgaxriR36230383 = -724667676;    double WvlBetKfrlwJapgaxriR12148867 = -247015879;    double WvlBetKfrlwJapgaxriR50411008 = 85385624;    double WvlBetKfrlwJapgaxriR13303 = -802730883;    double WvlBetKfrlwJapgaxriR44861544 = -365290689;    double WvlBetKfrlwJapgaxriR76930303 = -418478932;    double WvlBetKfrlwJapgaxriR16157667 = 41733605;    double WvlBetKfrlwJapgaxriR58137168 = -344351569;    double WvlBetKfrlwJapgaxriR10167035 = -233890958;    double WvlBetKfrlwJapgaxriR33598487 = -590785274;    double WvlBetKfrlwJapgaxriR62894231 = 69800231;    double WvlBetKfrlwJapgaxriR12540956 = -10680195;    double WvlBetKfrlwJapgaxriR23531999 = -915005580;    double WvlBetKfrlwJapgaxriR12295784 = -899467733;    double WvlBetKfrlwJapgaxriR8216842 = -966407344;    double WvlBetKfrlwJapgaxriR28057094 = -622579949;    double WvlBetKfrlwJapgaxriR20824048 = 33497543;    double WvlBetKfrlwJapgaxriR51040407 = 7158734;    double WvlBetKfrlwJapgaxriR21507840 = -968096442;    double WvlBetKfrlwJapgaxriR81677347 = -238463325;    double WvlBetKfrlwJapgaxriR26071183 = 61571267;    double WvlBetKfrlwJapgaxriR26837111 = -376334146;    double WvlBetKfrlwJapgaxriR1197931 = -35303113;    double WvlBetKfrlwJapgaxriR78639475 = -946273507;    double WvlBetKfrlwJapgaxriR28645466 = -611032896;    double WvlBetKfrlwJapgaxriR15786344 = -806624725;    double WvlBetKfrlwJapgaxriR22592090 = -218149991;    double WvlBetKfrlwJapgaxriR78225088 = -909071529;    double WvlBetKfrlwJapgaxriR5605348 = -749204111;    double WvlBetKfrlwJapgaxriR60199503 = -11193245;    double WvlBetKfrlwJapgaxriR37093325 = -17989096;    double WvlBetKfrlwJapgaxriR85488390 = -478513830;    double WvlBetKfrlwJapgaxriR71154241 = -589339462;    double WvlBetKfrlwJapgaxriR2220081 = -704585112;    double WvlBetKfrlwJapgaxriR8402415 = -590818387;    double WvlBetKfrlwJapgaxriR91903566 = -366749568;    double WvlBetKfrlwJapgaxriR41124528 = -765085327;    double WvlBetKfrlwJapgaxriR60557306 = -713508299;    double WvlBetKfrlwJapgaxriR35924692 = -964018020;    double WvlBetKfrlwJapgaxriR76523213 = 36218226;    double WvlBetKfrlwJapgaxriR64859201 = -641713503;    double WvlBetKfrlwJapgaxriR94664642 = -765755458;    double WvlBetKfrlwJapgaxriR14698405 = -834521040;    double WvlBetKfrlwJapgaxriR80185158 = 60519487;    double WvlBetKfrlwJapgaxriR14594709 = -980699664;    double WvlBetKfrlwJapgaxriR66416926 = 1159119;    double WvlBetKfrlwJapgaxriR89765020 = -322111473;    double WvlBetKfrlwJapgaxriR2564790 = -119448111;    double WvlBetKfrlwJapgaxriR34246832 = -958248259;    double WvlBetKfrlwJapgaxriR44633412 = -33385050;    double WvlBetKfrlwJapgaxriR13132294 = -637304694;    double WvlBetKfrlwJapgaxriR9818771 = -443697695;    double WvlBetKfrlwJapgaxriR4589030 = -319863964;    double WvlBetKfrlwJapgaxriR86464422 = -841056719;    double WvlBetKfrlwJapgaxriR68607879 = -552493717;    double WvlBetKfrlwJapgaxriR9393273 = -248333531;    double WvlBetKfrlwJapgaxriR10950937 = -111712767;    double WvlBetKfrlwJapgaxriR71771532 = 31659130;    double WvlBetKfrlwJapgaxriR71367837 = -91697987;    double WvlBetKfrlwJapgaxriR29075201 = -558665965;    double WvlBetKfrlwJapgaxriR54338214 = -100328942;    double WvlBetKfrlwJapgaxriR37932579 = -49194867;    double WvlBetKfrlwJapgaxriR52531820 = -595147458;    double WvlBetKfrlwJapgaxriR49967532 = -122697714;    double WvlBetKfrlwJapgaxriR96505161 = -472796178;    double WvlBetKfrlwJapgaxriR77405840 = -451685939;    double WvlBetKfrlwJapgaxriR41386714 = -421340733;    double WvlBetKfrlwJapgaxriR21311919 = -110420469;    double WvlBetKfrlwJapgaxriR3893370 = -208649347;    double WvlBetKfrlwJapgaxriR16313276 = -499657776;    double WvlBetKfrlwJapgaxriR86932565 = -857494623;    double WvlBetKfrlwJapgaxriR60266742 = -252994159;    double WvlBetKfrlwJapgaxriR15115716 = -28823247;    double WvlBetKfrlwJapgaxriR44984627 = -904314669;    double WvlBetKfrlwJapgaxriR16818146 = -596749822;    double WvlBetKfrlwJapgaxriR31406541 = -172673276;    double WvlBetKfrlwJapgaxriR12138706 = -541813106;    double WvlBetKfrlwJapgaxriR21012773 = 4177399;    double WvlBetKfrlwJapgaxriR64044767 = -965573843;    double WvlBetKfrlwJapgaxriR62228540 = -512192015;    double WvlBetKfrlwJapgaxriR26021323 = -384513253;    double WvlBetKfrlwJapgaxriR20027300 = 1298119;    double WvlBetKfrlwJapgaxriR43978256 = -950823270;    double WvlBetKfrlwJapgaxriR60971935 = -615819061;    double WvlBetKfrlwJapgaxriR47067209 = -373888551;    double WvlBetKfrlwJapgaxriR27274555 = -574291402;    double WvlBetKfrlwJapgaxriR80899361 = -58649867;    double WvlBetKfrlwJapgaxriR84689818 = -748282744;    double WvlBetKfrlwJapgaxriR33612201 = -52091396;    double WvlBetKfrlwJapgaxriR99009142 = -242484857;    double WvlBetKfrlwJapgaxriR80952629 = -155036802;    double WvlBetKfrlwJapgaxriR69352995 = -696744458;    double WvlBetKfrlwJapgaxriR89189468 = -521810312;    double WvlBetKfrlwJapgaxriR6849491 = -305352056;    double WvlBetKfrlwJapgaxriR22185000 = -863452833;    double WvlBetKfrlwJapgaxriR26926623 = -536538961;     WvlBetKfrlwJapgaxriR60859177 = WvlBetKfrlwJapgaxriR26096870;     WvlBetKfrlwJapgaxriR26096870 = WvlBetKfrlwJapgaxriR68141770;     WvlBetKfrlwJapgaxriR68141770 = WvlBetKfrlwJapgaxriR94679061;     WvlBetKfrlwJapgaxriR94679061 = WvlBetKfrlwJapgaxriR36230383;     WvlBetKfrlwJapgaxriR36230383 = WvlBetKfrlwJapgaxriR12148867;     WvlBetKfrlwJapgaxriR12148867 = WvlBetKfrlwJapgaxriR50411008;     WvlBetKfrlwJapgaxriR50411008 = WvlBetKfrlwJapgaxriR13303;     WvlBetKfrlwJapgaxriR13303 = WvlBetKfrlwJapgaxriR44861544;     WvlBetKfrlwJapgaxriR44861544 = WvlBetKfrlwJapgaxriR76930303;     WvlBetKfrlwJapgaxriR76930303 = WvlBetKfrlwJapgaxriR16157667;     WvlBetKfrlwJapgaxriR16157667 = WvlBetKfrlwJapgaxriR58137168;     WvlBetKfrlwJapgaxriR58137168 = WvlBetKfrlwJapgaxriR10167035;     WvlBetKfrlwJapgaxriR10167035 = WvlBetKfrlwJapgaxriR33598487;     WvlBetKfrlwJapgaxriR33598487 = WvlBetKfrlwJapgaxriR62894231;     WvlBetKfrlwJapgaxriR62894231 = WvlBetKfrlwJapgaxriR12540956;     WvlBetKfrlwJapgaxriR12540956 = WvlBetKfrlwJapgaxriR23531999;     WvlBetKfrlwJapgaxriR23531999 = WvlBetKfrlwJapgaxriR12295784;     WvlBetKfrlwJapgaxriR12295784 = WvlBetKfrlwJapgaxriR8216842;     WvlBetKfrlwJapgaxriR8216842 = WvlBetKfrlwJapgaxriR28057094;     WvlBetKfrlwJapgaxriR28057094 = WvlBetKfrlwJapgaxriR20824048;     WvlBetKfrlwJapgaxriR20824048 = WvlBetKfrlwJapgaxriR51040407;     WvlBetKfrlwJapgaxriR51040407 = WvlBetKfrlwJapgaxriR21507840;     WvlBetKfrlwJapgaxriR21507840 = WvlBetKfrlwJapgaxriR81677347;     WvlBetKfrlwJapgaxriR81677347 = WvlBetKfrlwJapgaxriR26071183;     WvlBetKfrlwJapgaxriR26071183 = WvlBetKfrlwJapgaxriR26837111;     WvlBetKfrlwJapgaxriR26837111 = WvlBetKfrlwJapgaxriR1197931;     WvlBetKfrlwJapgaxriR1197931 = WvlBetKfrlwJapgaxriR78639475;     WvlBetKfrlwJapgaxriR78639475 = WvlBetKfrlwJapgaxriR28645466;     WvlBetKfrlwJapgaxriR28645466 = WvlBetKfrlwJapgaxriR15786344;     WvlBetKfrlwJapgaxriR15786344 = WvlBetKfrlwJapgaxriR22592090;     WvlBetKfrlwJapgaxriR22592090 = WvlBetKfrlwJapgaxriR78225088;     WvlBetKfrlwJapgaxriR78225088 = WvlBetKfrlwJapgaxriR5605348;     WvlBetKfrlwJapgaxriR5605348 = WvlBetKfrlwJapgaxriR60199503;     WvlBetKfrlwJapgaxriR60199503 = WvlBetKfrlwJapgaxriR37093325;     WvlBetKfrlwJapgaxriR37093325 = WvlBetKfrlwJapgaxriR85488390;     WvlBetKfrlwJapgaxriR85488390 = WvlBetKfrlwJapgaxriR71154241;     WvlBetKfrlwJapgaxriR71154241 = WvlBetKfrlwJapgaxriR2220081;     WvlBetKfrlwJapgaxriR2220081 = WvlBetKfrlwJapgaxriR8402415;     WvlBetKfrlwJapgaxriR8402415 = WvlBetKfrlwJapgaxriR91903566;     WvlBetKfrlwJapgaxriR91903566 = WvlBetKfrlwJapgaxriR41124528;     WvlBetKfrlwJapgaxriR41124528 = WvlBetKfrlwJapgaxriR60557306;     WvlBetKfrlwJapgaxriR60557306 = WvlBetKfrlwJapgaxriR35924692;     WvlBetKfrlwJapgaxriR35924692 = WvlBetKfrlwJapgaxriR76523213;     WvlBetKfrlwJapgaxriR76523213 = WvlBetKfrlwJapgaxriR64859201;     WvlBetKfrlwJapgaxriR64859201 = WvlBetKfrlwJapgaxriR94664642;     WvlBetKfrlwJapgaxriR94664642 = WvlBetKfrlwJapgaxriR14698405;     WvlBetKfrlwJapgaxriR14698405 = WvlBetKfrlwJapgaxriR80185158;     WvlBetKfrlwJapgaxriR80185158 = WvlBetKfrlwJapgaxriR14594709;     WvlBetKfrlwJapgaxriR14594709 = WvlBetKfrlwJapgaxriR66416926;     WvlBetKfrlwJapgaxriR66416926 = WvlBetKfrlwJapgaxriR89765020;     WvlBetKfrlwJapgaxriR89765020 = WvlBetKfrlwJapgaxriR2564790;     WvlBetKfrlwJapgaxriR2564790 = WvlBetKfrlwJapgaxriR34246832;     WvlBetKfrlwJapgaxriR34246832 = WvlBetKfrlwJapgaxriR44633412;     WvlBetKfrlwJapgaxriR44633412 = WvlBetKfrlwJapgaxriR13132294;     WvlBetKfrlwJapgaxriR13132294 = WvlBetKfrlwJapgaxriR9818771;     WvlBetKfrlwJapgaxriR9818771 = WvlBetKfrlwJapgaxriR4589030;     WvlBetKfrlwJapgaxriR4589030 = WvlBetKfrlwJapgaxriR86464422;     WvlBetKfrlwJapgaxriR86464422 = WvlBetKfrlwJapgaxriR68607879;     WvlBetKfrlwJapgaxriR68607879 = WvlBetKfrlwJapgaxriR9393273;     WvlBetKfrlwJapgaxriR9393273 = WvlBetKfrlwJapgaxriR10950937;     WvlBetKfrlwJapgaxriR10950937 = WvlBetKfrlwJapgaxriR71771532;     WvlBetKfrlwJapgaxriR71771532 = WvlBetKfrlwJapgaxriR71367837;     WvlBetKfrlwJapgaxriR71367837 = WvlBetKfrlwJapgaxriR29075201;     WvlBetKfrlwJapgaxriR29075201 = WvlBetKfrlwJapgaxriR54338214;     WvlBetKfrlwJapgaxriR54338214 = WvlBetKfrlwJapgaxriR37932579;     WvlBetKfrlwJapgaxriR37932579 = WvlBetKfrlwJapgaxriR52531820;     WvlBetKfrlwJapgaxriR52531820 = WvlBetKfrlwJapgaxriR49967532;     WvlBetKfrlwJapgaxriR49967532 = WvlBetKfrlwJapgaxriR96505161;     WvlBetKfrlwJapgaxriR96505161 = WvlBetKfrlwJapgaxriR77405840;     WvlBetKfrlwJapgaxriR77405840 = WvlBetKfrlwJapgaxriR41386714;     WvlBetKfrlwJapgaxriR41386714 = WvlBetKfrlwJapgaxriR21311919;     WvlBetKfrlwJapgaxriR21311919 = WvlBetKfrlwJapgaxriR3893370;     WvlBetKfrlwJapgaxriR3893370 = WvlBetKfrlwJapgaxriR16313276;     WvlBetKfrlwJapgaxriR16313276 = WvlBetKfrlwJapgaxriR86932565;     WvlBetKfrlwJapgaxriR86932565 = WvlBetKfrlwJapgaxriR60266742;     WvlBetKfrlwJapgaxriR60266742 = WvlBetKfrlwJapgaxriR15115716;     WvlBetKfrlwJapgaxriR15115716 = WvlBetKfrlwJapgaxriR44984627;     WvlBetKfrlwJapgaxriR44984627 = WvlBetKfrlwJapgaxriR16818146;     WvlBetKfrlwJapgaxriR16818146 = WvlBetKfrlwJapgaxriR31406541;     WvlBetKfrlwJapgaxriR31406541 = WvlBetKfrlwJapgaxriR12138706;     WvlBetKfrlwJapgaxriR12138706 = WvlBetKfrlwJapgaxriR21012773;     WvlBetKfrlwJapgaxriR21012773 = WvlBetKfrlwJapgaxriR64044767;     WvlBetKfrlwJapgaxriR64044767 = WvlBetKfrlwJapgaxriR62228540;     WvlBetKfrlwJapgaxriR62228540 = WvlBetKfrlwJapgaxriR26021323;     WvlBetKfrlwJapgaxriR26021323 = WvlBetKfrlwJapgaxriR20027300;     WvlBetKfrlwJapgaxriR20027300 = WvlBetKfrlwJapgaxriR43978256;     WvlBetKfrlwJapgaxriR43978256 = WvlBetKfrlwJapgaxriR60971935;     WvlBetKfrlwJapgaxriR60971935 = WvlBetKfrlwJapgaxriR47067209;     WvlBetKfrlwJapgaxriR47067209 = WvlBetKfrlwJapgaxriR27274555;     WvlBetKfrlwJapgaxriR27274555 = WvlBetKfrlwJapgaxriR80899361;     WvlBetKfrlwJapgaxriR80899361 = WvlBetKfrlwJapgaxriR84689818;     WvlBetKfrlwJapgaxriR84689818 = WvlBetKfrlwJapgaxriR33612201;     WvlBetKfrlwJapgaxriR33612201 = WvlBetKfrlwJapgaxriR99009142;     WvlBetKfrlwJapgaxriR99009142 = WvlBetKfrlwJapgaxriR80952629;     WvlBetKfrlwJapgaxriR80952629 = WvlBetKfrlwJapgaxriR69352995;     WvlBetKfrlwJapgaxriR69352995 = WvlBetKfrlwJapgaxriR89189468;     WvlBetKfrlwJapgaxriR89189468 = WvlBetKfrlwJapgaxriR6849491;     WvlBetKfrlwJapgaxriR6849491 = WvlBetKfrlwJapgaxriR22185000;     WvlBetKfrlwJapgaxriR22185000 = WvlBetKfrlwJapgaxriR26926623;     WvlBetKfrlwJapgaxriR26926623 = WvlBetKfrlwJapgaxriR60859177;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void rIBnolFtAPUDmAQLoyKz61550599() {     double UNGmQhJYPTzqsrJxxApX86444454 = 38427939;    double UNGmQhJYPTzqsrJxxApX22494013 = -543438182;    double UNGmQhJYPTzqsrJxxApX1020946 = -981981579;    double UNGmQhJYPTzqsrJxxApX40400992 = -598245343;    double UNGmQhJYPTzqsrJxxApX5572388 = -771589963;    double UNGmQhJYPTzqsrJxxApX29158143 = -401996882;    double UNGmQhJYPTzqsrJxxApX55731853 = -496885583;    double UNGmQhJYPTzqsrJxxApX15023427 = -83412762;    double UNGmQhJYPTzqsrJxxApX70190837 = -861514734;    double UNGmQhJYPTzqsrJxxApX96088685 = -753040456;    double UNGmQhJYPTzqsrJxxApX30491221 = -268694282;    double UNGmQhJYPTzqsrJxxApX93454824 = -788060475;    double UNGmQhJYPTzqsrJxxApX86747812 = 1860677;    double UNGmQhJYPTzqsrJxxApX25158204 = -565309567;    double UNGmQhJYPTzqsrJxxApX19163119 = -918745727;    double UNGmQhJYPTzqsrJxxApX20261999 = -446871672;    double UNGmQhJYPTzqsrJxxApX23330115 = -603325374;    double UNGmQhJYPTzqsrJxxApX33411689 = -315745094;    double UNGmQhJYPTzqsrJxxApX69681562 = -649381201;    double UNGmQhJYPTzqsrJxxApX89218560 = -597019325;    double UNGmQhJYPTzqsrJxxApX21833314 = -428202238;    double UNGmQhJYPTzqsrJxxApX52543514 = -412574196;    double UNGmQhJYPTzqsrJxxApX25271638 = 87811453;    double UNGmQhJYPTzqsrJxxApX83391389 = -97312304;    double UNGmQhJYPTzqsrJxxApX80974322 = -605924876;    double UNGmQhJYPTzqsrJxxApX56326247 = 59215313;    double UNGmQhJYPTzqsrJxxApX83585839 = -701116248;    double UNGmQhJYPTzqsrJxxApX42538276 = -75935740;    double UNGmQhJYPTzqsrJxxApX36522678 = -847326746;    double UNGmQhJYPTzqsrJxxApX69108834 = -715534287;    double UNGmQhJYPTzqsrJxxApX44638724 = 3202978;    double UNGmQhJYPTzqsrJxxApX15415021 = -82867363;    double UNGmQhJYPTzqsrJxxApX26792400 = -97025248;    double UNGmQhJYPTzqsrJxxApX13786574 = -67338216;    double UNGmQhJYPTzqsrJxxApX38498313 = -719425008;    double UNGmQhJYPTzqsrJxxApX90280241 = -996108546;    double UNGmQhJYPTzqsrJxxApX20799627 = 30148593;    double UNGmQhJYPTzqsrJxxApX77792767 = -553299937;    double UNGmQhJYPTzqsrJxxApX49325994 = 50843291;    double UNGmQhJYPTzqsrJxxApX687240 = -836719887;    double UNGmQhJYPTzqsrJxxApX88275809 = -240145517;    double UNGmQhJYPTzqsrJxxApX4566896 = -122546803;    double UNGmQhJYPTzqsrJxxApX85124911 = -992624402;    double UNGmQhJYPTzqsrJxxApX47316493 = -432197372;    double UNGmQhJYPTzqsrJxxApX56840077 = -580734620;    double UNGmQhJYPTzqsrJxxApX61064964 = -188302996;    double UNGmQhJYPTzqsrJxxApX14466859 = -866989900;    double UNGmQhJYPTzqsrJxxApX88297928 = -132663359;    double UNGmQhJYPTzqsrJxxApX65406028 = 19624109;    double UNGmQhJYPTzqsrJxxApX20226921 = -946864034;    double UNGmQhJYPTzqsrJxxApX99901267 = -570770075;    double UNGmQhJYPTzqsrJxxApX29823369 = -297527290;    double UNGmQhJYPTzqsrJxxApX52320435 = -426146808;    double UNGmQhJYPTzqsrJxxApX13532321 = -348448832;    double UNGmQhJYPTzqsrJxxApX39717286 = -12395112;    double UNGmQhJYPTzqsrJxxApX33900941 = -548997866;    double UNGmQhJYPTzqsrJxxApX97222374 = -531249635;    double UNGmQhJYPTzqsrJxxApX17629556 = -784669275;    double UNGmQhJYPTzqsrJxxApX59426669 = -992320468;    double UNGmQhJYPTzqsrJxxApX49246141 = -730805277;    double UNGmQhJYPTzqsrJxxApX45572304 = -700880635;    double UNGmQhJYPTzqsrJxxApX13193577 = -320949844;    double UNGmQhJYPTzqsrJxxApX78500748 = -236086016;    double UNGmQhJYPTzqsrJxxApX1082003 = -45980447;    double UNGmQhJYPTzqsrJxxApX51449962 = -656243434;    double UNGmQhJYPTzqsrJxxApX15076200 = -85826919;    double UNGmQhJYPTzqsrJxxApX66662424 = -591035228;    double UNGmQhJYPTzqsrJxxApX72961239 = -930801107;    double UNGmQhJYPTzqsrJxxApX86659890 = -845884560;    double UNGmQhJYPTzqsrJxxApX28882878 = -922637181;    double UNGmQhJYPTzqsrJxxApX99462371 = -377020265;    double UNGmQhJYPTzqsrJxxApX45537348 = 49974562;    double UNGmQhJYPTzqsrJxxApX84085695 = -266588385;    double UNGmQhJYPTzqsrJxxApX68994322 = -812661315;    double UNGmQhJYPTzqsrJxxApX942752 = -256873808;    double UNGmQhJYPTzqsrJxxApX17266418 = -205655436;    double UNGmQhJYPTzqsrJxxApX67418602 = -419949795;    double UNGmQhJYPTzqsrJxxApX77955145 = -479991175;    double UNGmQhJYPTzqsrJxxApX26551313 = -516577684;    double UNGmQhJYPTzqsrJxxApX19909358 = -317621880;    double UNGmQhJYPTzqsrJxxApX41859388 = -73794787;    double UNGmQhJYPTzqsrJxxApX95287910 = -468452889;    double UNGmQhJYPTzqsrJxxApX77132247 = 4440151;    double UNGmQhJYPTzqsrJxxApX16295757 = -900462712;    double UNGmQhJYPTzqsrJxxApX69207566 = -44764213;    double UNGmQhJYPTzqsrJxxApX14815355 = -699269732;    double UNGmQhJYPTzqsrJxxApX63094585 = -656720555;    double UNGmQhJYPTzqsrJxxApX13260079 = -748576417;    double UNGmQhJYPTzqsrJxxApX74069288 = 45056895;    double UNGmQhJYPTzqsrJxxApX4597373 = -70427142;    double UNGmQhJYPTzqsrJxxApX93057866 = -364858911;    double UNGmQhJYPTzqsrJxxApX3170071 = -185182132;    double UNGmQhJYPTzqsrJxxApX18366098 = -560979470;    double UNGmQhJYPTzqsrJxxApX79853 = -218351433;    double UNGmQhJYPTzqsrJxxApX55114936 = -35839252;    double UNGmQhJYPTzqsrJxxApX75082232 = -919195673;    double UNGmQhJYPTzqsrJxxApX26066147 = -886460787;    double UNGmQhJYPTzqsrJxxApX84042908 = -846643955;    double UNGmQhJYPTzqsrJxxApX95866531 = -775953939;    double UNGmQhJYPTzqsrJxxApX41763878 = 38427939;     UNGmQhJYPTzqsrJxxApX86444454 = UNGmQhJYPTzqsrJxxApX22494013;     UNGmQhJYPTzqsrJxxApX22494013 = UNGmQhJYPTzqsrJxxApX1020946;     UNGmQhJYPTzqsrJxxApX1020946 = UNGmQhJYPTzqsrJxxApX40400992;     UNGmQhJYPTzqsrJxxApX40400992 = UNGmQhJYPTzqsrJxxApX5572388;     UNGmQhJYPTzqsrJxxApX5572388 = UNGmQhJYPTzqsrJxxApX29158143;     UNGmQhJYPTzqsrJxxApX29158143 = UNGmQhJYPTzqsrJxxApX55731853;     UNGmQhJYPTzqsrJxxApX55731853 = UNGmQhJYPTzqsrJxxApX15023427;     UNGmQhJYPTzqsrJxxApX15023427 = UNGmQhJYPTzqsrJxxApX70190837;     UNGmQhJYPTzqsrJxxApX70190837 = UNGmQhJYPTzqsrJxxApX96088685;     UNGmQhJYPTzqsrJxxApX96088685 = UNGmQhJYPTzqsrJxxApX30491221;     UNGmQhJYPTzqsrJxxApX30491221 = UNGmQhJYPTzqsrJxxApX93454824;     UNGmQhJYPTzqsrJxxApX93454824 = UNGmQhJYPTzqsrJxxApX86747812;     UNGmQhJYPTzqsrJxxApX86747812 = UNGmQhJYPTzqsrJxxApX25158204;     UNGmQhJYPTzqsrJxxApX25158204 = UNGmQhJYPTzqsrJxxApX19163119;     UNGmQhJYPTzqsrJxxApX19163119 = UNGmQhJYPTzqsrJxxApX20261999;     UNGmQhJYPTzqsrJxxApX20261999 = UNGmQhJYPTzqsrJxxApX23330115;     UNGmQhJYPTzqsrJxxApX23330115 = UNGmQhJYPTzqsrJxxApX33411689;     UNGmQhJYPTzqsrJxxApX33411689 = UNGmQhJYPTzqsrJxxApX69681562;     UNGmQhJYPTzqsrJxxApX69681562 = UNGmQhJYPTzqsrJxxApX89218560;     UNGmQhJYPTzqsrJxxApX89218560 = UNGmQhJYPTzqsrJxxApX21833314;     UNGmQhJYPTzqsrJxxApX21833314 = UNGmQhJYPTzqsrJxxApX52543514;     UNGmQhJYPTzqsrJxxApX52543514 = UNGmQhJYPTzqsrJxxApX25271638;     UNGmQhJYPTzqsrJxxApX25271638 = UNGmQhJYPTzqsrJxxApX83391389;     UNGmQhJYPTzqsrJxxApX83391389 = UNGmQhJYPTzqsrJxxApX80974322;     UNGmQhJYPTzqsrJxxApX80974322 = UNGmQhJYPTzqsrJxxApX56326247;     UNGmQhJYPTzqsrJxxApX56326247 = UNGmQhJYPTzqsrJxxApX83585839;     UNGmQhJYPTzqsrJxxApX83585839 = UNGmQhJYPTzqsrJxxApX42538276;     UNGmQhJYPTzqsrJxxApX42538276 = UNGmQhJYPTzqsrJxxApX36522678;     UNGmQhJYPTzqsrJxxApX36522678 = UNGmQhJYPTzqsrJxxApX69108834;     UNGmQhJYPTzqsrJxxApX69108834 = UNGmQhJYPTzqsrJxxApX44638724;     UNGmQhJYPTzqsrJxxApX44638724 = UNGmQhJYPTzqsrJxxApX15415021;     UNGmQhJYPTzqsrJxxApX15415021 = UNGmQhJYPTzqsrJxxApX26792400;     UNGmQhJYPTzqsrJxxApX26792400 = UNGmQhJYPTzqsrJxxApX13786574;     UNGmQhJYPTzqsrJxxApX13786574 = UNGmQhJYPTzqsrJxxApX38498313;     UNGmQhJYPTzqsrJxxApX38498313 = UNGmQhJYPTzqsrJxxApX90280241;     UNGmQhJYPTzqsrJxxApX90280241 = UNGmQhJYPTzqsrJxxApX20799627;     UNGmQhJYPTzqsrJxxApX20799627 = UNGmQhJYPTzqsrJxxApX77792767;     UNGmQhJYPTzqsrJxxApX77792767 = UNGmQhJYPTzqsrJxxApX49325994;     UNGmQhJYPTzqsrJxxApX49325994 = UNGmQhJYPTzqsrJxxApX687240;     UNGmQhJYPTzqsrJxxApX687240 = UNGmQhJYPTzqsrJxxApX88275809;     UNGmQhJYPTzqsrJxxApX88275809 = UNGmQhJYPTzqsrJxxApX4566896;     UNGmQhJYPTzqsrJxxApX4566896 = UNGmQhJYPTzqsrJxxApX85124911;     UNGmQhJYPTzqsrJxxApX85124911 = UNGmQhJYPTzqsrJxxApX47316493;     UNGmQhJYPTzqsrJxxApX47316493 = UNGmQhJYPTzqsrJxxApX56840077;     UNGmQhJYPTzqsrJxxApX56840077 = UNGmQhJYPTzqsrJxxApX61064964;     UNGmQhJYPTzqsrJxxApX61064964 = UNGmQhJYPTzqsrJxxApX14466859;     UNGmQhJYPTzqsrJxxApX14466859 = UNGmQhJYPTzqsrJxxApX88297928;     UNGmQhJYPTzqsrJxxApX88297928 = UNGmQhJYPTzqsrJxxApX65406028;     UNGmQhJYPTzqsrJxxApX65406028 = UNGmQhJYPTzqsrJxxApX20226921;     UNGmQhJYPTzqsrJxxApX20226921 = UNGmQhJYPTzqsrJxxApX99901267;     UNGmQhJYPTzqsrJxxApX99901267 = UNGmQhJYPTzqsrJxxApX29823369;     UNGmQhJYPTzqsrJxxApX29823369 = UNGmQhJYPTzqsrJxxApX52320435;     UNGmQhJYPTzqsrJxxApX52320435 = UNGmQhJYPTzqsrJxxApX13532321;     UNGmQhJYPTzqsrJxxApX13532321 = UNGmQhJYPTzqsrJxxApX39717286;     UNGmQhJYPTzqsrJxxApX39717286 = UNGmQhJYPTzqsrJxxApX33900941;     UNGmQhJYPTzqsrJxxApX33900941 = UNGmQhJYPTzqsrJxxApX97222374;     UNGmQhJYPTzqsrJxxApX97222374 = UNGmQhJYPTzqsrJxxApX17629556;     UNGmQhJYPTzqsrJxxApX17629556 = UNGmQhJYPTzqsrJxxApX59426669;     UNGmQhJYPTzqsrJxxApX59426669 = UNGmQhJYPTzqsrJxxApX49246141;     UNGmQhJYPTzqsrJxxApX49246141 = UNGmQhJYPTzqsrJxxApX45572304;     UNGmQhJYPTzqsrJxxApX45572304 = UNGmQhJYPTzqsrJxxApX13193577;     UNGmQhJYPTzqsrJxxApX13193577 = UNGmQhJYPTzqsrJxxApX78500748;     UNGmQhJYPTzqsrJxxApX78500748 = UNGmQhJYPTzqsrJxxApX1082003;     UNGmQhJYPTzqsrJxxApX1082003 = UNGmQhJYPTzqsrJxxApX51449962;     UNGmQhJYPTzqsrJxxApX51449962 = UNGmQhJYPTzqsrJxxApX15076200;     UNGmQhJYPTzqsrJxxApX15076200 = UNGmQhJYPTzqsrJxxApX66662424;     UNGmQhJYPTzqsrJxxApX66662424 = UNGmQhJYPTzqsrJxxApX72961239;     UNGmQhJYPTzqsrJxxApX72961239 = UNGmQhJYPTzqsrJxxApX86659890;     UNGmQhJYPTzqsrJxxApX86659890 = UNGmQhJYPTzqsrJxxApX28882878;     UNGmQhJYPTzqsrJxxApX28882878 = UNGmQhJYPTzqsrJxxApX99462371;     UNGmQhJYPTzqsrJxxApX99462371 = UNGmQhJYPTzqsrJxxApX45537348;     UNGmQhJYPTzqsrJxxApX45537348 = UNGmQhJYPTzqsrJxxApX84085695;     UNGmQhJYPTzqsrJxxApX84085695 = UNGmQhJYPTzqsrJxxApX68994322;     UNGmQhJYPTzqsrJxxApX68994322 = UNGmQhJYPTzqsrJxxApX942752;     UNGmQhJYPTzqsrJxxApX942752 = UNGmQhJYPTzqsrJxxApX17266418;     UNGmQhJYPTzqsrJxxApX17266418 = UNGmQhJYPTzqsrJxxApX67418602;     UNGmQhJYPTzqsrJxxApX67418602 = UNGmQhJYPTzqsrJxxApX77955145;     UNGmQhJYPTzqsrJxxApX77955145 = UNGmQhJYPTzqsrJxxApX26551313;     UNGmQhJYPTzqsrJxxApX26551313 = UNGmQhJYPTzqsrJxxApX19909358;     UNGmQhJYPTzqsrJxxApX19909358 = UNGmQhJYPTzqsrJxxApX41859388;     UNGmQhJYPTzqsrJxxApX41859388 = UNGmQhJYPTzqsrJxxApX95287910;     UNGmQhJYPTzqsrJxxApX95287910 = UNGmQhJYPTzqsrJxxApX77132247;     UNGmQhJYPTzqsrJxxApX77132247 = UNGmQhJYPTzqsrJxxApX16295757;     UNGmQhJYPTzqsrJxxApX16295757 = UNGmQhJYPTzqsrJxxApX69207566;     UNGmQhJYPTzqsrJxxApX69207566 = UNGmQhJYPTzqsrJxxApX14815355;     UNGmQhJYPTzqsrJxxApX14815355 = UNGmQhJYPTzqsrJxxApX63094585;     UNGmQhJYPTzqsrJxxApX63094585 = UNGmQhJYPTzqsrJxxApX13260079;     UNGmQhJYPTzqsrJxxApX13260079 = UNGmQhJYPTzqsrJxxApX74069288;     UNGmQhJYPTzqsrJxxApX74069288 = UNGmQhJYPTzqsrJxxApX4597373;     UNGmQhJYPTzqsrJxxApX4597373 = UNGmQhJYPTzqsrJxxApX93057866;     UNGmQhJYPTzqsrJxxApX93057866 = UNGmQhJYPTzqsrJxxApX3170071;     UNGmQhJYPTzqsrJxxApX3170071 = UNGmQhJYPTzqsrJxxApX18366098;     UNGmQhJYPTzqsrJxxApX18366098 = UNGmQhJYPTzqsrJxxApX79853;     UNGmQhJYPTzqsrJxxApX79853 = UNGmQhJYPTzqsrJxxApX55114936;     UNGmQhJYPTzqsrJxxApX55114936 = UNGmQhJYPTzqsrJxxApX75082232;     UNGmQhJYPTzqsrJxxApX75082232 = UNGmQhJYPTzqsrJxxApX26066147;     UNGmQhJYPTzqsrJxxApX26066147 = UNGmQhJYPTzqsrJxxApX84042908;     UNGmQhJYPTzqsrJxxApX84042908 = UNGmQhJYPTzqsrJxxApX95866531;     UNGmQhJYPTzqsrJxxApX95866531 = UNGmQhJYPTzqsrJxxApX41763878;     UNGmQhJYPTzqsrJxxApX41763878 = UNGmQhJYPTzqsrJxxApX86444454;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void JmgusdaGXDwUDJJwFerG89144855() {     double NvANzUVAhMRWErJBNkMm12029732 = -486605162;    double NvANzUVAhMRWErJBNkMm18891156 = -798915959;    double NvANzUVAhMRWErJBNkMm33900121 = -784443115;    double NvANzUVAhMRWErJBNkMm86122921 = -605568236;    double NvANzUVAhMRWErJBNkMm74914392 = -818512250;    double NvANzUVAhMRWErJBNkMm46167420 = -556977886;    double NvANzUVAhMRWErJBNkMm61052697 = 20843210;    double NvANzUVAhMRWErJBNkMm30033551 = -464094641;    double NvANzUVAhMRWErJBNkMm95520129 = -257738779;    double NvANzUVAhMRWErJBNkMm15247068 = 12398021;    double NvANzUVAhMRWErJBNkMm44824774 = -579122169;    double NvANzUVAhMRWErJBNkMm28772480 = -131769382;    double NvANzUVAhMRWErJBNkMm63328590 = -862387688;    double NvANzUVAhMRWErJBNkMm16717921 = -539833861;    double NvANzUVAhMRWErJBNkMm75432007 = -807291685;    double NvANzUVAhMRWErJBNkMm27983041 = -883063149;    double NvANzUVAhMRWErJBNkMm23128232 = -291645169;    double NvANzUVAhMRWErJBNkMm54527593 = -832022455;    double NvANzUVAhMRWErJBNkMm31146284 = -332355059;    double NvANzUVAhMRWErJBNkMm50380027 = -571458700;    double NvANzUVAhMRWErJBNkMm22842579 = -889902019;    double NvANzUVAhMRWErJBNkMm54046620 = -832307125;    double NvANzUVAhMRWErJBNkMm29035436 = 43719348;    double NvANzUVAhMRWErJBNkMm85105432 = 43838716;    double NvANzUVAhMRWErJBNkMm35877461 = -173421018;    double NvANzUVAhMRWErJBNkMm85815383 = -605235229;    double NvANzUVAhMRWErJBNkMm65973748 = -266929382;    double NvANzUVAhMRWErJBNkMm6437076 = -305597972;    double NvANzUVAhMRWErJBNkMm44399890 = 16379403;    double NvANzUVAhMRWErJBNkMm22431325 = -624443849;    double NvANzUVAhMRWErJBNkMm66685358 = -875444053;    double NvANzUVAhMRWErJBNkMm52604953 = -356663197;    double NvANzUVAhMRWErJBNkMm47979452 = -544846385;    double NvANzUVAhMRWErJBNkMm67373645 = -123483188;    double NvANzUVAhMRWErJBNkMm39903302 = -320860919;    double NvANzUVAhMRWErJBNkMm95072091 = -413703262;    double NvANzUVAhMRWErJBNkMm70445012 = -450363351;    double NvANzUVAhMRWErJBNkMm53365454 = -402014762;    double NvANzUVAhMRWErJBNkMm90249572 = -407495031;    double NvANzUVAhMRWErJBNkMm9470914 = -206690206;    double NvANzUVAhMRWErJBNkMm35427090 = -815205707;    double NvANzUVAhMRWErJBNkMm48576486 = -631585307;    double NvANzUVAhMRWErJBNkMm34325131 = 78769217;    double NvANzUVAhMRWErJBNkMm18109772 = -900612971;    double NvANzUVAhMRWErJBNkMm48820953 = -519755738;    double NvANzUVAhMRWErJBNkMm27465287 = -710850533;    double NvANzUVAhMRWErJBNkMm14235313 = -899458761;    double NvANzUVAhMRWErJBNkMm96410699 = -325846206;    double NvANzUVAhMRWErJBNkMm16217349 = -80052118;    double NvANzUVAhMRWErJBNkMm74036916 = -794887187;    double NvANzUVAhMRWErJBNkMm10037516 = -819428677;    double NvANzUVAhMRWErJBNkMm57081949 = -475606470;    double NvANzUVAhMRWErJBNkMm70394039 = -994045357;    double NvANzUVAhMRWErJBNkMm82431230 = -663512613;    double NvANzUVAhMRWErJBNkMm66302277 = -487485529;    double NvANzUVAhMRWErJBNkMm57983111 = -654298037;    double NvANzUVAhMRWErJBNkMm89855720 = -742635307;    double NvANzUVAhMRWErJBNkMm48794689 = -728281832;    double NvANzUVAhMRWErJBNkMm50245460 = -332147219;    double NvANzUVAhMRWErJBNkMm89099008 = -113277022;    double NvANzUVAhMRWErJBNkMm80193671 = -190048504;    double NvANzUVAhMRWErJBNkMm54615621 = -673558818;    double NvANzUVAhMRWErJBNkMm85633660 = -380474045;    double NvANzUVAhMRWErJBNkMm73088804 = -633294930;    double NvANzUVAhMRWErJBNkMm48561710 = -112157927;    double NvANzUVAhMRWErJBNkMm92219821 = -122458972;    double NvANzUVAhMRWErJBNkMm80793028 = -586922998;    double NvANzUVAhMRWErJBNkMm95954945 = -638904501;    double NvANzUVAhMRWErJBNkMm76814619 = -118972942;    double NvANzUVAhMRWErJBNkMm80359915 = -293588423;    double NvANzUVAhMRWErJBNkMm57538029 = -332699798;    double NvANzUVAhMRWErJBNkMm69762777 = -889630407;    double NvANzUVAhMRWErJBNkMm64278020 = -324527424;    double NvANzUVAhMRWErJBNkMm21675370 = -25664853;    double NvANzUVAhMRWErJBNkMm14952937 = -756252994;    double NvANzUVAhMRWErJBNkMm74266093 = -158316713;    double NvANzUVAhMRWErJBNkMm19721490 = -811076343;    double NvANzUVAhMRWErJBNkMm10925664 = -55667682;    double NvANzUVAhMRWErJBNkMm36284479 = -436405547;    double NvANzUVAhMRWErJBNkMm8412175 = -462570485;    double NvANzUVAhMRWErJBNkMm71580071 = -705776469;    double NvANzUVAhMRWErJBNkMm69563048 = -941083177;    double NvANzUVAhMRWErJBNkMm90219727 = -125545855;    double NvANzUVAhMRWErJBNkMm70362974 = -188733410;    double NvANzUVAhMRWErJBNkMm12393810 = -805015172;    double NvANzUVAhMRWErJBNkMm9603410 = -299837584;    double NvANzUVAhMRWErJBNkMm82210914 = -362617841;    double NvANzUVAhMRWErJBNkMm65548222 = -881333772;    double NvANzUVAhMRWErJBNkMm1071369 = -635997659;    double NvANzUVAhMRWErJBNkMm81920190 = -666562882;    double NvANzUVAhMRWErJBNkMm5216372 = -671067956;    double NvANzUVAhMRWErJBNkMm21650323 = -722081520;    double NvANzUVAhMRWErJBNkMm3119994 = 30132456;    double NvANzUVAhMRWErJBNkMm1150564 = -194218010;    double NvANzUVAhMRWErJBNkMm29277242 = 83358297;    double NvANzUVAhMRWErJBNkMm80811468 = -41646889;    double NvANzUVAhMRWErJBNkMm62942825 = -151111262;    double NvANzUVAhMRWErJBNkMm61236326 = -287935853;    double NvANzUVAhMRWErJBNkMm69548062 = -688455045;    double NvANzUVAhMRWErJBNkMm56601132 = -486605162;     NvANzUVAhMRWErJBNkMm12029732 = NvANzUVAhMRWErJBNkMm18891156;     NvANzUVAhMRWErJBNkMm18891156 = NvANzUVAhMRWErJBNkMm33900121;     NvANzUVAhMRWErJBNkMm33900121 = NvANzUVAhMRWErJBNkMm86122921;     NvANzUVAhMRWErJBNkMm86122921 = NvANzUVAhMRWErJBNkMm74914392;     NvANzUVAhMRWErJBNkMm74914392 = NvANzUVAhMRWErJBNkMm46167420;     NvANzUVAhMRWErJBNkMm46167420 = NvANzUVAhMRWErJBNkMm61052697;     NvANzUVAhMRWErJBNkMm61052697 = NvANzUVAhMRWErJBNkMm30033551;     NvANzUVAhMRWErJBNkMm30033551 = NvANzUVAhMRWErJBNkMm95520129;     NvANzUVAhMRWErJBNkMm95520129 = NvANzUVAhMRWErJBNkMm15247068;     NvANzUVAhMRWErJBNkMm15247068 = NvANzUVAhMRWErJBNkMm44824774;     NvANzUVAhMRWErJBNkMm44824774 = NvANzUVAhMRWErJBNkMm28772480;     NvANzUVAhMRWErJBNkMm28772480 = NvANzUVAhMRWErJBNkMm63328590;     NvANzUVAhMRWErJBNkMm63328590 = NvANzUVAhMRWErJBNkMm16717921;     NvANzUVAhMRWErJBNkMm16717921 = NvANzUVAhMRWErJBNkMm75432007;     NvANzUVAhMRWErJBNkMm75432007 = NvANzUVAhMRWErJBNkMm27983041;     NvANzUVAhMRWErJBNkMm27983041 = NvANzUVAhMRWErJBNkMm23128232;     NvANzUVAhMRWErJBNkMm23128232 = NvANzUVAhMRWErJBNkMm54527593;     NvANzUVAhMRWErJBNkMm54527593 = NvANzUVAhMRWErJBNkMm31146284;     NvANzUVAhMRWErJBNkMm31146284 = NvANzUVAhMRWErJBNkMm50380027;     NvANzUVAhMRWErJBNkMm50380027 = NvANzUVAhMRWErJBNkMm22842579;     NvANzUVAhMRWErJBNkMm22842579 = NvANzUVAhMRWErJBNkMm54046620;     NvANzUVAhMRWErJBNkMm54046620 = NvANzUVAhMRWErJBNkMm29035436;     NvANzUVAhMRWErJBNkMm29035436 = NvANzUVAhMRWErJBNkMm85105432;     NvANzUVAhMRWErJBNkMm85105432 = NvANzUVAhMRWErJBNkMm35877461;     NvANzUVAhMRWErJBNkMm35877461 = NvANzUVAhMRWErJBNkMm85815383;     NvANzUVAhMRWErJBNkMm85815383 = NvANzUVAhMRWErJBNkMm65973748;     NvANzUVAhMRWErJBNkMm65973748 = NvANzUVAhMRWErJBNkMm6437076;     NvANzUVAhMRWErJBNkMm6437076 = NvANzUVAhMRWErJBNkMm44399890;     NvANzUVAhMRWErJBNkMm44399890 = NvANzUVAhMRWErJBNkMm22431325;     NvANzUVAhMRWErJBNkMm22431325 = NvANzUVAhMRWErJBNkMm66685358;     NvANzUVAhMRWErJBNkMm66685358 = NvANzUVAhMRWErJBNkMm52604953;     NvANzUVAhMRWErJBNkMm52604953 = NvANzUVAhMRWErJBNkMm47979452;     NvANzUVAhMRWErJBNkMm47979452 = NvANzUVAhMRWErJBNkMm67373645;     NvANzUVAhMRWErJBNkMm67373645 = NvANzUVAhMRWErJBNkMm39903302;     NvANzUVAhMRWErJBNkMm39903302 = NvANzUVAhMRWErJBNkMm95072091;     NvANzUVAhMRWErJBNkMm95072091 = NvANzUVAhMRWErJBNkMm70445012;     NvANzUVAhMRWErJBNkMm70445012 = NvANzUVAhMRWErJBNkMm53365454;     NvANzUVAhMRWErJBNkMm53365454 = NvANzUVAhMRWErJBNkMm90249572;     NvANzUVAhMRWErJBNkMm90249572 = NvANzUVAhMRWErJBNkMm9470914;     NvANzUVAhMRWErJBNkMm9470914 = NvANzUVAhMRWErJBNkMm35427090;     NvANzUVAhMRWErJBNkMm35427090 = NvANzUVAhMRWErJBNkMm48576486;     NvANzUVAhMRWErJBNkMm48576486 = NvANzUVAhMRWErJBNkMm34325131;     NvANzUVAhMRWErJBNkMm34325131 = NvANzUVAhMRWErJBNkMm18109772;     NvANzUVAhMRWErJBNkMm18109772 = NvANzUVAhMRWErJBNkMm48820953;     NvANzUVAhMRWErJBNkMm48820953 = NvANzUVAhMRWErJBNkMm27465287;     NvANzUVAhMRWErJBNkMm27465287 = NvANzUVAhMRWErJBNkMm14235313;     NvANzUVAhMRWErJBNkMm14235313 = NvANzUVAhMRWErJBNkMm96410699;     NvANzUVAhMRWErJBNkMm96410699 = NvANzUVAhMRWErJBNkMm16217349;     NvANzUVAhMRWErJBNkMm16217349 = NvANzUVAhMRWErJBNkMm74036916;     NvANzUVAhMRWErJBNkMm74036916 = NvANzUVAhMRWErJBNkMm10037516;     NvANzUVAhMRWErJBNkMm10037516 = NvANzUVAhMRWErJBNkMm57081949;     NvANzUVAhMRWErJBNkMm57081949 = NvANzUVAhMRWErJBNkMm70394039;     NvANzUVAhMRWErJBNkMm70394039 = NvANzUVAhMRWErJBNkMm82431230;     NvANzUVAhMRWErJBNkMm82431230 = NvANzUVAhMRWErJBNkMm66302277;     NvANzUVAhMRWErJBNkMm66302277 = NvANzUVAhMRWErJBNkMm57983111;     NvANzUVAhMRWErJBNkMm57983111 = NvANzUVAhMRWErJBNkMm89855720;     NvANzUVAhMRWErJBNkMm89855720 = NvANzUVAhMRWErJBNkMm48794689;     NvANzUVAhMRWErJBNkMm48794689 = NvANzUVAhMRWErJBNkMm50245460;     NvANzUVAhMRWErJBNkMm50245460 = NvANzUVAhMRWErJBNkMm89099008;     NvANzUVAhMRWErJBNkMm89099008 = NvANzUVAhMRWErJBNkMm80193671;     NvANzUVAhMRWErJBNkMm80193671 = NvANzUVAhMRWErJBNkMm54615621;     NvANzUVAhMRWErJBNkMm54615621 = NvANzUVAhMRWErJBNkMm85633660;     NvANzUVAhMRWErJBNkMm85633660 = NvANzUVAhMRWErJBNkMm73088804;     NvANzUVAhMRWErJBNkMm73088804 = NvANzUVAhMRWErJBNkMm48561710;     NvANzUVAhMRWErJBNkMm48561710 = NvANzUVAhMRWErJBNkMm92219821;     NvANzUVAhMRWErJBNkMm92219821 = NvANzUVAhMRWErJBNkMm80793028;     NvANzUVAhMRWErJBNkMm80793028 = NvANzUVAhMRWErJBNkMm95954945;     NvANzUVAhMRWErJBNkMm95954945 = NvANzUVAhMRWErJBNkMm76814619;     NvANzUVAhMRWErJBNkMm76814619 = NvANzUVAhMRWErJBNkMm80359915;     NvANzUVAhMRWErJBNkMm80359915 = NvANzUVAhMRWErJBNkMm57538029;     NvANzUVAhMRWErJBNkMm57538029 = NvANzUVAhMRWErJBNkMm69762777;     NvANzUVAhMRWErJBNkMm69762777 = NvANzUVAhMRWErJBNkMm64278020;     NvANzUVAhMRWErJBNkMm64278020 = NvANzUVAhMRWErJBNkMm21675370;     NvANzUVAhMRWErJBNkMm21675370 = NvANzUVAhMRWErJBNkMm14952937;     NvANzUVAhMRWErJBNkMm14952937 = NvANzUVAhMRWErJBNkMm74266093;     NvANzUVAhMRWErJBNkMm74266093 = NvANzUVAhMRWErJBNkMm19721490;     NvANzUVAhMRWErJBNkMm19721490 = NvANzUVAhMRWErJBNkMm10925664;     NvANzUVAhMRWErJBNkMm10925664 = NvANzUVAhMRWErJBNkMm36284479;     NvANzUVAhMRWErJBNkMm36284479 = NvANzUVAhMRWErJBNkMm8412175;     NvANzUVAhMRWErJBNkMm8412175 = NvANzUVAhMRWErJBNkMm71580071;     NvANzUVAhMRWErJBNkMm71580071 = NvANzUVAhMRWErJBNkMm69563048;     NvANzUVAhMRWErJBNkMm69563048 = NvANzUVAhMRWErJBNkMm90219727;     NvANzUVAhMRWErJBNkMm90219727 = NvANzUVAhMRWErJBNkMm70362974;     NvANzUVAhMRWErJBNkMm70362974 = NvANzUVAhMRWErJBNkMm12393810;     NvANzUVAhMRWErJBNkMm12393810 = NvANzUVAhMRWErJBNkMm9603410;     NvANzUVAhMRWErJBNkMm9603410 = NvANzUVAhMRWErJBNkMm82210914;     NvANzUVAhMRWErJBNkMm82210914 = NvANzUVAhMRWErJBNkMm65548222;     NvANzUVAhMRWErJBNkMm65548222 = NvANzUVAhMRWErJBNkMm1071369;     NvANzUVAhMRWErJBNkMm1071369 = NvANzUVAhMRWErJBNkMm81920190;     NvANzUVAhMRWErJBNkMm81920190 = NvANzUVAhMRWErJBNkMm5216372;     NvANzUVAhMRWErJBNkMm5216372 = NvANzUVAhMRWErJBNkMm21650323;     NvANzUVAhMRWErJBNkMm21650323 = NvANzUVAhMRWErJBNkMm3119994;     NvANzUVAhMRWErJBNkMm3119994 = NvANzUVAhMRWErJBNkMm1150564;     NvANzUVAhMRWErJBNkMm1150564 = NvANzUVAhMRWErJBNkMm29277242;     NvANzUVAhMRWErJBNkMm29277242 = NvANzUVAhMRWErJBNkMm80811468;     NvANzUVAhMRWErJBNkMm80811468 = NvANzUVAhMRWErJBNkMm62942825;     NvANzUVAhMRWErJBNkMm62942825 = NvANzUVAhMRWErJBNkMm61236326;     NvANzUVAhMRWErJBNkMm61236326 = NvANzUVAhMRWErJBNkMm69548062;     NvANzUVAhMRWErJBNkMm69548062 = NvANzUVAhMRWErJBNkMm56601132;     NvANzUVAhMRWErJBNkMm56601132 = NvANzUVAhMRWErJBNkMm12029732;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void EXikbaERZFKmoaqHDbap16739113() {     double fYlKumOdqJpHnBczbGVb37615009 = 88361737;    double fYlKumOdqJpHnBczbGVb15288300 = 45606264;    double fYlKumOdqJpHnBczbGVb66779297 = -586904651;    double fYlKumOdqJpHnBczbGVb31844851 = -612891130;    double fYlKumOdqJpHnBczbGVb44256397 = -865434537;    double fYlKumOdqJpHnBczbGVb63176696 = -711958889;    double fYlKumOdqJpHnBczbGVb66373541 = -561427997;    double fYlKumOdqJpHnBczbGVb45043675 = -844776520;    double fYlKumOdqJpHnBczbGVb20849423 = -753962823;    double fYlKumOdqJpHnBczbGVb34405450 = -322163502;    double fYlKumOdqJpHnBczbGVb59158328 = -889550055;    double fYlKumOdqJpHnBczbGVb64090136 = -575478289;    double fYlKumOdqJpHnBczbGVb39909368 = -626636054;    double fYlKumOdqJpHnBczbGVb8277638 = -514358155;    double fYlKumOdqJpHnBczbGVb31700895 = -695837643;    double fYlKumOdqJpHnBczbGVb35704084 = -219254626;    double fYlKumOdqJpHnBczbGVb22926348 = 20035037;    double fYlKumOdqJpHnBczbGVb75643497 = -248299816;    double fYlKumOdqJpHnBczbGVb92611004 = -15328916;    double fYlKumOdqJpHnBczbGVb11541494 = -545898076;    double fYlKumOdqJpHnBczbGVb23851844 = -251601800;    double fYlKumOdqJpHnBczbGVb55549727 = -152040055;    double fYlKumOdqJpHnBczbGVb32799234 = -372758;    double fYlKumOdqJpHnBczbGVb86819474 = -915010264;    double fYlKumOdqJpHnBczbGVb90780600 = -840917160;    double fYlKumOdqJpHnBczbGVb15304521 = -169685770;    double fYlKumOdqJpHnBczbGVb48361657 = -932742517;    double fYlKumOdqJpHnBczbGVb70335876 = -535260205;    double fYlKumOdqJpHnBczbGVb52277103 = -219914447;    double fYlKumOdqJpHnBczbGVb75753816 = -533353411;    double fYlKumOdqJpHnBczbGVb88731992 = -654091084;    double fYlKumOdqJpHnBczbGVb89794885 = -630459031;    double fYlKumOdqJpHnBczbGVb69166503 = -992667522;    double fYlKumOdqJpHnBczbGVb20960717 = -179628159;    double fYlKumOdqJpHnBczbGVb41308290 = 77703169;    double fYlKumOdqJpHnBczbGVb99863942 = -931297978;    double fYlKumOdqJpHnBczbGVb20090397 = -930875296;    double fYlKumOdqJpHnBczbGVb28938141 = -250729587;    double fYlKumOdqJpHnBczbGVb31173151 = -865833353;    double fYlKumOdqJpHnBczbGVb18254588 = -676660525;    double fYlKumOdqJpHnBczbGVb82578371 = -290265896;    double fYlKumOdqJpHnBczbGVb92586075 = -40623811;    double fYlKumOdqJpHnBczbGVb83525350 = 50162836;    double fYlKumOdqJpHnBczbGVb88903051 = -269028570;    double fYlKumOdqJpHnBczbGVb40801830 = -458776855;    double fYlKumOdqJpHnBczbGVb93865608 = -133398071;    double fYlKumOdqJpHnBczbGVb14003767 = -931927621;    double fYlKumOdqJpHnBczbGVb4523471 = -519029053;    double fYlKumOdqJpHnBczbGVb67028668 = -179728344;    double fYlKumOdqJpHnBczbGVb27846911 = -642910340;    double fYlKumOdqJpHnBczbGVb20173763 = 31912720;    double fYlKumOdqJpHnBczbGVb84340528 = -653685649;    double fYlKumOdqJpHnBczbGVb88467642 = -461943906;    double fYlKumOdqJpHnBczbGVb51330139 = -978576395;    double fYlKumOdqJpHnBczbGVb92887268 = -962575946;    double fYlKumOdqJpHnBczbGVb82065281 = -759598209;    double fYlKumOdqJpHnBczbGVb82489065 = -954020979;    double fYlKumOdqJpHnBczbGVb79959822 = -671894388;    double fYlKumOdqJpHnBczbGVb41064250 = -771973970;    double fYlKumOdqJpHnBczbGVb28951877 = -595748768;    double fYlKumOdqJpHnBczbGVb14815039 = -779216373;    double fYlKumOdqJpHnBczbGVb96037665 = 73832207;    double fYlKumOdqJpHnBczbGVb92766572 = -524862074;    double fYlKumOdqJpHnBczbGVb45095606 = -120609413;    double fYlKumOdqJpHnBczbGVb45673458 = -668072419;    double fYlKumOdqJpHnBczbGVb69363443 = -159091025;    double fYlKumOdqJpHnBczbGVb94923632 = -582810767;    double fYlKumOdqJpHnBczbGVb18948652 = -347007895;    double fYlKumOdqJpHnBczbGVb66969348 = -492061325;    double fYlKumOdqJpHnBczbGVb31836953 = -764539665;    double fYlKumOdqJpHnBczbGVb15613687 = -288379330;    double fYlKumOdqJpHnBczbGVb93988207 = -729235376;    double fYlKumOdqJpHnBczbGVb44470346 = -382466463;    double fYlKumOdqJpHnBczbGVb74356417 = -338668391;    double fYlKumOdqJpHnBczbGVb28963123 = -155632180;    double fYlKumOdqJpHnBczbGVb31265769 = -110977990;    double fYlKumOdqJpHnBczbGVb72024377 = -102202891;    double fYlKumOdqJpHnBczbGVb43896183 = -731344188;    double fYlKumOdqJpHnBczbGVb46017645 = -356233409;    double fYlKumOdqJpHnBczbGVb96914991 = -607519090;    double fYlKumOdqJpHnBczbGVb1300754 = -237758150;    double fYlKumOdqJpHnBczbGVb43838187 = -313713465;    double fYlKumOdqJpHnBczbGVb3307208 = -255531861;    double fYlKumOdqJpHnBczbGVb24430192 = -577004107;    double fYlKumOdqJpHnBczbGVb55580053 = -465266132;    double fYlKumOdqJpHnBczbGVb4391465 = 99594565;    double fYlKumOdqJpHnBczbGVb1327243 = -68515126;    double fYlKumOdqJpHnBczbGVb17836365 = 85908872;    double fYlKumOdqJpHnBczbGVb28073448 = -217052213;    double fYlKumOdqJpHnBczbGVb59243008 = -162698623;    double fYlKumOdqJpHnBczbGVb17374877 = -977277000;    double fYlKumOdqJpHnBczbGVb40130575 = -158980908;    double fYlKumOdqJpHnBczbGVb87873890 = -478755618;    double fYlKumOdqJpHnBczbGVb2221274 = -170084586;    double fYlKumOdqJpHnBczbGVb3439549 = -897444153;    double fYlKumOdqJpHnBczbGVb86540705 = -264098104;    double fYlKumOdqJpHnBczbGVb99819503 = -515761738;    double fYlKumOdqJpHnBczbGVb38429744 = -829227752;    double fYlKumOdqJpHnBczbGVb43229594 = -600956152;    double fYlKumOdqJpHnBczbGVb71438387 = 88361737;     fYlKumOdqJpHnBczbGVb37615009 = fYlKumOdqJpHnBczbGVb15288300;     fYlKumOdqJpHnBczbGVb15288300 = fYlKumOdqJpHnBczbGVb66779297;     fYlKumOdqJpHnBczbGVb66779297 = fYlKumOdqJpHnBczbGVb31844851;     fYlKumOdqJpHnBczbGVb31844851 = fYlKumOdqJpHnBczbGVb44256397;     fYlKumOdqJpHnBczbGVb44256397 = fYlKumOdqJpHnBczbGVb63176696;     fYlKumOdqJpHnBczbGVb63176696 = fYlKumOdqJpHnBczbGVb66373541;     fYlKumOdqJpHnBczbGVb66373541 = fYlKumOdqJpHnBczbGVb45043675;     fYlKumOdqJpHnBczbGVb45043675 = fYlKumOdqJpHnBczbGVb20849423;     fYlKumOdqJpHnBczbGVb20849423 = fYlKumOdqJpHnBczbGVb34405450;     fYlKumOdqJpHnBczbGVb34405450 = fYlKumOdqJpHnBczbGVb59158328;     fYlKumOdqJpHnBczbGVb59158328 = fYlKumOdqJpHnBczbGVb64090136;     fYlKumOdqJpHnBczbGVb64090136 = fYlKumOdqJpHnBczbGVb39909368;     fYlKumOdqJpHnBczbGVb39909368 = fYlKumOdqJpHnBczbGVb8277638;     fYlKumOdqJpHnBczbGVb8277638 = fYlKumOdqJpHnBczbGVb31700895;     fYlKumOdqJpHnBczbGVb31700895 = fYlKumOdqJpHnBczbGVb35704084;     fYlKumOdqJpHnBczbGVb35704084 = fYlKumOdqJpHnBczbGVb22926348;     fYlKumOdqJpHnBczbGVb22926348 = fYlKumOdqJpHnBczbGVb75643497;     fYlKumOdqJpHnBczbGVb75643497 = fYlKumOdqJpHnBczbGVb92611004;     fYlKumOdqJpHnBczbGVb92611004 = fYlKumOdqJpHnBczbGVb11541494;     fYlKumOdqJpHnBczbGVb11541494 = fYlKumOdqJpHnBczbGVb23851844;     fYlKumOdqJpHnBczbGVb23851844 = fYlKumOdqJpHnBczbGVb55549727;     fYlKumOdqJpHnBczbGVb55549727 = fYlKumOdqJpHnBczbGVb32799234;     fYlKumOdqJpHnBczbGVb32799234 = fYlKumOdqJpHnBczbGVb86819474;     fYlKumOdqJpHnBczbGVb86819474 = fYlKumOdqJpHnBczbGVb90780600;     fYlKumOdqJpHnBczbGVb90780600 = fYlKumOdqJpHnBczbGVb15304521;     fYlKumOdqJpHnBczbGVb15304521 = fYlKumOdqJpHnBczbGVb48361657;     fYlKumOdqJpHnBczbGVb48361657 = fYlKumOdqJpHnBczbGVb70335876;     fYlKumOdqJpHnBczbGVb70335876 = fYlKumOdqJpHnBczbGVb52277103;     fYlKumOdqJpHnBczbGVb52277103 = fYlKumOdqJpHnBczbGVb75753816;     fYlKumOdqJpHnBczbGVb75753816 = fYlKumOdqJpHnBczbGVb88731992;     fYlKumOdqJpHnBczbGVb88731992 = fYlKumOdqJpHnBczbGVb89794885;     fYlKumOdqJpHnBczbGVb89794885 = fYlKumOdqJpHnBczbGVb69166503;     fYlKumOdqJpHnBczbGVb69166503 = fYlKumOdqJpHnBczbGVb20960717;     fYlKumOdqJpHnBczbGVb20960717 = fYlKumOdqJpHnBczbGVb41308290;     fYlKumOdqJpHnBczbGVb41308290 = fYlKumOdqJpHnBczbGVb99863942;     fYlKumOdqJpHnBczbGVb99863942 = fYlKumOdqJpHnBczbGVb20090397;     fYlKumOdqJpHnBczbGVb20090397 = fYlKumOdqJpHnBczbGVb28938141;     fYlKumOdqJpHnBczbGVb28938141 = fYlKumOdqJpHnBczbGVb31173151;     fYlKumOdqJpHnBczbGVb31173151 = fYlKumOdqJpHnBczbGVb18254588;     fYlKumOdqJpHnBczbGVb18254588 = fYlKumOdqJpHnBczbGVb82578371;     fYlKumOdqJpHnBczbGVb82578371 = fYlKumOdqJpHnBczbGVb92586075;     fYlKumOdqJpHnBczbGVb92586075 = fYlKumOdqJpHnBczbGVb83525350;     fYlKumOdqJpHnBczbGVb83525350 = fYlKumOdqJpHnBczbGVb88903051;     fYlKumOdqJpHnBczbGVb88903051 = fYlKumOdqJpHnBczbGVb40801830;     fYlKumOdqJpHnBczbGVb40801830 = fYlKumOdqJpHnBczbGVb93865608;     fYlKumOdqJpHnBczbGVb93865608 = fYlKumOdqJpHnBczbGVb14003767;     fYlKumOdqJpHnBczbGVb14003767 = fYlKumOdqJpHnBczbGVb4523471;     fYlKumOdqJpHnBczbGVb4523471 = fYlKumOdqJpHnBczbGVb67028668;     fYlKumOdqJpHnBczbGVb67028668 = fYlKumOdqJpHnBczbGVb27846911;     fYlKumOdqJpHnBczbGVb27846911 = fYlKumOdqJpHnBczbGVb20173763;     fYlKumOdqJpHnBczbGVb20173763 = fYlKumOdqJpHnBczbGVb84340528;     fYlKumOdqJpHnBczbGVb84340528 = fYlKumOdqJpHnBczbGVb88467642;     fYlKumOdqJpHnBczbGVb88467642 = fYlKumOdqJpHnBczbGVb51330139;     fYlKumOdqJpHnBczbGVb51330139 = fYlKumOdqJpHnBczbGVb92887268;     fYlKumOdqJpHnBczbGVb92887268 = fYlKumOdqJpHnBczbGVb82065281;     fYlKumOdqJpHnBczbGVb82065281 = fYlKumOdqJpHnBczbGVb82489065;     fYlKumOdqJpHnBczbGVb82489065 = fYlKumOdqJpHnBczbGVb79959822;     fYlKumOdqJpHnBczbGVb79959822 = fYlKumOdqJpHnBczbGVb41064250;     fYlKumOdqJpHnBczbGVb41064250 = fYlKumOdqJpHnBczbGVb28951877;     fYlKumOdqJpHnBczbGVb28951877 = fYlKumOdqJpHnBczbGVb14815039;     fYlKumOdqJpHnBczbGVb14815039 = fYlKumOdqJpHnBczbGVb96037665;     fYlKumOdqJpHnBczbGVb96037665 = fYlKumOdqJpHnBczbGVb92766572;     fYlKumOdqJpHnBczbGVb92766572 = fYlKumOdqJpHnBczbGVb45095606;     fYlKumOdqJpHnBczbGVb45095606 = fYlKumOdqJpHnBczbGVb45673458;     fYlKumOdqJpHnBczbGVb45673458 = fYlKumOdqJpHnBczbGVb69363443;     fYlKumOdqJpHnBczbGVb69363443 = fYlKumOdqJpHnBczbGVb94923632;     fYlKumOdqJpHnBczbGVb94923632 = fYlKumOdqJpHnBczbGVb18948652;     fYlKumOdqJpHnBczbGVb18948652 = fYlKumOdqJpHnBczbGVb66969348;     fYlKumOdqJpHnBczbGVb66969348 = fYlKumOdqJpHnBczbGVb31836953;     fYlKumOdqJpHnBczbGVb31836953 = fYlKumOdqJpHnBczbGVb15613687;     fYlKumOdqJpHnBczbGVb15613687 = fYlKumOdqJpHnBczbGVb93988207;     fYlKumOdqJpHnBczbGVb93988207 = fYlKumOdqJpHnBczbGVb44470346;     fYlKumOdqJpHnBczbGVb44470346 = fYlKumOdqJpHnBczbGVb74356417;     fYlKumOdqJpHnBczbGVb74356417 = fYlKumOdqJpHnBczbGVb28963123;     fYlKumOdqJpHnBczbGVb28963123 = fYlKumOdqJpHnBczbGVb31265769;     fYlKumOdqJpHnBczbGVb31265769 = fYlKumOdqJpHnBczbGVb72024377;     fYlKumOdqJpHnBczbGVb72024377 = fYlKumOdqJpHnBczbGVb43896183;     fYlKumOdqJpHnBczbGVb43896183 = fYlKumOdqJpHnBczbGVb46017645;     fYlKumOdqJpHnBczbGVb46017645 = fYlKumOdqJpHnBczbGVb96914991;     fYlKumOdqJpHnBczbGVb96914991 = fYlKumOdqJpHnBczbGVb1300754;     fYlKumOdqJpHnBczbGVb1300754 = fYlKumOdqJpHnBczbGVb43838187;     fYlKumOdqJpHnBczbGVb43838187 = fYlKumOdqJpHnBczbGVb3307208;     fYlKumOdqJpHnBczbGVb3307208 = fYlKumOdqJpHnBczbGVb24430192;     fYlKumOdqJpHnBczbGVb24430192 = fYlKumOdqJpHnBczbGVb55580053;     fYlKumOdqJpHnBczbGVb55580053 = fYlKumOdqJpHnBczbGVb4391465;     fYlKumOdqJpHnBczbGVb4391465 = fYlKumOdqJpHnBczbGVb1327243;     fYlKumOdqJpHnBczbGVb1327243 = fYlKumOdqJpHnBczbGVb17836365;     fYlKumOdqJpHnBczbGVb17836365 = fYlKumOdqJpHnBczbGVb28073448;     fYlKumOdqJpHnBczbGVb28073448 = fYlKumOdqJpHnBczbGVb59243008;     fYlKumOdqJpHnBczbGVb59243008 = fYlKumOdqJpHnBczbGVb17374877;     fYlKumOdqJpHnBczbGVb17374877 = fYlKumOdqJpHnBczbGVb40130575;     fYlKumOdqJpHnBczbGVb40130575 = fYlKumOdqJpHnBczbGVb87873890;     fYlKumOdqJpHnBczbGVb87873890 = fYlKumOdqJpHnBczbGVb2221274;     fYlKumOdqJpHnBczbGVb2221274 = fYlKumOdqJpHnBczbGVb3439549;     fYlKumOdqJpHnBczbGVb3439549 = fYlKumOdqJpHnBczbGVb86540705;     fYlKumOdqJpHnBczbGVb86540705 = fYlKumOdqJpHnBczbGVb99819503;     fYlKumOdqJpHnBczbGVb99819503 = fYlKumOdqJpHnBczbGVb38429744;     fYlKumOdqJpHnBczbGVb38429744 = fYlKumOdqJpHnBczbGVb43229594;     fYlKumOdqJpHnBczbGVb43229594 = fYlKumOdqJpHnBczbGVb71438387;     fYlKumOdqJpHnBczbGVb71438387 = fYlKumOdqJpHnBczbGVb37615009;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void wIyokyRPrzDAzWKxcMQT44333369() {     double DrZAyInHtLrBvGudoHnw63200285 = -436671363;    double DrZAyInHtLrBvGudoHnw11685443 = -209871512;    double DrZAyInHtLrBvGudoHnw99658472 = -389366187;    double DrZAyInHtLrBvGudoHnw77566780 = -620214023;    double DrZAyInHtLrBvGudoHnw13598402 = -912356825;    double DrZAyInHtLrBvGudoHnw80185973 = -866939892;    double DrZAyInHtLrBvGudoHnw71694386 = -43699204;    double DrZAyInHtLrBvGudoHnw60053799 = -125458399;    double DrZAyInHtLrBvGudoHnw46178715 = -150186868;    double DrZAyInHtLrBvGudoHnw53563833 = -656725026;    double DrZAyInHtLrBvGudoHnw73491882 = -99977942;    double DrZAyInHtLrBvGudoHnw99407792 = 80812805;    double DrZAyInHtLrBvGudoHnw16490146 = -390884419;    double DrZAyInHtLrBvGudoHnw99837354 = -488882448;    double DrZAyInHtLrBvGudoHnw87969783 = -584383601;    double DrZAyInHtLrBvGudoHnw43425127 = -655446103;    double DrZAyInHtLrBvGudoHnw22724464 = -768284757;    double DrZAyInHtLrBvGudoHnw96759401 = -764577176;    double DrZAyInHtLrBvGudoHnw54075726 = -798302773;    double DrZAyInHtLrBvGudoHnw72702960 = -520337451;    double DrZAyInHtLrBvGudoHnw24861110 = -713301581;    double DrZAyInHtLrBvGudoHnw57052834 = -571772984;    double DrZAyInHtLrBvGudoHnw36563032 = -44464863;    double DrZAyInHtLrBvGudoHnw88533517 = -773859243;    double DrZAyInHtLrBvGudoHnw45683740 = -408413303;    double DrZAyInHtLrBvGudoHnw44793657 = -834136312;    double DrZAyInHtLrBvGudoHnw30749567 = -498555652;    double DrZAyInHtLrBvGudoHnw34234676 = -764922438;    double DrZAyInHtLrBvGudoHnw60154315 = -456208297;    double DrZAyInHtLrBvGudoHnw29076307 = -442262973;    double DrZAyInHtLrBvGudoHnw10778628 = -432738115;    double DrZAyInHtLrBvGudoHnw26984818 = -904254865;    double DrZAyInHtLrBvGudoHnw90353555 = -340488659;    double DrZAyInHtLrBvGudoHnw74547787 = -235773131;    double DrZAyInHtLrBvGudoHnw42713278 = -623732742;    double DrZAyInHtLrBvGudoHnw4655793 = -348892694;    double DrZAyInHtLrBvGudoHnw69735782 = -311387240;    double DrZAyInHtLrBvGudoHnw4510828 = -99444412;    double DrZAyInHtLrBvGudoHnw72096729 = -224171675;    double DrZAyInHtLrBvGudoHnw27038262 = -46630845;    double DrZAyInHtLrBvGudoHnw29729652 = -865326086;    double DrZAyInHtLrBvGudoHnw36595666 = -549662315;    double DrZAyInHtLrBvGudoHnw32725570 = 21556454;    double DrZAyInHtLrBvGudoHnw59696330 = -737444169;    double DrZAyInHtLrBvGudoHnw32782706 = -397797973;    double DrZAyInHtLrBvGudoHnw60265931 = -655945609;    double DrZAyInHtLrBvGudoHnw13772221 = -964396481;    double DrZAyInHtLrBvGudoHnw12636242 = -712211899;    double DrZAyInHtLrBvGudoHnw17839989 = -279404571;    double DrZAyInHtLrBvGudoHnw81656906 = -490933494;    double DrZAyInHtLrBvGudoHnw30310010 = -216745882;    double DrZAyInHtLrBvGudoHnw11599108 = -831764829;    double DrZAyInHtLrBvGudoHnw6541247 = 70157546;    double DrZAyInHtLrBvGudoHnw20229048 = -193640176;    double DrZAyInHtLrBvGudoHnw19472260 = -337666364;    double DrZAyInHtLrBvGudoHnw6147452 = -864898380;    double DrZAyInHtLrBvGudoHnw75122411 = -65406650;    double DrZAyInHtLrBvGudoHnw11124956 = -615506945;    double DrZAyInHtLrBvGudoHnw31883041 = -111800721;    double DrZAyInHtLrBvGudoHnw68804745 = 21779487;    double DrZAyInHtLrBvGudoHnw49436406 = -268384241;    double DrZAyInHtLrBvGudoHnw37459710 = -278776767;    double DrZAyInHtLrBvGudoHnw99899484 = -669250102;    double DrZAyInHtLrBvGudoHnw17102409 = -707923896;    double DrZAyInHtLrBvGudoHnw42785206 = -123986912;    double DrZAyInHtLrBvGudoHnw46507064 = -195723078;    double DrZAyInHtLrBvGudoHnw9054237 = -578698537;    double DrZAyInHtLrBvGudoHnw41942359 = -55111289;    double DrZAyInHtLrBvGudoHnw57124077 = -865149707;    double DrZAyInHtLrBvGudoHnw83313990 = -135490907;    double DrZAyInHtLrBvGudoHnw73689344 = -244058863;    double DrZAyInHtLrBvGudoHnw18213637 = -568840345;    double DrZAyInHtLrBvGudoHnw24662672 = -440405502;    double DrZAyInHtLrBvGudoHnw27037465 = -651671929;    double DrZAyInHtLrBvGudoHnw42973308 = -655011366;    double DrZAyInHtLrBvGudoHnw88265443 = -63639267;    double DrZAyInHtLrBvGudoHnw24327265 = -493329439;    double DrZAyInHtLrBvGudoHnw76866701 = -307020694;    double DrZAyInHtLrBvGudoHnw55750811 = -276061271;    double DrZAyInHtLrBvGudoHnw85417808 = -752467695;    double DrZAyInHtLrBvGudoHnw31021436 = -869739831;    double DrZAyInHtLrBvGudoHnw18113325 = -786343753;    double DrZAyInHtLrBvGudoHnw16394688 = -385517867;    double DrZAyInHtLrBvGudoHnw78497409 = -965274804;    double DrZAyInHtLrBvGudoHnw98766296 = -125517091;    double DrZAyInHtLrBvGudoHnw99179519 = -600973287;    double DrZAyInHtLrBvGudoHnw20443572 = -874412411;    double DrZAyInHtLrBvGudoHnw70124508 = -46848483;    double DrZAyInHtLrBvGudoHnw55075528 = -898106768;    double DrZAyInHtLrBvGudoHnw36565826 = -758834363;    double DrZAyInHtLrBvGudoHnw29533382 = -183486045;    double DrZAyInHtLrBvGudoHnw58610827 = -695880296;    double DrZAyInHtLrBvGudoHnw72627786 = -987643692;    double DrZAyInHtLrBvGudoHnw3291985 = -145951162;    double DrZAyInHtLrBvGudoHnw77601855 = -778246604;    double DrZAyInHtLrBvGudoHnw92269942 = -486549320;    double DrZAyInHtLrBvGudoHnw36696182 = -880412213;    double DrZAyInHtLrBvGudoHnw15623162 = -270519650;    double DrZAyInHtLrBvGudoHnw16911125 = -513457258;    double DrZAyInHtLrBvGudoHnw86275641 = -436671363;     DrZAyInHtLrBvGudoHnw63200285 = DrZAyInHtLrBvGudoHnw11685443;     DrZAyInHtLrBvGudoHnw11685443 = DrZAyInHtLrBvGudoHnw99658472;     DrZAyInHtLrBvGudoHnw99658472 = DrZAyInHtLrBvGudoHnw77566780;     DrZAyInHtLrBvGudoHnw77566780 = DrZAyInHtLrBvGudoHnw13598402;     DrZAyInHtLrBvGudoHnw13598402 = DrZAyInHtLrBvGudoHnw80185973;     DrZAyInHtLrBvGudoHnw80185973 = DrZAyInHtLrBvGudoHnw71694386;     DrZAyInHtLrBvGudoHnw71694386 = DrZAyInHtLrBvGudoHnw60053799;     DrZAyInHtLrBvGudoHnw60053799 = DrZAyInHtLrBvGudoHnw46178715;     DrZAyInHtLrBvGudoHnw46178715 = DrZAyInHtLrBvGudoHnw53563833;     DrZAyInHtLrBvGudoHnw53563833 = DrZAyInHtLrBvGudoHnw73491882;     DrZAyInHtLrBvGudoHnw73491882 = DrZAyInHtLrBvGudoHnw99407792;     DrZAyInHtLrBvGudoHnw99407792 = DrZAyInHtLrBvGudoHnw16490146;     DrZAyInHtLrBvGudoHnw16490146 = DrZAyInHtLrBvGudoHnw99837354;     DrZAyInHtLrBvGudoHnw99837354 = DrZAyInHtLrBvGudoHnw87969783;     DrZAyInHtLrBvGudoHnw87969783 = DrZAyInHtLrBvGudoHnw43425127;     DrZAyInHtLrBvGudoHnw43425127 = DrZAyInHtLrBvGudoHnw22724464;     DrZAyInHtLrBvGudoHnw22724464 = DrZAyInHtLrBvGudoHnw96759401;     DrZAyInHtLrBvGudoHnw96759401 = DrZAyInHtLrBvGudoHnw54075726;     DrZAyInHtLrBvGudoHnw54075726 = DrZAyInHtLrBvGudoHnw72702960;     DrZAyInHtLrBvGudoHnw72702960 = DrZAyInHtLrBvGudoHnw24861110;     DrZAyInHtLrBvGudoHnw24861110 = DrZAyInHtLrBvGudoHnw57052834;     DrZAyInHtLrBvGudoHnw57052834 = DrZAyInHtLrBvGudoHnw36563032;     DrZAyInHtLrBvGudoHnw36563032 = DrZAyInHtLrBvGudoHnw88533517;     DrZAyInHtLrBvGudoHnw88533517 = DrZAyInHtLrBvGudoHnw45683740;     DrZAyInHtLrBvGudoHnw45683740 = DrZAyInHtLrBvGudoHnw44793657;     DrZAyInHtLrBvGudoHnw44793657 = DrZAyInHtLrBvGudoHnw30749567;     DrZAyInHtLrBvGudoHnw30749567 = DrZAyInHtLrBvGudoHnw34234676;     DrZAyInHtLrBvGudoHnw34234676 = DrZAyInHtLrBvGudoHnw60154315;     DrZAyInHtLrBvGudoHnw60154315 = DrZAyInHtLrBvGudoHnw29076307;     DrZAyInHtLrBvGudoHnw29076307 = DrZAyInHtLrBvGudoHnw10778628;     DrZAyInHtLrBvGudoHnw10778628 = DrZAyInHtLrBvGudoHnw26984818;     DrZAyInHtLrBvGudoHnw26984818 = DrZAyInHtLrBvGudoHnw90353555;     DrZAyInHtLrBvGudoHnw90353555 = DrZAyInHtLrBvGudoHnw74547787;     DrZAyInHtLrBvGudoHnw74547787 = DrZAyInHtLrBvGudoHnw42713278;     DrZAyInHtLrBvGudoHnw42713278 = DrZAyInHtLrBvGudoHnw4655793;     DrZAyInHtLrBvGudoHnw4655793 = DrZAyInHtLrBvGudoHnw69735782;     DrZAyInHtLrBvGudoHnw69735782 = DrZAyInHtLrBvGudoHnw4510828;     DrZAyInHtLrBvGudoHnw4510828 = DrZAyInHtLrBvGudoHnw72096729;     DrZAyInHtLrBvGudoHnw72096729 = DrZAyInHtLrBvGudoHnw27038262;     DrZAyInHtLrBvGudoHnw27038262 = DrZAyInHtLrBvGudoHnw29729652;     DrZAyInHtLrBvGudoHnw29729652 = DrZAyInHtLrBvGudoHnw36595666;     DrZAyInHtLrBvGudoHnw36595666 = DrZAyInHtLrBvGudoHnw32725570;     DrZAyInHtLrBvGudoHnw32725570 = DrZAyInHtLrBvGudoHnw59696330;     DrZAyInHtLrBvGudoHnw59696330 = DrZAyInHtLrBvGudoHnw32782706;     DrZAyInHtLrBvGudoHnw32782706 = DrZAyInHtLrBvGudoHnw60265931;     DrZAyInHtLrBvGudoHnw60265931 = DrZAyInHtLrBvGudoHnw13772221;     DrZAyInHtLrBvGudoHnw13772221 = DrZAyInHtLrBvGudoHnw12636242;     DrZAyInHtLrBvGudoHnw12636242 = DrZAyInHtLrBvGudoHnw17839989;     DrZAyInHtLrBvGudoHnw17839989 = DrZAyInHtLrBvGudoHnw81656906;     DrZAyInHtLrBvGudoHnw81656906 = DrZAyInHtLrBvGudoHnw30310010;     DrZAyInHtLrBvGudoHnw30310010 = DrZAyInHtLrBvGudoHnw11599108;     DrZAyInHtLrBvGudoHnw11599108 = DrZAyInHtLrBvGudoHnw6541247;     DrZAyInHtLrBvGudoHnw6541247 = DrZAyInHtLrBvGudoHnw20229048;     DrZAyInHtLrBvGudoHnw20229048 = DrZAyInHtLrBvGudoHnw19472260;     DrZAyInHtLrBvGudoHnw19472260 = DrZAyInHtLrBvGudoHnw6147452;     DrZAyInHtLrBvGudoHnw6147452 = DrZAyInHtLrBvGudoHnw75122411;     DrZAyInHtLrBvGudoHnw75122411 = DrZAyInHtLrBvGudoHnw11124956;     DrZAyInHtLrBvGudoHnw11124956 = DrZAyInHtLrBvGudoHnw31883041;     DrZAyInHtLrBvGudoHnw31883041 = DrZAyInHtLrBvGudoHnw68804745;     DrZAyInHtLrBvGudoHnw68804745 = DrZAyInHtLrBvGudoHnw49436406;     DrZAyInHtLrBvGudoHnw49436406 = DrZAyInHtLrBvGudoHnw37459710;     DrZAyInHtLrBvGudoHnw37459710 = DrZAyInHtLrBvGudoHnw99899484;     DrZAyInHtLrBvGudoHnw99899484 = DrZAyInHtLrBvGudoHnw17102409;     DrZAyInHtLrBvGudoHnw17102409 = DrZAyInHtLrBvGudoHnw42785206;     DrZAyInHtLrBvGudoHnw42785206 = DrZAyInHtLrBvGudoHnw46507064;     DrZAyInHtLrBvGudoHnw46507064 = DrZAyInHtLrBvGudoHnw9054237;     DrZAyInHtLrBvGudoHnw9054237 = DrZAyInHtLrBvGudoHnw41942359;     DrZAyInHtLrBvGudoHnw41942359 = DrZAyInHtLrBvGudoHnw57124077;     DrZAyInHtLrBvGudoHnw57124077 = DrZAyInHtLrBvGudoHnw83313990;     DrZAyInHtLrBvGudoHnw83313990 = DrZAyInHtLrBvGudoHnw73689344;     DrZAyInHtLrBvGudoHnw73689344 = DrZAyInHtLrBvGudoHnw18213637;     DrZAyInHtLrBvGudoHnw18213637 = DrZAyInHtLrBvGudoHnw24662672;     DrZAyInHtLrBvGudoHnw24662672 = DrZAyInHtLrBvGudoHnw27037465;     DrZAyInHtLrBvGudoHnw27037465 = DrZAyInHtLrBvGudoHnw42973308;     DrZAyInHtLrBvGudoHnw42973308 = DrZAyInHtLrBvGudoHnw88265443;     DrZAyInHtLrBvGudoHnw88265443 = DrZAyInHtLrBvGudoHnw24327265;     DrZAyInHtLrBvGudoHnw24327265 = DrZAyInHtLrBvGudoHnw76866701;     DrZAyInHtLrBvGudoHnw76866701 = DrZAyInHtLrBvGudoHnw55750811;     DrZAyInHtLrBvGudoHnw55750811 = DrZAyInHtLrBvGudoHnw85417808;     DrZAyInHtLrBvGudoHnw85417808 = DrZAyInHtLrBvGudoHnw31021436;     DrZAyInHtLrBvGudoHnw31021436 = DrZAyInHtLrBvGudoHnw18113325;     DrZAyInHtLrBvGudoHnw18113325 = DrZAyInHtLrBvGudoHnw16394688;     DrZAyInHtLrBvGudoHnw16394688 = DrZAyInHtLrBvGudoHnw78497409;     DrZAyInHtLrBvGudoHnw78497409 = DrZAyInHtLrBvGudoHnw98766296;     DrZAyInHtLrBvGudoHnw98766296 = DrZAyInHtLrBvGudoHnw99179519;     DrZAyInHtLrBvGudoHnw99179519 = DrZAyInHtLrBvGudoHnw20443572;     DrZAyInHtLrBvGudoHnw20443572 = DrZAyInHtLrBvGudoHnw70124508;     DrZAyInHtLrBvGudoHnw70124508 = DrZAyInHtLrBvGudoHnw55075528;     DrZAyInHtLrBvGudoHnw55075528 = DrZAyInHtLrBvGudoHnw36565826;     DrZAyInHtLrBvGudoHnw36565826 = DrZAyInHtLrBvGudoHnw29533382;     DrZAyInHtLrBvGudoHnw29533382 = DrZAyInHtLrBvGudoHnw58610827;     DrZAyInHtLrBvGudoHnw58610827 = DrZAyInHtLrBvGudoHnw72627786;     DrZAyInHtLrBvGudoHnw72627786 = DrZAyInHtLrBvGudoHnw3291985;     DrZAyInHtLrBvGudoHnw3291985 = DrZAyInHtLrBvGudoHnw77601855;     DrZAyInHtLrBvGudoHnw77601855 = DrZAyInHtLrBvGudoHnw92269942;     DrZAyInHtLrBvGudoHnw92269942 = DrZAyInHtLrBvGudoHnw36696182;     DrZAyInHtLrBvGudoHnw36696182 = DrZAyInHtLrBvGudoHnw15623162;     DrZAyInHtLrBvGudoHnw15623162 = DrZAyInHtLrBvGudoHnw16911125;     DrZAyInHtLrBvGudoHnw16911125 = DrZAyInHtLrBvGudoHnw86275641;     DrZAyInHtLrBvGudoHnw86275641 = DrZAyInHtLrBvGudoHnw63200285;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void LOgPDSKwAbiptlCCYbIf71927626() {     double tsfRZxvmIXtXRelrXFfQ88785562 = -961704464;    double tsfRZxvmIXtXRelrXFfQ8082586 = -465349289;    double tsfRZxvmIXtXRelrXFfQ32537648 = -191827723;    double tsfRZxvmIXtXRelrXFfQ23288710 = -627536916;    double tsfRZxvmIXtXRelrXFfQ82940406 = -959279112;    double tsfRZxvmIXtXRelrXFfQ97195249 = 78079104;    double tsfRZxvmIXtXRelrXFfQ77015230 = -625970411;    double tsfRZxvmIXtXRelrXFfQ75063923 = -506140278;    double tsfRZxvmIXtXRelrXFfQ71508008 = -646410913;    double tsfRZxvmIXtXRelrXFfQ72722215 = -991286549;    double tsfRZxvmIXtXRelrXFfQ87825436 = -410405829;    double tsfRZxvmIXtXRelrXFfQ34725448 = -362896102;    double tsfRZxvmIXtXRelrXFfQ93070923 = -155132784;    double tsfRZxvmIXtXRelrXFfQ91397071 = -463406742;    double tsfRZxvmIXtXRelrXFfQ44238671 = -472929558;    double tsfRZxvmIXtXRelrXFfQ51146170 = 8362420;    double tsfRZxvmIXtXRelrXFfQ22522580 = -456604551;    double tsfRZxvmIXtXRelrXFfQ17875306 = -180854537;    double tsfRZxvmIXtXRelrXFfQ15540447 = -481276631;    double tsfRZxvmIXtXRelrXFfQ33864427 = -494776827;    double tsfRZxvmIXtXRelrXFfQ25870375 = -75001362;    double tsfRZxvmIXtXRelrXFfQ58555941 = -991505914;    double tsfRZxvmIXtXRelrXFfQ40326830 = -88556968;    double tsfRZxvmIXtXRelrXFfQ90247559 = -632708223;    double tsfRZxvmIXtXRelrXFfQ586879 = 24090555;    double tsfRZxvmIXtXRelrXFfQ74282793 = -398586853;    double tsfRZxvmIXtXRelrXFfQ13137476 = -64368786;    double tsfRZxvmIXtXRelrXFfQ98133476 = -994584670;    double tsfRZxvmIXtXRelrXFfQ68031527 = -692502148;    double tsfRZxvmIXtXRelrXFfQ82398797 = -351172534;    double tsfRZxvmIXtXRelrXFfQ32825262 = -211385145;    double tsfRZxvmIXtXRelrXFfQ64174750 = -78050699;    double tsfRZxvmIXtXRelrXFfQ11540608 = -788309796;    double tsfRZxvmIXtXRelrXFfQ28134859 = -291918102;    double tsfRZxvmIXtXRelrXFfQ44118266 = -225168654;    double tsfRZxvmIXtXRelrXFfQ9447643 = -866487410;    double tsfRZxvmIXtXRelrXFfQ19381168 = -791899185;    double tsfRZxvmIXtXRelrXFfQ80083514 = 51840763;    double tsfRZxvmIXtXRelrXFfQ13020308 = -682509997;    double tsfRZxvmIXtXRelrXFfQ35821935 = -516601164;    double tsfRZxvmIXtXRelrXFfQ76880933 = -340386276;    double tsfRZxvmIXtXRelrXFfQ80605255 = 41299181;    double tsfRZxvmIXtXRelrXFfQ81925789 = -7049927;    double tsfRZxvmIXtXRelrXFfQ30489610 = -105859768;    double tsfRZxvmIXtXRelrXFfQ24763582 = -336819090;    double tsfRZxvmIXtXRelrXFfQ26666253 = -78493146;    double tsfRZxvmIXtXRelrXFfQ13540676 = -996865342;    double tsfRZxvmIXtXRelrXFfQ20749013 = -905394746;    double tsfRZxvmIXtXRelrXFfQ68651308 = -379080798;    double tsfRZxvmIXtXRelrXFfQ35466901 = -338956647;    double tsfRZxvmIXtXRelrXFfQ40446258 = -465404484;    double tsfRZxvmIXtXRelrXFfQ38857688 = 90155992;    double tsfRZxvmIXtXRelrXFfQ24614850 = -497741003;    double tsfRZxvmIXtXRelrXFfQ89127956 = -508703957;    double tsfRZxvmIXtXRelrXFfQ46057251 = -812756781;    double tsfRZxvmIXtXRelrXFfQ30229622 = -970198551;    double tsfRZxvmIXtXRelrXFfQ67755756 = -276792322;    double tsfRZxvmIXtXRelrXFfQ42290088 = -559119501;    double tsfRZxvmIXtXRelrXFfQ22701831 = -551627472;    double tsfRZxvmIXtXRelrXFfQ8657613 = -460692259;    double tsfRZxvmIXtXRelrXFfQ84057774 = -857552110;    double tsfRZxvmIXtXRelrXFfQ78881754 = -631385741;    double tsfRZxvmIXtXRelrXFfQ7032396 = -813638131;    double tsfRZxvmIXtXRelrXFfQ89109210 = -195238379;    double tsfRZxvmIXtXRelrXFfQ39896954 = -679901404;    double tsfRZxvmIXtXRelrXFfQ23650686 = -232355130;    double tsfRZxvmIXtXRelrXFfQ23184841 = -574586307;    double tsfRZxvmIXtXRelrXFfQ64936065 = -863214683;    double tsfRZxvmIXtXRelrXFfQ47278805 = -138238089;    double tsfRZxvmIXtXRelrXFfQ34791028 = -606442149;    double tsfRZxvmIXtXRelrXFfQ31765002 = -199738396;    double tsfRZxvmIXtXRelrXFfQ42439066 = -408445314;    double tsfRZxvmIXtXRelrXFfQ4854998 = -498344541;    double tsfRZxvmIXtXRelrXFfQ79718511 = -964675468;    double tsfRZxvmIXtXRelrXFfQ56983494 = -54390552;    double tsfRZxvmIXtXRelrXFfQ45265119 = -16300544;    double tsfRZxvmIXtXRelrXFfQ76630151 = -884455987;    double tsfRZxvmIXtXRelrXFfQ9837221 = -982697201;    double tsfRZxvmIXtXRelrXFfQ65483978 = -195889134;    double tsfRZxvmIXtXRelrXFfQ73920626 = -897416299;    double tsfRZxvmIXtXRelrXFfQ60742118 = -401721512;    double tsfRZxvmIXtXRelrXFfQ92388463 = -158974041;    double tsfRZxvmIXtXRelrXFfQ29482168 = -515503873;    double tsfRZxvmIXtXRelrXFfQ32564627 = -253545502;    double tsfRZxvmIXtXRelrXFfQ41952540 = -885768051;    double tsfRZxvmIXtXRelrXFfQ93967574 = -201541138;    double tsfRZxvmIXtXRelrXFfQ39559900 = -580309696;    double tsfRZxvmIXtXRelrXFfQ22412651 = -179605839;    double tsfRZxvmIXtXRelrXFfQ82077607 = -479161322;    double tsfRZxvmIXtXRelrXFfQ13888644 = -254970103;    double tsfRZxvmIXtXRelrXFfQ41691887 = -489695089;    double tsfRZxvmIXtXRelrXFfQ77091079 = -132779685;    double tsfRZxvmIXtXRelrXFfQ57381683 = -396531766;    double tsfRZxvmIXtXRelrXFfQ4362695 = -121817739;    double tsfRZxvmIXtXRelrXFfQ51764161 = -659049054;    double tsfRZxvmIXtXRelrXFfQ97999178 = -709000535;    double tsfRZxvmIXtXRelrXFfQ73572859 = -145062688;    double tsfRZxvmIXtXRelrXFfQ92816578 = -811811549;    double tsfRZxvmIXtXRelrXFfQ90592656 = -425958364;    double tsfRZxvmIXtXRelrXFfQ1112897 = -961704464;     tsfRZxvmIXtXRelrXFfQ88785562 = tsfRZxvmIXtXRelrXFfQ8082586;     tsfRZxvmIXtXRelrXFfQ8082586 = tsfRZxvmIXtXRelrXFfQ32537648;     tsfRZxvmIXtXRelrXFfQ32537648 = tsfRZxvmIXtXRelrXFfQ23288710;     tsfRZxvmIXtXRelrXFfQ23288710 = tsfRZxvmIXtXRelrXFfQ82940406;     tsfRZxvmIXtXRelrXFfQ82940406 = tsfRZxvmIXtXRelrXFfQ97195249;     tsfRZxvmIXtXRelrXFfQ97195249 = tsfRZxvmIXtXRelrXFfQ77015230;     tsfRZxvmIXtXRelrXFfQ77015230 = tsfRZxvmIXtXRelrXFfQ75063923;     tsfRZxvmIXtXRelrXFfQ75063923 = tsfRZxvmIXtXRelrXFfQ71508008;     tsfRZxvmIXtXRelrXFfQ71508008 = tsfRZxvmIXtXRelrXFfQ72722215;     tsfRZxvmIXtXRelrXFfQ72722215 = tsfRZxvmIXtXRelrXFfQ87825436;     tsfRZxvmIXtXRelrXFfQ87825436 = tsfRZxvmIXtXRelrXFfQ34725448;     tsfRZxvmIXtXRelrXFfQ34725448 = tsfRZxvmIXtXRelrXFfQ93070923;     tsfRZxvmIXtXRelrXFfQ93070923 = tsfRZxvmIXtXRelrXFfQ91397071;     tsfRZxvmIXtXRelrXFfQ91397071 = tsfRZxvmIXtXRelrXFfQ44238671;     tsfRZxvmIXtXRelrXFfQ44238671 = tsfRZxvmIXtXRelrXFfQ51146170;     tsfRZxvmIXtXRelrXFfQ51146170 = tsfRZxvmIXtXRelrXFfQ22522580;     tsfRZxvmIXtXRelrXFfQ22522580 = tsfRZxvmIXtXRelrXFfQ17875306;     tsfRZxvmIXtXRelrXFfQ17875306 = tsfRZxvmIXtXRelrXFfQ15540447;     tsfRZxvmIXtXRelrXFfQ15540447 = tsfRZxvmIXtXRelrXFfQ33864427;     tsfRZxvmIXtXRelrXFfQ33864427 = tsfRZxvmIXtXRelrXFfQ25870375;     tsfRZxvmIXtXRelrXFfQ25870375 = tsfRZxvmIXtXRelrXFfQ58555941;     tsfRZxvmIXtXRelrXFfQ58555941 = tsfRZxvmIXtXRelrXFfQ40326830;     tsfRZxvmIXtXRelrXFfQ40326830 = tsfRZxvmIXtXRelrXFfQ90247559;     tsfRZxvmIXtXRelrXFfQ90247559 = tsfRZxvmIXtXRelrXFfQ586879;     tsfRZxvmIXtXRelrXFfQ586879 = tsfRZxvmIXtXRelrXFfQ74282793;     tsfRZxvmIXtXRelrXFfQ74282793 = tsfRZxvmIXtXRelrXFfQ13137476;     tsfRZxvmIXtXRelrXFfQ13137476 = tsfRZxvmIXtXRelrXFfQ98133476;     tsfRZxvmIXtXRelrXFfQ98133476 = tsfRZxvmIXtXRelrXFfQ68031527;     tsfRZxvmIXtXRelrXFfQ68031527 = tsfRZxvmIXtXRelrXFfQ82398797;     tsfRZxvmIXtXRelrXFfQ82398797 = tsfRZxvmIXtXRelrXFfQ32825262;     tsfRZxvmIXtXRelrXFfQ32825262 = tsfRZxvmIXtXRelrXFfQ64174750;     tsfRZxvmIXtXRelrXFfQ64174750 = tsfRZxvmIXtXRelrXFfQ11540608;     tsfRZxvmIXtXRelrXFfQ11540608 = tsfRZxvmIXtXRelrXFfQ28134859;     tsfRZxvmIXtXRelrXFfQ28134859 = tsfRZxvmIXtXRelrXFfQ44118266;     tsfRZxvmIXtXRelrXFfQ44118266 = tsfRZxvmIXtXRelrXFfQ9447643;     tsfRZxvmIXtXRelrXFfQ9447643 = tsfRZxvmIXtXRelrXFfQ19381168;     tsfRZxvmIXtXRelrXFfQ19381168 = tsfRZxvmIXtXRelrXFfQ80083514;     tsfRZxvmIXtXRelrXFfQ80083514 = tsfRZxvmIXtXRelrXFfQ13020308;     tsfRZxvmIXtXRelrXFfQ13020308 = tsfRZxvmIXtXRelrXFfQ35821935;     tsfRZxvmIXtXRelrXFfQ35821935 = tsfRZxvmIXtXRelrXFfQ76880933;     tsfRZxvmIXtXRelrXFfQ76880933 = tsfRZxvmIXtXRelrXFfQ80605255;     tsfRZxvmIXtXRelrXFfQ80605255 = tsfRZxvmIXtXRelrXFfQ81925789;     tsfRZxvmIXtXRelrXFfQ81925789 = tsfRZxvmIXtXRelrXFfQ30489610;     tsfRZxvmIXtXRelrXFfQ30489610 = tsfRZxvmIXtXRelrXFfQ24763582;     tsfRZxvmIXtXRelrXFfQ24763582 = tsfRZxvmIXtXRelrXFfQ26666253;     tsfRZxvmIXtXRelrXFfQ26666253 = tsfRZxvmIXtXRelrXFfQ13540676;     tsfRZxvmIXtXRelrXFfQ13540676 = tsfRZxvmIXtXRelrXFfQ20749013;     tsfRZxvmIXtXRelrXFfQ20749013 = tsfRZxvmIXtXRelrXFfQ68651308;     tsfRZxvmIXtXRelrXFfQ68651308 = tsfRZxvmIXtXRelrXFfQ35466901;     tsfRZxvmIXtXRelrXFfQ35466901 = tsfRZxvmIXtXRelrXFfQ40446258;     tsfRZxvmIXtXRelrXFfQ40446258 = tsfRZxvmIXtXRelrXFfQ38857688;     tsfRZxvmIXtXRelrXFfQ38857688 = tsfRZxvmIXtXRelrXFfQ24614850;     tsfRZxvmIXtXRelrXFfQ24614850 = tsfRZxvmIXtXRelrXFfQ89127956;     tsfRZxvmIXtXRelrXFfQ89127956 = tsfRZxvmIXtXRelrXFfQ46057251;     tsfRZxvmIXtXRelrXFfQ46057251 = tsfRZxvmIXtXRelrXFfQ30229622;     tsfRZxvmIXtXRelrXFfQ30229622 = tsfRZxvmIXtXRelrXFfQ67755756;     tsfRZxvmIXtXRelrXFfQ67755756 = tsfRZxvmIXtXRelrXFfQ42290088;     tsfRZxvmIXtXRelrXFfQ42290088 = tsfRZxvmIXtXRelrXFfQ22701831;     tsfRZxvmIXtXRelrXFfQ22701831 = tsfRZxvmIXtXRelrXFfQ8657613;     tsfRZxvmIXtXRelrXFfQ8657613 = tsfRZxvmIXtXRelrXFfQ84057774;     tsfRZxvmIXtXRelrXFfQ84057774 = tsfRZxvmIXtXRelrXFfQ78881754;     tsfRZxvmIXtXRelrXFfQ78881754 = tsfRZxvmIXtXRelrXFfQ7032396;     tsfRZxvmIXtXRelrXFfQ7032396 = tsfRZxvmIXtXRelrXFfQ89109210;     tsfRZxvmIXtXRelrXFfQ89109210 = tsfRZxvmIXtXRelrXFfQ39896954;     tsfRZxvmIXtXRelrXFfQ39896954 = tsfRZxvmIXtXRelrXFfQ23650686;     tsfRZxvmIXtXRelrXFfQ23650686 = tsfRZxvmIXtXRelrXFfQ23184841;     tsfRZxvmIXtXRelrXFfQ23184841 = tsfRZxvmIXtXRelrXFfQ64936065;     tsfRZxvmIXtXRelrXFfQ64936065 = tsfRZxvmIXtXRelrXFfQ47278805;     tsfRZxvmIXtXRelrXFfQ47278805 = tsfRZxvmIXtXRelrXFfQ34791028;     tsfRZxvmIXtXRelrXFfQ34791028 = tsfRZxvmIXtXRelrXFfQ31765002;     tsfRZxvmIXtXRelrXFfQ31765002 = tsfRZxvmIXtXRelrXFfQ42439066;     tsfRZxvmIXtXRelrXFfQ42439066 = tsfRZxvmIXtXRelrXFfQ4854998;     tsfRZxvmIXtXRelrXFfQ4854998 = tsfRZxvmIXtXRelrXFfQ79718511;     tsfRZxvmIXtXRelrXFfQ79718511 = tsfRZxvmIXtXRelrXFfQ56983494;     tsfRZxvmIXtXRelrXFfQ56983494 = tsfRZxvmIXtXRelrXFfQ45265119;     tsfRZxvmIXtXRelrXFfQ45265119 = tsfRZxvmIXtXRelrXFfQ76630151;     tsfRZxvmIXtXRelrXFfQ76630151 = tsfRZxvmIXtXRelrXFfQ9837221;     tsfRZxvmIXtXRelrXFfQ9837221 = tsfRZxvmIXtXRelrXFfQ65483978;     tsfRZxvmIXtXRelrXFfQ65483978 = tsfRZxvmIXtXRelrXFfQ73920626;     tsfRZxvmIXtXRelrXFfQ73920626 = tsfRZxvmIXtXRelrXFfQ60742118;     tsfRZxvmIXtXRelrXFfQ60742118 = tsfRZxvmIXtXRelrXFfQ92388463;     tsfRZxvmIXtXRelrXFfQ92388463 = tsfRZxvmIXtXRelrXFfQ29482168;     tsfRZxvmIXtXRelrXFfQ29482168 = tsfRZxvmIXtXRelrXFfQ32564627;     tsfRZxvmIXtXRelrXFfQ32564627 = tsfRZxvmIXtXRelrXFfQ41952540;     tsfRZxvmIXtXRelrXFfQ41952540 = tsfRZxvmIXtXRelrXFfQ93967574;     tsfRZxvmIXtXRelrXFfQ93967574 = tsfRZxvmIXtXRelrXFfQ39559900;     tsfRZxvmIXtXRelrXFfQ39559900 = tsfRZxvmIXtXRelrXFfQ22412651;     tsfRZxvmIXtXRelrXFfQ22412651 = tsfRZxvmIXtXRelrXFfQ82077607;     tsfRZxvmIXtXRelrXFfQ82077607 = tsfRZxvmIXtXRelrXFfQ13888644;     tsfRZxvmIXtXRelrXFfQ13888644 = tsfRZxvmIXtXRelrXFfQ41691887;     tsfRZxvmIXtXRelrXFfQ41691887 = tsfRZxvmIXtXRelrXFfQ77091079;     tsfRZxvmIXtXRelrXFfQ77091079 = tsfRZxvmIXtXRelrXFfQ57381683;     tsfRZxvmIXtXRelrXFfQ57381683 = tsfRZxvmIXtXRelrXFfQ4362695;     tsfRZxvmIXtXRelrXFfQ4362695 = tsfRZxvmIXtXRelrXFfQ51764161;     tsfRZxvmIXtXRelrXFfQ51764161 = tsfRZxvmIXtXRelrXFfQ97999178;     tsfRZxvmIXtXRelrXFfQ97999178 = tsfRZxvmIXtXRelrXFfQ73572859;     tsfRZxvmIXtXRelrXFfQ73572859 = tsfRZxvmIXtXRelrXFfQ92816578;     tsfRZxvmIXtXRelrXFfQ92816578 = tsfRZxvmIXtXRelrXFfQ90592656;     tsfRZxvmIXtXRelrXFfQ90592656 = tsfRZxvmIXtXRelrXFfQ1112897;     tsfRZxvmIXtXRelrXFfQ1112897 = tsfRZxvmIXtXRelrXFfQ88785562;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void WrgTIVebuNHQuEGUqQLM99521882() {     double UQHDrGLowqAVdEodSePX14370840 = -386737565;    double UQHDrGLowqAVdEodSePX4479730 = -720827066;    double UQHDrGLowqAVdEodSePX65416823 = 5710741;    double UQHDrGLowqAVdEodSePX69010639 = -634859810;    double UQHDrGLowqAVdEodSePX52282411 = 93798601;    double UQHDrGLowqAVdEodSePX14204526 = -76901899;    double UQHDrGLowqAVdEodSePX82336075 = -108241618;    double UQHDrGLowqAVdEodSePX90074047 = -886822157;    double UQHDrGLowqAVdEodSePX96837300 = -42634957;    double UQHDrGLowqAVdEodSePX91880597 = -225848072;    double UQHDrGLowqAVdEodSePX2158990 = -720833715;    double UQHDrGLowqAVdEodSePX70043104 = -806605009;    double UQHDrGLowqAVdEodSePX69651702 = 80618850;    double UQHDrGLowqAVdEodSePX82956788 = -437931036;    double UQHDrGLowqAVdEodSePX507559 = -361475516;    double UQHDrGLowqAVdEodSePX58867213 = -427829057;    double UQHDrGLowqAVdEodSePX22320697 = -144924345;    double UQHDrGLowqAVdEodSePX38991210 = -697131898;    double UQHDrGLowqAVdEodSePX77005168 = -164250488;    double UQHDrGLowqAVdEodSePX95025893 = -469216203;    double UQHDrGLowqAVdEodSePX26879640 = -536701143;    double UQHDrGLowqAVdEodSePX60059047 = -311238843;    double UQHDrGLowqAVdEodSePX44090627 = -132649073;    double UQHDrGLowqAVdEodSePX91961602 = -491557203;    double UQHDrGLowqAVdEodSePX55490018 = -643405587;    double UQHDrGLowqAVdEodSePX3771931 = 36962605;    double UQHDrGLowqAVdEodSePX95525384 = -730181921;    double UQHDrGLowqAVdEodSePX62032276 = -124246903;    double UQHDrGLowqAVdEodSePX75908740 = -928795998;    double UQHDrGLowqAVdEodSePX35721289 = -260082096;    double UQHDrGLowqAVdEodSePX54871896 = 9967824;    double UQHDrGLowqAVdEodSePX1364683 = -351846532;    double UQHDrGLowqAVdEodSePX32727659 = -136130933;    double UQHDrGLowqAVdEodSePX81721930 = -348063074;    double UQHDrGLowqAVdEodSePX45523254 = -926604565;    double UQHDrGLowqAVdEodSePX14239494 = -284082126;    double UQHDrGLowqAVdEodSePX69026553 = -172411130;    double UQHDrGLowqAVdEodSePX55656201 = -896874063;    double UQHDrGLowqAVdEodSePX53943887 = -40848319;    double UQHDrGLowqAVdEodSePX44605609 = -986571483;    double UQHDrGLowqAVdEodSePX24032214 = -915446466;    double UQHDrGLowqAVdEodSePX24614846 = -467739323;    double UQHDrGLowqAVdEodSePX31126009 = -35656309;    double UQHDrGLowqAVdEodSePX1282889 = -574275367;    double UQHDrGLowqAVdEodSePX16744458 = -275840207;    double UQHDrGLowqAVdEodSePX93066575 = -601040684;    double UQHDrGLowqAVdEodSePX13309130 = 70665798;    double UQHDrGLowqAVdEodSePX28861784 = 1422407;    double UQHDrGLowqAVdEodSePX19462629 = -478757024;    double UQHDrGLowqAVdEodSePX89276896 = -186979800;    double UQHDrGLowqAVdEodSePX50582505 = -714063087;    double UQHDrGLowqAVdEodSePX66116267 = -87923187;    double UQHDrGLowqAVdEodSePX42688454 = 34360448;    double UQHDrGLowqAVdEodSePX58026865 = -823767739;    double UQHDrGLowqAVdEodSePX72642242 = -187847198;    double UQHDrGLowqAVdEodSePX54311792 = 24501278;    double UQHDrGLowqAVdEodSePX60389102 = -488177993;    double UQHDrGLowqAVdEodSePX73455221 = -502732057;    double UQHDrGLowqAVdEodSePX13520622 = -991454223;    double UQHDrGLowqAVdEodSePX48510481 = -943164004;    double UQHDrGLowqAVdEodSePX18679142 = -346719979;    double UQHDrGLowqAVdEodSePX20303799 = -983994716;    double UQHDrGLowqAVdEodSePX14165308 = -958026160;    double UQHDrGLowqAVdEodSePX61116012 = -782552862;    double UQHDrGLowqAVdEodSePX37008702 = -135815897;    double UQHDrGLowqAVdEodSePX794308 = -268987183;    double UQHDrGLowqAVdEodSePX37315445 = -570474076;    double UQHDrGLowqAVdEodSePX87929771 = -571318076;    double UQHDrGLowqAVdEodSePX37433534 = -511326471;    double UQHDrGLowqAVdEodSePX86268065 = 22606610;    double UQHDrGLowqAVdEodSePX89840659 = -155417928;    double UQHDrGLowqAVdEodSePX66664496 = -248050283;    double UQHDrGLowqAVdEodSePX85047323 = -556283579;    double UQHDrGLowqAVdEodSePX32399559 = -177679006;    double UQHDrGLowqAVdEodSePX70993679 = -553769737;    double UQHDrGLowqAVdEodSePX2264795 = 31038179;    double UQHDrGLowqAVdEodSePX28933039 = -175582535;    double UQHDrGLowqAVdEodSePX42807739 = -558373707;    double UQHDrGLowqAVdEodSePX75217144 = -115716996;    double UQHDrGLowqAVdEodSePX62423443 = 57635096;    double UQHDrGLowqAVdEodSePX90462800 = 66296807;    double UQHDrGLowqAVdEodSePX66663601 = -631604329;    double UQHDrGLowqAVdEodSePX42569648 = -645489879;    double UQHDrGLowqAVdEodSePX86631844 = -641816199;    double UQHDrGLowqAVdEodSePX85138783 = -546019010;    double UQHDrGLowqAVdEodSePX88755628 = -902108989;    double UQHDrGLowqAVdEodSePX58676229 = -286206981;    double UQHDrGLowqAVdEodSePX74700794 = -312363194;    double UQHDrGLowqAVdEodSePX9079688 = -60215876;    double UQHDrGLowqAVdEodSePX91211461 = -851105843;    double UQHDrGLowqAVdEodSePX53850392 = -795904133;    double UQHDrGLowqAVdEodSePX95571331 = -669679073;    double UQHDrGLowqAVdEodSePX42135579 = -905419840;    double UQHDrGLowqAVdEodSePX5433406 = -97684315;    double UQHDrGLowqAVdEodSePX25926468 = -539851505;    double UQHDrGLowqAVdEodSePX3728416 = -931451750;    double UQHDrGLowqAVdEodSePX10449538 = -509713164;    double UQHDrGLowqAVdEodSePX70009996 = -253103447;    double UQHDrGLowqAVdEodSePX64274187 = -338459471;    double UQHDrGLowqAVdEodSePX15950151 = -386737565;     UQHDrGLowqAVdEodSePX14370840 = UQHDrGLowqAVdEodSePX4479730;     UQHDrGLowqAVdEodSePX4479730 = UQHDrGLowqAVdEodSePX65416823;     UQHDrGLowqAVdEodSePX65416823 = UQHDrGLowqAVdEodSePX69010639;     UQHDrGLowqAVdEodSePX69010639 = UQHDrGLowqAVdEodSePX52282411;     UQHDrGLowqAVdEodSePX52282411 = UQHDrGLowqAVdEodSePX14204526;     UQHDrGLowqAVdEodSePX14204526 = UQHDrGLowqAVdEodSePX82336075;     UQHDrGLowqAVdEodSePX82336075 = UQHDrGLowqAVdEodSePX90074047;     UQHDrGLowqAVdEodSePX90074047 = UQHDrGLowqAVdEodSePX96837300;     UQHDrGLowqAVdEodSePX96837300 = UQHDrGLowqAVdEodSePX91880597;     UQHDrGLowqAVdEodSePX91880597 = UQHDrGLowqAVdEodSePX2158990;     UQHDrGLowqAVdEodSePX2158990 = UQHDrGLowqAVdEodSePX70043104;     UQHDrGLowqAVdEodSePX70043104 = UQHDrGLowqAVdEodSePX69651702;     UQHDrGLowqAVdEodSePX69651702 = UQHDrGLowqAVdEodSePX82956788;     UQHDrGLowqAVdEodSePX82956788 = UQHDrGLowqAVdEodSePX507559;     UQHDrGLowqAVdEodSePX507559 = UQHDrGLowqAVdEodSePX58867213;     UQHDrGLowqAVdEodSePX58867213 = UQHDrGLowqAVdEodSePX22320697;     UQHDrGLowqAVdEodSePX22320697 = UQHDrGLowqAVdEodSePX38991210;     UQHDrGLowqAVdEodSePX38991210 = UQHDrGLowqAVdEodSePX77005168;     UQHDrGLowqAVdEodSePX77005168 = UQHDrGLowqAVdEodSePX95025893;     UQHDrGLowqAVdEodSePX95025893 = UQHDrGLowqAVdEodSePX26879640;     UQHDrGLowqAVdEodSePX26879640 = UQHDrGLowqAVdEodSePX60059047;     UQHDrGLowqAVdEodSePX60059047 = UQHDrGLowqAVdEodSePX44090627;     UQHDrGLowqAVdEodSePX44090627 = UQHDrGLowqAVdEodSePX91961602;     UQHDrGLowqAVdEodSePX91961602 = UQHDrGLowqAVdEodSePX55490018;     UQHDrGLowqAVdEodSePX55490018 = UQHDrGLowqAVdEodSePX3771931;     UQHDrGLowqAVdEodSePX3771931 = UQHDrGLowqAVdEodSePX95525384;     UQHDrGLowqAVdEodSePX95525384 = UQHDrGLowqAVdEodSePX62032276;     UQHDrGLowqAVdEodSePX62032276 = UQHDrGLowqAVdEodSePX75908740;     UQHDrGLowqAVdEodSePX75908740 = UQHDrGLowqAVdEodSePX35721289;     UQHDrGLowqAVdEodSePX35721289 = UQHDrGLowqAVdEodSePX54871896;     UQHDrGLowqAVdEodSePX54871896 = UQHDrGLowqAVdEodSePX1364683;     UQHDrGLowqAVdEodSePX1364683 = UQHDrGLowqAVdEodSePX32727659;     UQHDrGLowqAVdEodSePX32727659 = UQHDrGLowqAVdEodSePX81721930;     UQHDrGLowqAVdEodSePX81721930 = UQHDrGLowqAVdEodSePX45523254;     UQHDrGLowqAVdEodSePX45523254 = UQHDrGLowqAVdEodSePX14239494;     UQHDrGLowqAVdEodSePX14239494 = UQHDrGLowqAVdEodSePX69026553;     UQHDrGLowqAVdEodSePX69026553 = UQHDrGLowqAVdEodSePX55656201;     UQHDrGLowqAVdEodSePX55656201 = UQHDrGLowqAVdEodSePX53943887;     UQHDrGLowqAVdEodSePX53943887 = UQHDrGLowqAVdEodSePX44605609;     UQHDrGLowqAVdEodSePX44605609 = UQHDrGLowqAVdEodSePX24032214;     UQHDrGLowqAVdEodSePX24032214 = UQHDrGLowqAVdEodSePX24614846;     UQHDrGLowqAVdEodSePX24614846 = UQHDrGLowqAVdEodSePX31126009;     UQHDrGLowqAVdEodSePX31126009 = UQHDrGLowqAVdEodSePX1282889;     UQHDrGLowqAVdEodSePX1282889 = UQHDrGLowqAVdEodSePX16744458;     UQHDrGLowqAVdEodSePX16744458 = UQHDrGLowqAVdEodSePX93066575;     UQHDrGLowqAVdEodSePX93066575 = UQHDrGLowqAVdEodSePX13309130;     UQHDrGLowqAVdEodSePX13309130 = UQHDrGLowqAVdEodSePX28861784;     UQHDrGLowqAVdEodSePX28861784 = UQHDrGLowqAVdEodSePX19462629;     UQHDrGLowqAVdEodSePX19462629 = UQHDrGLowqAVdEodSePX89276896;     UQHDrGLowqAVdEodSePX89276896 = UQHDrGLowqAVdEodSePX50582505;     UQHDrGLowqAVdEodSePX50582505 = UQHDrGLowqAVdEodSePX66116267;     UQHDrGLowqAVdEodSePX66116267 = UQHDrGLowqAVdEodSePX42688454;     UQHDrGLowqAVdEodSePX42688454 = UQHDrGLowqAVdEodSePX58026865;     UQHDrGLowqAVdEodSePX58026865 = UQHDrGLowqAVdEodSePX72642242;     UQHDrGLowqAVdEodSePX72642242 = UQHDrGLowqAVdEodSePX54311792;     UQHDrGLowqAVdEodSePX54311792 = UQHDrGLowqAVdEodSePX60389102;     UQHDrGLowqAVdEodSePX60389102 = UQHDrGLowqAVdEodSePX73455221;     UQHDrGLowqAVdEodSePX73455221 = UQHDrGLowqAVdEodSePX13520622;     UQHDrGLowqAVdEodSePX13520622 = UQHDrGLowqAVdEodSePX48510481;     UQHDrGLowqAVdEodSePX48510481 = UQHDrGLowqAVdEodSePX18679142;     UQHDrGLowqAVdEodSePX18679142 = UQHDrGLowqAVdEodSePX20303799;     UQHDrGLowqAVdEodSePX20303799 = UQHDrGLowqAVdEodSePX14165308;     UQHDrGLowqAVdEodSePX14165308 = UQHDrGLowqAVdEodSePX61116012;     UQHDrGLowqAVdEodSePX61116012 = UQHDrGLowqAVdEodSePX37008702;     UQHDrGLowqAVdEodSePX37008702 = UQHDrGLowqAVdEodSePX794308;     UQHDrGLowqAVdEodSePX794308 = UQHDrGLowqAVdEodSePX37315445;     UQHDrGLowqAVdEodSePX37315445 = UQHDrGLowqAVdEodSePX87929771;     UQHDrGLowqAVdEodSePX87929771 = UQHDrGLowqAVdEodSePX37433534;     UQHDrGLowqAVdEodSePX37433534 = UQHDrGLowqAVdEodSePX86268065;     UQHDrGLowqAVdEodSePX86268065 = UQHDrGLowqAVdEodSePX89840659;     UQHDrGLowqAVdEodSePX89840659 = UQHDrGLowqAVdEodSePX66664496;     UQHDrGLowqAVdEodSePX66664496 = UQHDrGLowqAVdEodSePX85047323;     UQHDrGLowqAVdEodSePX85047323 = UQHDrGLowqAVdEodSePX32399559;     UQHDrGLowqAVdEodSePX32399559 = UQHDrGLowqAVdEodSePX70993679;     UQHDrGLowqAVdEodSePX70993679 = UQHDrGLowqAVdEodSePX2264795;     UQHDrGLowqAVdEodSePX2264795 = UQHDrGLowqAVdEodSePX28933039;     UQHDrGLowqAVdEodSePX28933039 = UQHDrGLowqAVdEodSePX42807739;     UQHDrGLowqAVdEodSePX42807739 = UQHDrGLowqAVdEodSePX75217144;     UQHDrGLowqAVdEodSePX75217144 = UQHDrGLowqAVdEodSePX62423443;     UQHDrGLowqAVdEodSePX62423443 = UQHDrGLowqAVdEodSePX90462800;     UQHDrGLowqAVdEodSePX90462800 = UQHDrGLowqAVdEodSePX66663601;     UQHDrGLowqAVdEodSePX66663601 = UQHDrGLowqAVdEodSePX42569648;     UQHDrGLowqAVdEodSePX42569648 = UQHDrGLowqAVdEodSePX86631844;     UQHDrGLowqAVdEodSePX86631844 = UQHDrGLowqAVdEodSePX85138783;     UQHDrGLowqAVdEodSePX85138783 = UQHDrGLowqAVdEodSePX88755628;     UQHDrGLowqAVdEodSePX88755628 = UQHDrGLowqAVdEodSePX58676229;     UQHDrGLowqAVdEodSePX58676229 = UQHDrGLowqAVdEodSePX74700794;     UQHDrGLowqAVdEodSePX74700794 = UQHDrGLowqAVdEodSePX9079688;     UQHDrGLowqAVdEodSePX9079688 = UQHDrGLowqAVdEodSePX91211461;     UQHDrGLowqAVdEodSePX91211461 = UQHDrGLowqAVdEodSePX53850392;     UQHDrGLowqAVdEodSePX53850392 = UQHDrGLowqAVdEodSePX95571331;     UQHDrGLowqAVdEodSePX95571331 = UQHDrGLowqAVdEodSePX42135579;     UQHDrGLowqAVdEodSePX42135579 = UQHDrGLowqAVdEodSePX5433406;     UQHDrGLowqAVdEodSePX5433406 = UQHDrGLowqAVdEodSePX25926468;     UQHDrGLowqAVdEodSePX25926468 = UQHDrGLowqAVdEodSePX3728416;     UQHDrGLowqAVdEodSePX3728416 = UQHDrGLowqAVdEodSePX10449538;     UQHDrGLowqAVdEodSePX10449538 = UQHDrGLowqAVdEodSePX70009996;     UQHDrGLowqAVdEodSePX70009996 = UQHDrGLowqAVdEodSePX64274187;     UQHDrGLowqAVdEodSePX64274187 = UQHDrGLowqAVdEodSePX15950151;     UQHDrGLowqAVdEodSePX15950151 = UQHDrGLowqAVdEodSePX14370840;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zudfNbPWqjmNYKHfcIsU82515116() {     double tDTwuzUkaxbcCVuKTARw19023620 = 32603816;    double tDTwuzUkaxbcCVuKTARw86120427 = -211040661;    double tDTwuzUkaxbcCVuKTARw1477774 = -93956026;    double tDTwuzUkaxbcCVuKTARw36715546 = -871097428;    double tDTwuzUkaxbcCVuKTARw28944108 = -245725469;    double tDTwuzUkaxbcCVuKTARw37080281 = -130910938;    double tDTwuzUkaxbcCVuKTARw45573613 = -61268407;    double tDTwuzUkaxbcCVuKTARw86552663 = -763434719;    double tDTwuzUkaxbcCVuKTARw55296752 = -292899846;    double tDTwuzUkaxbcCVuKTARw73672461 = -138851060;    double tDTwuzUkaxbcCVuKTARw85850436 = -786058305;    double tDTwuzUkaxbcCVuKTARw36380737 = -376898688;    double tDTwuzUkaxbcCVuKTARw91432245 = -553336073;    double tDTwuzUkaxbcCVuKTARw64384274 = -686322877;    double tDTwuzUkaxbcCVuKTARw92024659 = -259275361;    double tDTwuzUkaxbcCVuKTARw42275885 = -416187534;    double tDTwuzUkaxbcCVuKTARw36115650 = -23427816;    double tDTwuzUkaxbcCVuKTARw87337930 = -584964271;    double tDTwuzUkaxbcCVuKTARw58332836 = -54557603;    double tDTwuzUkaxbcCVuKTARw80678889 = -417588462;    double tDTwuzUkaxbcCVuKTARw80971384 = -482400888;    double tDTwuzUkaxbcCVuKTARw55885703 = -330180922;    double tDTwuzUkaxbcCVuKTARw89780058 = -559131955;    double tDTwuzUkaxbcCVuKTARw54302498 = -710828150;    double tDTwuzUkaxbcCVuKTARw33353307 = -297992503;    double tDTwuzUkaxbcCVuKTARw74156396 = -493797662;    double tDTwuzUkaxbcCVuKTARw90070704 = -260226128;    double tDTwuzUkaxbcCVuKTARw24132158 = -834540511;    double tDTwuzUkaxbcCVuKTARw68976028 = -95558452;    double tDTwuzUkaxbcCVuKTARw24179165 = -469697908;    double tDTwuzUkaxbcCVuKTARw14930595 = -85678011;    double tDTwuzUkaxbcCVuKTARw28537258 = -311298501;    double tDTwuzUkaxbcCVuKTARw71746641 = -468134601;    double tDTwuzUkaxbcCVuKTARw76448532 = -822354317;    double tDTwuzUkaxbcCVuKTARw50616920 = -109296306;    double tDTwuzUkaxbcCVuKTARw12273083 = -827019159;    double tDTwuzUkaxbcCVuKTARw21716383 = -872751095;    double tDTwuzUkaxbcCVuKTARw36112860 = -769618753;    double tDTwuzUkaxbcCVuKTARw63541935 = -875967275;    double tDTwuzUkaxbcCVuKTARw61560227 = -272238004;    double tDTwuzUkaxbcCVuKTARw76955531 = -774982598;    double tDTwuzUkaxbcCVuKTARw57713919 = -23719430;    double tDTwuzUkaxbcCVuKTARw12197032 = -565944190;    double tDTwuzUkaxbcCVuKTARw50085262 = 62803857;    double tDTwuzUkaxbcCVuKTARw47733034 = -251672655;    double tDTwuzUkaxbcCVuKTARw58507169 = -347841466;    double tDTwuzUkaxbcCVuKTARw86707289 = -626711742;    double tDTwuzUkaxbcCVuKTARw96134987 = -953420304;    double tDTwuzUkaxbcCVuKTARw91469992 = -544428178;    double tDTwuzUkaxbcCVuKTARw89129914 = -323755316;    double tDTwuzUkaxbcCVuKTARw79010887 = -265917340;    double tDTwuzUkaxbcCVuKTARw9135232 = -973525607;    double tDTwuzUkaxbcCVuKTARw18278544 = 46431489;    double tDTwuzUkaxbcCVuKTARw68005190 = -577634186;    double tDTwuzUkaxbcCVuKTARw1543733 = -522980699;    double tDTwuzUkaxbcCVuKTARw63137916 = -637215263;    double tDTwuzUkaxbcCVuKTARw96340369 = -651908707;    double tDTwuzUkaxbcCVuKTARw47175275 = -383127877;    double tDTwuzUkaxbcCVuKTARw3362240 = -473104926;    double tDTwuzUkaxbcCVuKTARw54787711 = -751927807;    double tDTwuzUkaxbcCVuKTARw47009577 = -870684811;    double tDTwuzUkaxbcCVuKTARw21441455 = -226727897;    double tDTwuzUkaxbcCVuKTARw17576636 = -567876267;    double tDTwuzUkaxbcCVuKTARw31117587 = -823201938;    double tDTwuzUkaxbcCVuKTARw58741867 = 46826950;    double tDTwuzUkaxbcCVuKTARw57313179 = -374759805;    double tDTwuzUkaxbcCVuKTARw64634095 = -908764088;    double tDTwuzUkaxbcCVuKTARw14983713 = -730981757;    double tDTwuzUkaxbcCVuKTARw13767354 = -477026571;    double tDTwuzUkaxbcCVuKTARw79751576 = -432256202;    double tDTwuzUkaxbcCVuKTARw20559503 = -543436440;    double tDTwuzUkaxbcCVuKTARw2791 = -253809063;    double tDTwuzUkaxbcCVuKTARw23795996 = -708996997;    double tDTwuzUkaxbcCVuKTARw96772609 = -782319600;    double tDTwuzUkaxbcCVuKTARw3723358 = -642605864;    double tDTwuzUkaxbcCVuKTARw23257465 = -358681458;    double tDTwuzUkaxbcCVuKTARw43688672 = -764236732;    double tDTwuzUkaxbcCVuKTARw39694796 = -521935812;    double tDTwuzUkaxbcCVuKTARw6569464 = -359155495;    double tDTwuzUkaxbcCVuKTARw74846137 = -950151037;    double tDTwuzUkaxbcCVuKTARw87449107 = -867085921;    double tDTwuzUkaxbcCVuKTARw93935716 = -306805825;    double tDTwuzUkaxbcCVuKTARw32662166 = -190112333;    double tDTwuzUkaxbcCVuKTARw79846114 = -771803137;    double tDTwuzUkaxbcCVuKTARw45168278 = -103780568;    double tDTwuzUkaxbcCVuKTARw5795364 = -112152404;    double tDTwuzUkaxbcCVuKTARw10258714 = -257729990;    double tDTwuzUkaxbcCVuKTARw3741452 = -890500415;    double tDTwuzUkaxbcCVuKTARw74904800 = -199373618;    double tDTwuzUkaxbcCVuKTARw87479003 = -472081043;    double tDTwuzUkaxbcCVuKTARw15932714 = -75110453;    double tDTwuzUkaxbcCVuKTARw74541108 = -389623218;    double tDTwuzUkaxbcCVuKTARw32750620 = -196513828;    double tDTwuzUkaxbcCVuKTARw8754224 = -24039468;    double tDTwuzUkaxbcCVuKTARw14550650 = -401553194;    double tDTwuzUkaxbcCVuKTARw55514077 = -448254702;    double tDTwuzUkaxbcCVuKTARw40137284 = -455843163;    double tDTwuzUkaxbcCVuKTARw81079444 = -742742252;    double tDTwuzUkaxbcCVuKTARw91343395 = -984023094;    double tDTwuzUkaxbcCVuKTARw90419855 = 32603816;     tDTwuzUkaxbcCVuKTARw19023620 = tDTwuzUkaxbcCVuKTARw86120427;     tDTwuzUkaxbcCVuKTARw86120427 = tDTwuzUkaxbcCVuKTARw1477774;     tDTwuzUkaxbcCVuKTARw1477774 = tDTwuzUkaxbcCVuKTARw36715546;     tDTwuzUkaxbcCVuKTARw36715546 = tDTwuzUkaxbcCVuKTARw28944108;     tDTwuzUkaxbcCVuKTARw28944108 = tDTwuzUkaxbcCVuKTARw37080281;     tDTwuzUkaxbcCVuKTARw37080281 = tDTwuzUkaxbcCVuKTARw45573613;     tDTwuzUkaxbcCVuKTARw45573613 = tDTwuzUkaxbcCVuKTARw86552663;     tDTwuzUkaxbcCVuKTARw86552663 = tDTwuzUkaxbcCVuKTARw55296752;     tDTwuzUkaxbcCVuKTARw55296752 = tDTwuzUkaxbcCVuKTARw73672461;     tDTwuzUkaxbcCVuKTARw73672461 = tDTwuzUkaxbcCVuKTARw85850436;     tDTwuzUkaxbcCVuKTARw85850436 = tDTwuzUkaxbcCVuKTARw36380737;     tDTwuzUkaxbcCVuKTARw36380737 = tDTwuzUkaxbcCVuKTARw91432245;     tDTwuzUkaxbcCVuKTARw91432245 = tDTwuzUkaxbcCVuKTARw64384274;     tDTwuzUkaxbcCVuKTARw64384274 = tDTwuzUkaxbcCVuKTARw92024659;     tDTwuzUkaxbcCVuKTARw92024659 = tDTwuzUkaxbcCVuKTARw42275885;     tDTwuzUkaxbcCVuKTARw42275885 = tDTwuzUkaxbcCVuKTARw36115650;     tDTwuzUkaxbcCVuKTARw36115650 = tDTwuzUkaxbcCVuKTARw87337930;     tDTwuzUkaxbcCVuKTARw87337930 = tDTwuzUkaxbcCVuKTARw58332836;     tDTwuzUkaxbcCVuKTARw58332836 = tDTwuzUkaxbcCVuKTARw80678889;     tDTwuzUkaxbcCVuKTARw80678889 = tDTwuzUkaxbcCVuKTARw80971384;     tDTwuzUkaxbcCVuKTARw80971384 = tDTwuzUkaxbcCVuKTARw55885703;     tDTwuzUkaxbcCVuKTARw55885703 = tDTwuzUkaxbcCVuKTARw89780058;     tDTwuzUkaxbcCVuKTARw89780058 = tDTwuzUkaxbcCVuKTARw54302498;     tDTwuzUkaxbcCVuKTARw54302498 = tDTwuzUkaxbcCVuKTARw33353307;     tDTwuzUkaxbcCVuKTARw33353307 = tDTwuzUkaxbcCVuKTARw74156396;     tDTwuzUkaxbcCVuKTARw74156396 = tDTwuzUkaxbcCVuKTARw90070704;     tDTwuzUkaxbcCVuKTARw90070704 = tDTwuzUkaxbcCVuKTARw24132158;     tDTwuzUkaxbcCVuKTARw24132158 = tDTwuzUkaxbcCVuKTARw68976028;     tDTwuzUkaxbcCVuKTARw68976028 = tDTwuzUkaxbcCVuKTARw24179165;     tDTwuzUkaxbcCVuKTARw24179165 = tDTwuzUkaxbcCVuKTARw14930595;     tDTwuzUkaxbcCVuKTARw14930595 = tDTwuzUkaxbcCVuKTARw28537258;     tDTwuzUkaxbcCVuKTARw28537258 = tDTwuzUkaxbcCVuKTARw71746641;     tDTwuzUkaxbcCVuKTARw71746641 = tDTwuzUkaxbcCVuKTARw76448532;     tDTwuzUkaxbcCVuKTARw76448532 = tDTwuzUkaxbcCVuKTARw50616920;     tDTwuzUkaxbcCVuKTARw50616920 = tDTwuzUkaxbcCVuKTARw12273083;     tDTwuzUkaxbcCVuKTARw12273083 = tDTwuzUkaxbcCVuKTARw21716383;     tDTwuzUkaxbcCVuKTARw21716383 = tDTwuzUkaxbcCVuKTARw36112860;     tDTwuzUkaxbcCVuKTARw36112860 = tDTwuzUkaxbcCVuKTARw63541935;     tDTwuzUkaxbcCVuKTARw63541935 = tDTwuzUkaxbcCVuKTARw61560227;     tDTwuzUkaxbcCVuKTARw61560227 = tDTwuzUkaxbcCVuKTARw76955531;     tDTwuzUkaxbcCVuKTARw76955531 = tDTwuzUkaxbcCVuKTARw57713919;     tDTwuzUkaxbcCVuKTARw57713919 = tDTwuzUkaxbcCVuKTARw12197032;     tDTwuzUkaxbcCVuKTARw12197032 = tDTwuzUkaxbcCVuKTARw50085262;     tDTwuzUkaxbcCVuKTARw50085262 = tDTwuzUkaxbcCVuKTARw47733034;     tDTwuzUkaxbcCVuKTARw47733034 = tDTwuzUkaxbcCVuKTARw58507169;     tDTwuzUkaxbcCVuKTARw58507169 = tDTwuzUkaxbcCVuKTARw86707289;     tDTwuzUkaxbcCVuKTARw86707289 = tDTwuzUkaxbcCVuKTARw96134987;     tDTwuzUkaxbcCVuKTARw96134987 = tDTwuzUkaxbcCVuKTARw91469992;     tDTwuzUkaxbcCVuKTARw91469992 = tDTwuzUkaxbcCVuKTARw89129914;     tDTwuzUkaxbcCVuKTARw89129914 = tDTwuzUkaxbcCVuKTARw79010887;     tDTwuzUkaxbcCVuKTARw79010887 = tDTwuzUkaxbcCVuKTARw9135232;     tDTwuzUkaxbcCVuKTARw9135232 = tDTwuzUkaxbcCVuKTARw18278544;     tDTwuzUkaxbcCVuKTARw18278544 = tDTwuzUkaxbcCVuKTARw68005190;     tDTwuzUkaxbcCVuKTARw68005190 = tDTwuzUkaxbcCVuKTARw1543733;     tDTwuzUkaxbcCVuKTARw1543733 = tDTwuzUkaxbcCVuKTARw63137916;     tDTwuzUkaxbcCVuKTARw63137916 = tDTwuzUkaxbcCVuKTARw96340369;     tDTwuzUkaxbcCVuKTARw96340369 = tDTwuzUkaxbcCVuKTARw47175275;     tDTwuzUkaxbcCVuKTARw47175275 = tDTwuzUkaxbcCVuKTARw3362240;     tDTwuzUkaxbcCVuKTARw3362240 = tDTwuzUkaxbcCVuKTARw54787711;     tDTwuzUkaxbcCVuKTARw54787711 = tDTwuzUkaxbcCVuKTARw47009577;     tDTwuzUkaxbcCVuKTARw47009577 = tDTwuzUkaxbcCVuKTARw21441455;     tDTwuzUkaxbcCVuKTARw21441455 = tDTwuzUkaxbcCVuKTARw17576636;     tDTwuzUkaxbcCVuKTARw17576636 = tDTwuzUkaxbcCVuKTARw31117587;     tDTwuzUkaxbcCVuKTARw31117587 = tDTwuzUkaxbcCVuKTARw58741867;     tDTwuzUkaxbcCVuKTARw58741867 = tDTwuzUkaxbcCVuKTARw57313179;     tDTwuzUkaxbcCVuKTARw57313179 = tDTwuzUkaxbcCVuKTARw64634095;     tDTwuzUkaxbcCVuKTARw64634095 = tDTwuzUkaxbcCVuKTARw14983713;     tDTwuzUkaxbcCVuKTARw14983713 = tDTwuzUkaxbcCVuKTARw13767354;     tDTwuzUkaxbcCVuKTARw13767354 = tDTwuzUkaxbcCVuKTARw79751576;     tDTwuzUkaxbcCVuKTARw79751576 = tDTwuzUkaxbcCVuKTARw20559503;     tDTwuzUkaxbcCVuKTARw20559503 = tDTwuzUkaxbcCVuKTARw2791;     tDTwuzUkaxbcCVuKTARw2791 = tDTwuzUkaxbcCVuKTARw23795996;     tDTwuzUkaxbcCVuKTARw23795996 = tDTwuzUkaxbcCVuKTARw96772609;     tDTwuzUkaxbcCVuKTARw96772609 = tDTwuzUkaxbcCVuKTARw3723358;     tDTwuzUkaxbcCVuKTARw3723358 = tDTwuzUkaxbcCVuKTARw23257465;     tDTwuzUkaxbcCVuKTARw23257465 = tDTwuzUkaxbcCVuKTARw43688672;     tDTwuzUkaxbcCVuKTARw43688672 = tDTwuzUkaxbcCVuKTARw39694796;     tDTwuzUkaxbcCVuKTARw39694796 = tDTwuzUkaxbcCVuKTARw6569464;     tDTwuzUkaxbcCVuKTARw6569464 = tDTwuzUkaxbcCVuKTARw74846137;     tDTwuzUkaxbcCVuKTARw74846137 = tDTwuzUkaxbcCVuKTARw87449107;     tDTwuzUkaxbcCVuKTARw87449107 = tDTwuzUkaxbcCVuKTARw93935716;     tDTwuzUkaxbcCVuKTARw93935716 = tDTwuzUkaxbcCVuKTARw32662166;     tDTwuzUkaxbcCVuKTARw32662166 = tDTwuzUkaxbcCVuKTARw79846114;     tDTwuzUkaxbcCVuKTARw79846114 = tDTwuzUkaxbcCVuKTARw45168278;     tDTwuzUkaxbcCVuKTARw45168278 = tDTwuzUkaxbcCVuKTARw5795364;     tDTwuzUkaxbcCVuKTARw5795364 = tDTwuzUkaxbcCVuKTARw10258714;     tDTwuzUkaxbcCVuKTARw10258714 = tDTwuzUkaxbcCVuKTARw3741452;     tDTwuzUkaxbcCVuKTARw3741452 = tDTwuzUkaxbcCVuKTARw74904800;     tDTwuzUkaxbcCVuKTARw74904800 = tDTwuzUkaxbcCVuKTARw87479003;     tDTwuzUkaxbcCVuKTARw87479003 = tDTwuzUkaxbcCVuKTARw15932714;     tDTwuzUkaxbcCVuKTARw15932714 = tDTwuzUkaxbcCVuKTARw74541108;     tDTwuzUkaxbcCVuKTARw74541108 = tDTwuzUkaxbcCVuKTARw32750620;     tDTwuzUkaxbcCVuKTARw32750620 = tDTwuzUkaxbcCVuKTARw8754224;     tDTwuzUkaxbcCVuKTARw8754224 = tDTwuzUkaxbcCVuKTARw14550650;     tDTwuzUkaxbcCVuKTARw14550650 = tDTwuzUkaxbcCVuKTARw55514077;     tDTwuzUkaxbcCVuKTARw55514077 = tDTwuzUkaxbcCVuKTARw40137284;     tDTwuzUkaxbcCVuKTARw40137284 = tDTwuzUkaxbcCVuKTARw81079444;     tDTwuzUkaxbcCVuKTARw81079444 = tDTwuzUkaxbcCVuKTARw91343395;     tDTwuzUkaxbcCVuKTARw91343395 = tDTwuzUkaxbcCVuKTARw90419855;     tDTwuzUkaxbcCVuKTARw90419855 = tDTwuzUkaxbcCVuKTARw19023620;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void AQlDJDrENFKOeQesmhpm54710396() {     double otiEYvSCMWthETeKAFMu65541393 = -336803766;    double otiEYvSCMWthETeKAFMu97274015 = -131782620;    double otiEYvSCMWthETeKAFMu31175175 = -699212332;    double otiEYvSCMWthETeKAFMu60454499 = -649505596;    double otiEYvSCMWthETeKAFMu90966421 = -45973;    double otiEYvSCMWthETeKAFMu48223079 = -386863906;    double otiEYvSCMWthETeKAFMu92977764 = -172784032;    double otiEYvSCMWthETeKAFMu20094296 = -548185916;    double otiEYvSCMWthETeKAFMu47495886 = 64916953;    double otiEYvSCMWthETeKAFMu30197363 = -894971119;    double otiEYvSCMWthETeKAFMu30826098 = -241689489;    double otiEYvSCMWthETeKAFMu40678416 = -594022822;    double otiEYvSCMWthETeKAFMu22813258 = -547877880;    double otiEYvSCMWthETeKAFMu66076222 = -386979623;    double otiEYvSCMWthETeKAFMu13045335 = -138567432;    double otiEYvSCMWthETeKAFMu74309298 = -200212011;    double otiEYvSCMWthETeKAFMu21916929 = -621563933;    double otiEYvSCMWthETeKAFMu81223019 = -629686620;    double otiEYvSCMWthETeKAFMu99934609 = -630198203;    double otiEYvSCMWthETeKAFMu17348827 = -418094954;    double otiEYvSCMWthETeKAFMu28898171 = -360100705;    double otiEYvSCMWthETeKAFMu63065261 = -50704702;    double otiEYvSCMWthETeKAFMu51618223 = -220833284;    double otiEYvSCMWthETeKAFMu95389687 = -209255162;    double otiEYvSCMWthETeKAFMu65296296 = -878397872;    double otiEYvSCMWthETeKAFMu62750203 = -191938478;    double otiEYvSCMWthETeKAFMu60301203 = -961808190;    double otiEYvSCMWthETeKAFMu89829876 = -583571368;    double otiEYvSCMWthETeKAFMu91663164 = -301383699;    double otiEYvSCMWthETeKAFMu42366270 = -77901220;    double otiEYvSCMWthETeKAFMu98965165 = -647326238;    double otiEYvSCMWthETeKAFMu75744547 = -899438200;    double otiEYvSCMWthETeKAFMu75101763 = 68226793;    double otiEYvSCMWthETeKAFMu88896072 = -460353017;    double otiEYvSCMWthETeKAFMu48333230 = -129476388;    double otiEYvSCMWthETeKAFMu23823195 = -219271558;    double otiEYvSCMWthETeKAFMu68317324 = -33435019;    double otiEYvSCMWthETeKAFMu6801575 = -594303713;    double otiEYvSCMWthETeKAFMu35791044 = -957524963;    double otiEYvSCMWthETeKAFMu62172957 = -826512121;    double otiEYvSCMWthETeKAFMu18334776 = -965566845;    double otiEYvSCMWthETeKAFMu12634026 = -385816331;    double otiEYvSCMWthETeKAFMu29526447 = -92869072;    double otiEYvSCMWthETeKAFMu42869447 = -411106564;    double otiEYvSCMWthETeKAFMu706211 = -153882442;    double otiEYvSCMWthETeKAFMu25867220 = -546135759;    double otiEYvSCMWthETeKAFMu12846038 = 5728077;    double otiEYvSCMWthETeKAFMu45087325 = -384943286;    double otiEYvSCMWthETeKAFMu21085269 = -678109477;    double otiEYvSCMWthETeKAFMu96896886 = -983026106;    double otiEYvSCMWthETeKAFMu70855000 = -111380291;    double otiEYvSCMWthETeKAFMu20633427 = -444081546;    double otiEYvSCMWthETeKAFMu78835661 = -1436649;    double otiEYvSCMWthETeKAFMu95824682 = -353895302;    double otiEYvSCMWthETeKAFMu25812226 = -38028033;    double otiEYvSCMWthETeKAFMu2476133 = -186099065;    double otiEYvSCMWthETeKAFMu45655793 = -910949336;    double otiEYvSCMWthETeKAFMu35785488 = -389957170;    double otiEYvSCMWthETeKAFMu95158202 = -771107725;    double otiEYvSCMWthETeKAFMu28216218 = -808107496;    double otiEYvSCMWthETeKAFMu87921876 = -425055716;    double otiEYvSCMWthETeKAFMu3147888 = -589212664;    double otiEYvSCMWthETeKAFMu28431132 = -146802217;    double otiEYvSCMWthETeKAFMu5129616 = -857181827;    double otiEYvSCMWthETeKAFMu31232198 = -147644882;    double otiEYvSCMWthETeKAFMu55081550 = -342251289;    double otiEYvSCMWthETeKAFMu65576653 = -562249616;    double otiEYvSCMWthETeKAFMu33917185 = 12475136;    double otiEYvSCMWthETeKAFMu17742992 = -157503236;    double otiEYvSCMWthETeKAFMu89222140 = -919295874;    double otiEYvSCMWthETeKAFMu5991975 = -66776993;    double otiEYvSCMWthETeKAFMu15115355 = 72739779;    double otiEYvSCMWthETeKAFMu45431975 = -672161657;    double otiEYvSCMWthETeKAFMu37761653 = -803686082;    double otiEYvSCMWthETeKAFMu99014050 = -452528109;    double otiEYvSCMWthETeKAFMu16264146 = -974284375;    double otiEYvSCMWthETeKAFMu33538814 = -957835631;    double otiEYvSCMWthETeKAFMu8748777 = -809726720;    double otiEYvSCMWthETeKAFMu94683476 = 44627279;    double otiEYvSCMWthETeKAFMu39429077 = -232262114;    double otiEYvSCMWthETeKAFMu49904166 = -97666556;    double otiEYvSCMWthETeKAFMu15213878 = -476864905;    double otiEYvSCMWthETeKAFMu68744608 = -905461891;    double otiEYvSCMWthETeKAFMu94766278 = -318357594;    double otiEYvSCMWthETeKAFMu71511270 = -966520929;    double otiEYvSCMWthETeKAFMu78331738 = -103244692;    double otiEYvSCMWthETeKAFMu96908886 = -798001551;    double otiEYvSCMWthETeKAFMu79277080 = -577877906;    double otiEYvSCMWthETeKAFMu63083847 = -322324984;    double otiEYvSCMWthETeKAFMu45857097 = -943377324;    double otiEYvSCMWthETeKAFMu78167402 = -308322222;    double otiEYvSCMWthETeKAFMu32531836 = -643477849;    double otiEYvSCMWthETeKAFMu11643372 = -823195989;    double otiEYvSCMWthETeKAFMu7574827 = -49417468;    double otiEYvSCMWthETeKAFMu74251080 = -301456406;    double otiEYvSCMWthETeKAFMu15186889 = -276354181;    double otiEYvSCMWthETeKAFMu84202894 = -139014115;    double otiEYvSCMWthETeKAFMu24396832 = -235687245;    double otiEYvSCMWthETeKAFMu11637250 = -163461683;    double otiEYvSCMWthETeKAFMu45624660 = -336803766;     otiEYvSCMWthETeKAFMu65541393 = otiEYvSCMWthETeKAFMu97274015;     otiEYvSCMWthETeKAFMu97274015 = otiEYvSCMWthETeKAFMu31175175;     otiEYvSCMWthETeKAFMu31175175 = otiEYvSCMWthETeKAFMu60454499;     otiEYvSCMWthETeKAFMu60454499 = otiEYvSCMWthETeKAFMu90966421;     otiEYvSCMWthETeKAFMu90966421 = otiEYvSCMWthETeKAFMu48223079;     otiEYvSCMWthETeKAFMu48223079 = otiEYvSCMWthETeKAFMu92977764;     otiEYvSCMWthETeKAFMu92977764 = otiEYvSCMWthETeKAFMu20094296;     otiEYvSCMWthETeKAFMu20094296 = otiEYvSCMWthETeKAFMu47495886;     otiEYvSCMWthETeKAFMu47495886 = otiEYvSCMWthETeKAFMu30197363;     otiEYvSCMWthETeKAFMu30197363 = otiEYvSCMWthETeKAFMu30826098;     otiEYvSCMWthETeKAFMu30826098 = otiEYvSCMWthETeKAFMu40678416;     otiEYvSCMWthETeKAFMu40678416 = otiEYvSCMWthETeKAFMu22813258;     otiEYvSCMWthETeKAFMu22813258 = otiEYvSCMWthETeKAFMu66076222;     otiEYvSCMWthETeKAFMu66076222 = otiEYvSCMWthETeKAFMu13045335;     otiEYvSCMWthETeKAFMu13045335 = otiEYvSCMWthETeKAFMu74309298;     otiEYvSCMWthETeKAFMu74309298 = otiEYvSCMWthETeKAFMu21916929;     otiEYvSCMWthETeKAFMu21916929 = otiEYvSCMWthETeKAFMu81223019;     otiEYvSCMWthETeKAFMu81223019 = otiEYvSCMWthETeKAFMu99934609;     otiEYvSCMWthETeKAFMu99934609 = otiEYvSCMWthETeKAFMu17348827;     otiEYvSCMWthETeKAFMu17348827 = otiEYvSCMWthETeKAFMu28898171;     otiEYvSCMWthETeKAFMu28898171 = otiEYvSCMWthETeKAFMu63065261;     otiEYvSCMWthETeKAFMu63065261 = otiEYvSCMWthETeKAFMu51618223;     otiEYvSCMWthETeKAFMu51618223 = otiEYvSCMWthETeKAFMu95389687;     otiEYvSCMWthETeKAFMu95389687 = otiEYvSCMWthETeKAFMu65296296;     otiEYvSCMWthETeKAFMu65296296 = otiEYvSCMWthETeKAFMu62750203;     otiEYvSCMWthETeKAFMu62750203 = otiEYvSCMWthETeKAFMu60301203;     otiEYvSCMWthETeKAFMu60301203 = otiEYvSCMWthETeKAFMu89829876;     otiEYvSCMWthETeKAFMu89829876 = otiEYvSCMWthETeKAFMu91663164;     otiEYvSCMWthETeKAFMu91663164 = otiEYvSCMWthETeKAFMu42366270;     otiEYvSCMWthETeKAFMu42366270 = otiEYvSCMWthETeKAFMu98965165;     otiEYvSCMWthETeKAFMu98965165 = otiEYvSCMWthETeKAFMu75744547;     otiEYvSCMWthETeKAFMu75744547 = otiEYvSCMWthETeKAFMu75101763;     otiEYvSCMWthETeKAFMu75101763 = otiEYvSCMWthETeKAFMu88896072;     otiEYvSCMWthETeKAFMu88896072 = otiEYvSCMWthETeKAFMu48333230;     otiEYvSCMWthETeKAFMu48333230 = otiEYvSCMWthETeKAFMu23823195;     otiEYvSCMWthETeKAFMu23823195 = otiEYvSCMWthETeKAFMu68317324;     otiEYvSCMWthETeKAFMu68317324 = otiEYvSCMWthETeKAFMu6801575;     otiEYvSCMWthETeKAFMu6801575 = otiEYvSCMWthETeKAFMu35791044;     otiEYvSCMWthETeKAFMu35791044 = otiEYvSCMWthETeKAFMu62172957;     otiEYvSCMWthETeKAFMu62172957 = otiEYvSCMWthETeKAFMu18334776;     otiEYvSCMWthETeKAFMu18334776 = otiEYvSCMWthETeKAFMu12634026;     otiEYvSCMWthETeKAFMu12634026 = otiEYvSCMWthETeKAFMu29526447;     otiEYvSCMWthETeKAFMu29526447 = otiEYvSCMWthETeKAFMu42869447;     otiEYvSCMWthETeKAFMu42869447 = otiEYvSCMWthETeKAFMu706211;     otiEYvSCMWthETeKAFMu706211 = otiEYvSCMWthETeKAFMu25867220;     otiEYvSCMWthETeKAFMu25867220 = otiEYvSCMWthETeKAFMu12846038;     otiEYvSCMWthETeKAFMu12846038 = otiEYvSCMWthETeKAFMu45087325;     otiEYvSCMWthETeKAFMu45087325 = otiEYvSCMWthETeKAFMu21085269;     otiEYvSCMWthETeKAFMu21085269 = otiEYvSCMWthETeKAFMu96896886;     otiEYvSCMWthETeKAFMu96896886 = otiEYvSCMWthETeKAFMu70855000;     otiEYvSCMWthETeKAFMu70855000 = otiEYvSCMWthETeKAFMu20633427;     otiEYvSCMWthETeKAFMu20633427 = otiEYvSCMWthETeKAFMu78835661;     otiEYvSCMWthETeKAFMu78835661 = otiEYvSCMWthETeKAFMu95824682;     otiEYvSCMWthETeKAFMu95824682 = otiEYvSCMWthETeKAFMu25812226;     otiEYvSCMWthETeKAFMu25812226 = otiEYvSCMWthETeKAFMu2476133;     otiEYvSCMWthETeKAFMu2476133 = otiEYvSCMWthETeKAFMu45655793;     otiEYvSCMWthETeKAFMu45655793 = otiEYvSCMWthETeKAFMu35785488;     otiEYvSCMWthETeKAFMu35785488 = otiEYvSCMWthETeKAFMu95158202;     otiEYvSCMWthETeKAFMu95158202 = otiEYvSCMWthETeKAFMu28216218;     otiEYvSCMWthETeKAFMu28216218 = otiEYvSCMWthETeKAFMu87921876;     otiEYvSCMWthETeKAFMu87921876 = otiEYvSCMWthETeKAFMu3147888;     otiEYvSCMWthETeKAFMu3147888 = otiEYvSCMWthETeKAFMu28431132;     otiEYvSCMWthETeKAFMu28431132 = otiEYvSCMWthETeKAFMu5129616;     otiEYvSCMWthETeKAFMu5129616 = otiEYvSCMWthETeKAFMu31232198;     otiEYvSCMWthETeKAFMu31232198 = otiEYvSCMWthETeKAFMu55081550;     otiEYvSCMWthETeKAFMu55081550 = otiEYvSCMWthETeKAFMu65576653;     otiEYvSCMWthETeKAFMu65576653 = otiEYvSCMWthETeKAFMu33917185;     otiEYvSCMWthETeKAFMu33917185 = otiEYvSCMWthETeKAFMu17742992;     otiEYvSCMWthETeKAFMu17742992 = otiEYvSCMWthETeKAFMu89222140;     otiEYvSCMWthETeKAFMu89222140 = otiEYvSCMWthETeKAFMu5991975;     otiEYvSCMWthETeKAFMu5991975 = otiEYvSCMWthETeKAFMu15115355;     otiEYvSCMWthETeKAFMu15115355 = otiEYvSCMWthETeKAFMu45431975;     otiEYvSCMWthETeKAFMu45431975 = otiEYvSCMWthETeKAFMu37761653;     otiEYvSCMWthETeKAFMu37761653 = otiEYvSCMWthETeKAFMu99014050;     otiEYvSCMWthETeKAFMu99014050 = otiEYvSCMWthETeKAFMu16264146;     otiEYvSCMWthETeKAFMu16264146 = otiEYvSCMWthETeKAFMu33538814;     otiEYvSCMWthETeKAFMu33538814 = otiEYvSCMWthETeKAFMu8748777;     otiEYvSCMWthETeKAFMu8748777 = otiEYvSCMWthETeKAFMu94683476;     otiEYvSCMWthETeKAFMu94683476 = otiEYvSCMWthETeKAFMu39429077;     otiEYvSCMWthETeKAFMu39429077 = otiEYvSCMWthETeKAFMu49904166;     otiEYvSCMWthETeKAFMu49904166 = otiEYvSCMWthETeKAFMu15213878;     otiEYvSCMWthETeKAFMu15213878 = otiEYvSCMWthETeKAFMu68744608;     otiEYvSCMWthETeKAFMu68744608 = otiEYvSCMWthETeKAFMu94766278;     otiEYvSCMWthETeKAFMu94766278 = otiEYvSCMWthETeKAFMu71511270;     otiEYvSCMWthETeKAFMu71511270 = otiEYvSCMWthETeKAFMu78331738;     otiEYvSCMWthETeKAFMu78331738 = otiEYvSCMWthETeKAFMu96908886;     otiEYvSCMWthETeKAFMu96908886 = otiEYvSCMWthETeKAFMu79277080;     otiEYvSCMWthETeKAFMu79277080 = otiEYvSCMWthETeKAFMu63083847;     otiEYvSCMWthETeKAFMu63083847 = otiEYvSCMWthETeKAFMu45857097;     otiEYvSCMWthETeKAFMu45857097 = otiEYvSCMWthETeKAFMu78167402;     otiEYvSCMWthETeKAFMu78167402 = otiEYvSCMWthETeKAFMu32531836;     otiEYvSCMWthETeKAFMu32531836 = otiEYvSCMWthETeKAFMu11643372;     otiEYvSCMWthETeKAFMu11643372 = otiEYvSCMWthETeKAFMu7574827;     otiEYvSCMWthETeKAFMu7574827 = otiEYvSCMWthETeKAFMu74251080;     otiEYvSCMWthETeKAFMu74251080 = otiEYvSCMWthETeKAFMu15186889;     otiEYvSCMWthETeKAFMu15186889 = otiEYvSCMWthETeKAFMu84202894;     otiEYvSCMWthETeKAFMu84202894 = otiEYvSCMWthETeKAFMu24396832;     otiEYvSCMWthETeKAFMu24396832 = otiEYvSCMWthETeKAFMu11637250;     otiEYvSCMWthETeKAFMu11637250 = otiEYvSCMWthETeKAFMu45624660;     otiEYvSCMWthETeKAFMu45624660 = otiEYvSCMWthETeKAFMu65541393;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ctoKksLqtMjDMimcXBhM82304652() {     double APQldhUeJEBACgjtwqTH91126670 = -861836867;    double APQldhUeJEBACgjtwqTH93671159 = -387260396;    double APQldhUeJEBACgjtwqTH64054350 = -501673868;    double APQldhUeJEBACgjtwqTH6176429 = -656828489;    double APQldhUeJEBACgjtwqTH60308426 = -46968261;    double APQldhUeJEBACgjtwqTH65232355 = -541844909;    double APQldhUeJEBACgjtwqTH98298608 = -755055239;    double APQldhUeJEBACgjtwqTH35104420 = -928867795;    double APQldhUeJEBACgjtwqTH72825178 = -431307092;    double APQldhUeJEBACgjtwqTH49355745 = -129532643;    double APQldhUeJEBACgjtwqTH45159652 = -552117375;    double APQldhUeJEBACgjtwqTH75996072 = 62268271;    double APQldhUeJEBACgjtwqTH99394035 = -312126246;    double APQldhUeJEBACgjtwqTH57635939 = -361503917;    double APQldhUeJEBACgjtwqTH69314223 = -27113390;    double APQldhUeJEBACgjtwqTH82030341 = -636403488;    double APQldhUeJEBACgjtwqTH21715046 = -309883727;    double APQldhUeJEBACgjtwqTH2338924 = -45963980;    double APQldhUeJEBACgjtwqTH61399331 = -313172060;    double APQldhUeJEBACgjtwqTH78510293 = -392534329;    double APQldhUeJEBACgjtwqTH29907436 = -821800487;    double APQldhUeJEBACgjtwqTH64568367 = -470437631;    double APQldhUeJEBACgjtwqTH55382021 = -264925389;    double APQldhUeJEBACgjtwqTH97103729 = -68104142;    double APQldhUeJEBACgjtwqTH20199436 = -445894014;    double APQldhUeJEBACgjtwqTH92239340 = -856389020;    double APQldhUeJEBACgjtwqTH42689112 = -527621325;    double APQldhUeJEBACgjtwqTH53728677 = -813233601;    double APQldhUeJEBACgjtwqTH99540377 = -537677550;    double APQldhUeJEBACgjtwqTH95688761 = 13189218;    double APQldhUeJEBACgjtwqTH21011800 = -425973269;    double APQldhUeJEBACgjtwqTH12934481 = -73234034;    double APQldhUeJEBACgjtwqTH96288814 = -379594344;    double APQldhUeJEBACgjtwqTH42483144 = -516497988;    double APQldhUeJEBACgjtwqTH49738218 = -830912299;    double APQldhUeJEBACgjtwqTH28615045 = -736866274;    double APQldhUeJEBACgjtwqTH17962710 = -513946963;    double APQldhUeJEBACgjtwqTH82374261 = -443018538;    double APQldhUeJEBACgjtwqTH76714622 = -315863285;    double APQldhUeJEBACgjtwqTH70956630 = -196482440;    double APQldhUeJEBACgjtwqTH65486057 = -440627035;    double APQldhUeJEBACgjtwqTH56643615 = -894854835;    double APQldhUeJEBACgjtwqTH78726666 = -121475453;    double APQldhUeJEBACgjtwqTH13662726 = -879522163;    double APQldhUeJEBACgjtwqTH92687086 = -92903559;    double APQldhUeJEBACgjtwqTH92267541 = 31316703;    double APQldhUeJEBACgjtwqTH12614492 = -26740784;    double APQldhUeJEBACgjtwqTH53200096 = -578126133;    double APQldhUeJEBACgjtwqTH71896588 = -777785704;    double APQldhUeJEBACgjtwqTH50706881 = -831049259;    double APQldhUeJEBACgjtwqTH80991247 = -360038893;    double APQldhUeJEBACgjtwqTH47892006 = -622160726;    double APQldhUeJEBACgjtwqTH96909264 = -569335198;    double APQldhUeJEBACgjtwqTH64723591 = -668959083;    double APQldhUeJEBACgjtwqTH52397217 = -513118451;    double APQldhUeJEBACgjtwqTH26558303 = -291399236;    double APQldhUeJEBACgjtwqTH38289138 = -22335008;    double APQldhUeJEBACgjtwqTH66950621 = -333569727;    double APQldhUeJEBACgjtwqTH85976992 = -110934476;    double APQldhUeJEBACgjtwqTH68069085 = -190579241;    double APQldhUeJEBACgjtwqTH22543244 = 85776415;    double APQldhUeJEBACgjtwqTH44569932 = -941821639;    double APQldhUeJEBACgjtwqTH35564043 = -291190246;    double APQldhUeJEBACgjtwqTH77136417 = -344496310;    double APQldhUeJEBACgjtwqTH28343946 = -703559374;    double APQldhUeJEBACgjtwqTH32225172 = -378883342;    double APQldhUeJEBACgjtwqTH79707257 = -558137385;    double APQldhUeJEBACgjtwqTH56910891 = -795628258;    double APQldhUeJEBACgjtwqTH7897721 = -530591618;    double APQldhUeJEBACgjtwqTH40699178 = -290247116;    double APQldhUeJEBACgjtwqTH64067632 = -22456526;    double APQldhUeJEBACgjtwqTH39340784 = -866865190;    double APQldhUeJEBACgjtwqTH25624301 = -730100696;    double APQldhUeJEBACgjtwqTH90442700 = -16689621;    double APQldhUeJEBACgjtwqTH13024237 = -951907295;    double APQldhUeJEBACgjtwqTH73263820 = -926945652;    double APQldhUeJEBACgjtwqTH85841700 = -248962179;    double APQldhUeJEBACgjtwqTH41719295 = -385403226;    double APQldhUeJEBACgjtwqTH4416644 = -975200583;    double APQldhUeJEBACgjtwqTH27931894 = -377210718;    double APQldhUeJEBACgjtwqTH79624848 = -729648237;    double APQldhUeJEBACgjtwqTH89489015 = -949495193;    double APQldhUeJEBACgjtwqTH81832088 = 64552103;    double APQldhUeJEBACgjtwqTH48833496 = -706628291;    double APQldhUeJEBACgjtwqTH14697514 = -626771889;    double APQldhUeJEBACgjtwqTH73119793 = -803812544;    double APQldhUeJEBACgjtwqTH16025216 = -503898837;    double APQldhUeJEBACgjtwqTH31565224 = -710635261;    double APQldhUeJEBACgjtwqTH90085926 = 96620462;    double APQldhUeJEBACgjtwqTH23179915 = -439513064;    double APQldhUeJEBACgjtwqTH90325906 = -614531267;    double APQldhUeJEBACgjtwqTH51012088 = -80377237;    double APQldhUeJEBACgjtwqTH96397268 = -232084063;    double APQldhUeJEBACgjtwqTH8645537 = -25284044;    double APQldhUeJEBACgjtwqTH48413387 = -182258856;    double APQldhUeJEBACgjtwqTH20916126 = -498805397;    double APQldhUeJEBACgjtwqTH21079573 = -503664590;    double APQldhUeJEBACgjtwqTH1590250 = -776979143;    double APQldhUeJEBACgjtwqTH85318780 = -75962789;    double APQldhUeJEBACgjtwqTH60461915 = -861836867;     APQldhUeJEBACgjtwqTH91126670 = APQldhUeJEBACgjtwqTH93671159;     APQldhUeJEBACgjtwqTH93671159 = APQldhUeJEBACgjtwqTH64054350;     APQldhUeJEBACgjtwqTH64054350 = APQldhUeJEBACgjtwqTH6176429;     APQldhUeJEBACgjtwqTH6176429 = APQldhUeJEBACgjtwqTH60308426;     APQldhUeJEBACgjtwqTH60308426 = APQldhUeJEBACgjtwqTH65232355;     APQldhUeJEBACgjtwqTH65232355 = APQldhUeJEBACgjtwqTH98298608;     APQldhUeJEBACgjtwqTH98298608 = APQldhUeJEBACgjtwqTH35104420;     APQldhUeJEBACgjtwqTH35104420 = APQldhUeJEBACgjtwqTH72825178;     APQldhUeJEBACgjtwqTH72825178 = APQldhUeJEBACgjtwqTH49355745;     APQldhUeJEBACgjtwqTH49355745 = APQldhUeJEBACgjtwqTH45159652;     APQldhUeJEBACgjtwqTH45159652 = APQldhUeJEBACgjtwqTH75996072;     APQldhUeJEBACgjtwqTH75996072 = APQldhUeJEBACgjtwqTH99394035;     APQldhUeJEBACgjtwqTH99394035 = APQldhUeJEBACgjtwqTH57635939;     APQldhUeJEBACgjtwqTH57635939 = APQldhUeJEBACgjtwqTH69314223;     APQldhUeJEBACgjtwqTH69314223 = APQldhUeJEBACgjtwqTH82030341;     APQldhUeJEBACgjtwqTH82030341 = APQldhUeJEBACgjtwqTH21715046;     APQldhUeJEBACgjtwqTH21715046 = APQldhUeJEBACgjtwqTH2338924;     APQldhUeJEBACgjtwqTH2338924 = APQldhUeJEBACgjtwqTH61399331;     APQldhUeJEBACgjtwqTH61399331 = APQldhUeJEBACgjtwqTH78510293;     APQldhUeJEBACgjtwqTH78510293 = APQldhUeJEBACgjtwqTH29907436;     APQldhUeJEBACgjtwqTH29907436 = APQldhUeJEBACgjtwqTH64568367;     APQldhUeJEBACgjtwqTH64568367 = APQldhUeJEBACgjtwqTH55382021;     APQldhUeJEBACgjtwqTH55382021 = APQldhUeJEBACgjtwqTH97103729;     APQldhUeJEBACgjtwqTH97103729 = APQldhUeJEBACgjtwqTH20199436;     APQldhUeJEBACgjtwqTH20199436 = APQldhUeJEBACgjtwqTH92239340;     APQldhUeJEBACgjtwqTH92239340 = APQldhUeJEBACgjtwqTH42689112;     APQldhUeJEBACgjtwqTH42689112 = APQldhUeJEBACgjtwqTH53728677;     APQldhUeJEBACgjtwqTH53728677 = APQldhUeJEBACgjtwqTH99540377;     APQldhUeJEBACgjtwqTH99540377 = APQldhUeJEBACgjtwqTH95688761;     APQldhUeJEBACgjtwqTH95688761 = APQldhUeJEBACgjtwqTH21011800;     APQldhUeJEBACgjtwqTH21011800 = APQldhUeJEBACgjtwqTH12934481;     APQldhUeJEBACgjtwqTH12934481 = APQldhUeJEBACgjtwqTH96288814;     APQldhUeJEBACgjtwqTH96288814 = APQldhUeJEBACgjtwqTH42483144;     APQldhUeJEBACgjtwqTH42483144 = APQldhUeJEBACgjtwqTH49738218;     APQldhUeJEBACgjtwqTH49738218 = APQldhUeJEBACgjtwqTH28615045;     APQldhUeJEBACgjtwqTH28615045 = APQldhUeJEBACgjtwqTH17962710;     APQldhUeJEBACgjtwqTH17962710 = APQldhUeJEBACgjtwqTH82374261;     APQldhUeJEBACgjtwqTH82374261 = APQldhUeJEBACgjtwqTH76714622;     APQldhUeJEBACgjtwqTH76714622 = APQldhUeJEBACgjtwqTH70956630;     APQldhUeJEBACgjtwqTH70956630 = APQldhUeJEBACgjtwqTH65486057;     APQldhUeJEBACgjtwqTH65486057 = APQldhUeJEBACgjtwqTH56643615;     APQldhUeJEBACgjtwqTH56643615 = APQldhUeJEBACgjtwqTH78726666;     APQldhUeJEBACgjtwqTH78726666 = APQldhUeJEBACgjtwqTH13662726;     APQldhUeJEBACgjtwqTH13662726 = APQldhUeJEBACgjtwqTH92687086;     APQldhUeJEBACgjtwqTH92687086 = APQldhUeJEBACgjtwqTH92267541;     APQldhUeJEBACgjtwqTH92267541 = APQldhUeJEBACgjtwqTH12614492;     APQldhUeJEBACgjtwqTH12614492 = APQldhUeJEBACgjtwqTH53200096;     APQldhUeJEBACgjtwqTH53200096 = APQldhUeJEBACgjtwqTH71896588;     APQldhUeJEBACgjtwqTH71896588 = APQldhUeJEBACgjtwqTH50706881;     APQldhUeJEBACgjtwqTH50706881 = APQldhUeJEBACgjtwqTH80991247;     APQldhUeJEBACgjtwqTH80991247 = APQldhUeJEBACgjtwqTH47892006;     APQldhUeJEBACgjtwqTH47892006 = APQldhUeJEBACgjtwqTH96909264;     APQldhUeJEBACgjtwqTH96909264 = APQldhUeJEBACgjtwqTH64723591;     APQldhUeJEBACgjtwqTH64723591 = APQldhUeJEBACgjtwqTH52397217;     APQldhUeJEBACgjtwqTH52397217 = APQldhUeJEBACgjtwqTH26558303;     APQldhUeJEBACgjtwqTH26558303 = APQldhUeJEBACgjtwqTH38289138;     APQldhUeJEBACgjtwqTH38289138 = APQldhUeJEBACgjtwqTH66950621;     APQldhUeJEBACgjtwqTH66950621 = APQldhUeJEBACgjtwqTH85976992;     APQldhUeJEBACgjtwqTH85976992 = APQldhUeJEBACgjtwqTH68069085;     APQldhUeJEBACgjtwqTH68069085 = APQldhUeJEBACgjtwqTH22543244;     APQldhUeJEBACgjtwqTH22543244 = APQldhUeJEBACgjtwqTH44569932;     APQldhUeJEBACgjtwqTH44569932 = APQldhUeJEBACgjtwqTH35564043;     APQldhUeJEBACgjtwqTH35564043 = APQldhUeJEBACgjtwqTH77136417;     APQldhUeJEBACgjtwqTH77136417 = APQldhUeJEBACgjtwqTH28343946;     APQldhUeJEBACgjtwqTH28343946 = APQldhUeJEBACgjtwqTH32225172;     APQldhUeJEBACgjtwqTH32225172 = APQldhUeJEBACgjtwqTH79707257;     APQldhUeJEBACgjtwqTH79707257 = APQldhUeJEBACgjtwqTH56910891;     APQldhUeJEBACgjtwqTH56910891 = APQldhUeJEBACgjtwqTH7897721;     APQldhUeJEBACgjtwqTH7897721 = APQldhUeJEBACgjtwqTH40699178;     APQldhUeJEBACgjtwqTH40699178 = APQldhUeJEBACgjtwqTH64067632;     APQldhUeJEBACgjtwqTH64067632 = APQldhUeJEBACgjtwqTH39340784;     APQldhUeJEBACgjtwqTH39340784 = APQldhUeJEBACgjtwqTH25624301;     APQldhUeJEBACgjtwqTH25624301 = APQldhUeJEBACgjtwqTH90442700;     APQldhUeJEBACgjtwqTH90442700 = APQldhUeJEBACgjtwqTH13024237;     APQldhUeJEBACgjtwqTH13024237 = APQldhUeJEBACgjtwqTH73263820;     APQldhUeJEBACgjtwqTH73263820 = APQldhUeJEBACgjtwqTH85841700;     APQldhUeJEBACgjtwqTH85841700 = APQldhUeJEBACgjtwqTH41719295;     APQldhUeJEBACgjtwqTH41719295 = APQldhUeJEBACgjtwqTH4416644;     APQldhUeJEBACgjtwqTH4416644 = APQldhUeJEBACgjtwqTH27931894;     APQldhUeJEBACgjtwqTH27931894 = APQldhUeJEBACgjtwqTH79624848;     APQldhUeJEBACgjtwqTH79624848 = APQldhUeJEBACgjtwqTH89489015;     APQldhUeJEBACgjtwqTH89489015 = APQldhUeJEBACgjtwqTH81832088;     APQldhUeJEBACgjtwqTH81832088 = APQldhUeJEBACgjtwqTH48833496;     APQldhUeJEBACgjtwqTH48833496 = APQldhUeJEBACgjtwqTH14697514;     APQldhUeJEBACgjtwqTH14697514 = APQldhUeJEBACgjtwqTH73119793;     APQldhUeJEBACgjtwqTH73119793 = APQldhUeJEBACgjtwqTH16025216;     APQldhUeJEBACgjtwqTH16025216 = APQldhUeJEBACgjtwqTH31565224;     APQldhUeJEBACgjtwqTH31565224 = APQldhUeJEBACgjtwqTH90085926;     APQldhUeJEBACgjtwqTH90085926 = APQldhUeJEBACgjtwqTH23179915;     APQldhUeJEBACgjtwqTH23179915 = APQldhUeJEBACgjtwqTH90325906;     APQldhUeJEBACgjtwqTH90325906 = APQldhUeJEBACgjtwqTH51012088;     APQldhUeJEBACgjtwqTH51012088 = APQldhUeJEBACgjtwqTH96397268;     APQldhUeJEBACgjtwqTH96397268 = APQldhUeJEBACgjtwqTH8645537;     APQldhUeJEBACgjtwqTH8645537 = APQldhUeJEBACgjtwqTH48413387;     APQldhUeJEBACgjtwqTH48413387 = APQldhUeJEBACgjtwqTH20916126;     APQldhUeJEBACgjtwqTH20916126 = APQldhUeJEBACgjtwqTH21079573;     APQldhUeJEBACgjtwqTH21079573 = APQldhUeJEBACgjtwqTH1590250;     APQldhUeJEBACgjtwqTH1590250 = APQldhUeJEBACgjtwqTH85318780;     APQldhUeJEBACgjtwqTH85318780 = APQldhUeJEBACgjtwqTH60461915;     APQldhUeJEBACgjtwqTH60461915 = APQldhUeJEBACgjtwqTH91126670;}
// Junk Finished
