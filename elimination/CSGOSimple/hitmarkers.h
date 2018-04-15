#pragma once


#include <Windows.h>
#pragma comment(lib, "Winmm.lib")

#include "valve_sdk\interfaces\IGameEventmanager.hpp"
#include "valve_sdk\sdk.hpp"
#include "menu.hpp"
#include "Options.hpp"

#define g_pGameEventManager g_GameEvents

class hitmarker
{
	class player_hurt_listener
		: public IGameEventListener2
	{
	public:
		void start()
		{
			if (!g_pGameEventManager->AddListener(this, "player_hurt", false)) {
				throw std::exception("Failed to register the event");
			}
		}
		void stop()
		{
			g_pGameEventManager->RemoveListener(this);
		}
		void FireGameEvent(IGameEvent *event) override
		{
			hitmarker::singleton()->on_fire_event(event);
		}
		int GetEventDebugID(void) override
		{
			return 0x2A;
		}
	};

public:
	static hitmarker* singleton()
	{
		static hitmarker* instance = new hitmarker;
		return instance;
	}

	void initialize()
	{
		_listener.start();
	}
	void on_fire_event(IGameEvent* event)
	{
		if (!strcmp(event->GetName(), "player_hurt")) {
			int attacker = event->GetInt("attacker");
			if (g_EngineClient->GetPlayerForUserID(attacker) == g_EngineClient->GetLocalPlayer() && g_Options.hitmarkers == 1)
			{
				_flHurtTime = g_GlobalVars->curtime;
			}
			switch (g_Options.hitmarkers_sound) {
			case NOHITSOUND:
				break;
			case CODSOUND:
				if (g_EngineClient->GetPlayerForUserID(attacker) == g_EngineClient->GetLocalPlayer())
					PlaySoundA(_soundCOD, NULL, SND_ASYNC);
				break;
			case Anime:
				if (g_EngineClient->GetPlayerForUserID(attacker) == g_EngineClient->GetLocalPlayer())
					PlaySoundA(_soundAnime, NULL, SND_ASYNC);
				break;
			case Bubbles:
				if (g_EngineClient->GetPlayerForUserID(attacker) == g_EngineClient->GetLocalPlayer())
					PlaySoundA(_soundBubble, NULL, SND_ASYNC);
				break;
			case Custom:
				if (g_EngineClient->GetPlayerForUserID(attacker) == g_EngineClient->GetLocalPlayer())
					PlaySoundA(_soundCustom, NULL, SND_ASYNC);
				break;

			}
		}
	}


	void on_paint()
	{
		auto curtime = g_GlobalVars->curtime;
		auto lineSize = 8;
		if (_flHurtTime + 0.25f >= curtime)
		{
			int screenSizeX, screenCenterX;
			int screenSizeY, screenCenterY;
			g_EngineClient->GetScreenSize(screenSizeX, screenSizeY);

			screenCenterX = screenSizeX / 2;
			screenCenterY = screenSizeY / 2;

#define g_pSurface g_VGuiSurface
			g_pSurface->DrawSetColor(200, 200, 200, 255);
			g_pSurface->DrawLine(screenCenterX - lineSize, screenCenterY - lineSize, screenCenterX - (lineSize / 4), screenCenterY - (lineSize / 4));
			g_pSurface->DrawLine(screenCenterX - lineSize, screenCenterY + lineSize, screenCenterX - (lineSize / 4), screenCenterY + (lineSize / 4));
			g_pSurface->DrawLine(screenCenterX + lineSize, screenCenterY + lineSize, screenCenterX + (lineSize / 4), screenCenterY + (lineSize / 4));
			g_pSurface->DrawLine(screenCenterX + lineSize, screenCenterY - lineSize, screenCenterX + (lineSize / 4), screenCenterY - (lineSize / 4));
		}
	}

private:
	player_hurt_listener    _listener;
	const char*             _soundCOD = "C:\\Hitmarker.wav";
	const char*             _soundAnime = "C:\\anime.wav";
	const char*             _soundBubble = "C:\\bubble.wav";
	const char*             _soundCustom = "C:\\custom.wav";
	float                   _flHurtTime;
};