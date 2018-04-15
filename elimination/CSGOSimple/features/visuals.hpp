#pragma once

#include "../valve_sdk/Math/Vector.hpp"
#include "../valve_sdk/Interfaces/IVEngineClient.hpp"
#include "../SpoofedConvar.hpp"
#include "../valve_sdk/csgostructs.hpp"

class C_BasePlayer;
class C_BaseEntity;
class C_BaseCombatWeapon;
class C_PlantedC4;
class Color;
class ClientClass;

namespace Visuals
{
    struct ESPBox
    {
        int x, y, w, h, gay;
    };

    namespace Player
    {
        bool Begin(C_BasePlayer* pl);

        void RenderBox();
        void RenderName();
        void RenderHealth();
		void RenderArmour();
        void RenderWeapon();
        void RenderSnapline();
    }

    namespace Misc
    {
        void RenderCrosshair();
        void RenderWeapon(C_BaseCombatWeapon* ent);
        void RenderDefuseKit(C_BaseEntity* ent);
        void RenderPlantedC4(C_BaseEntity* ent);
		void DrawSkeleton(IClientEntity* pEntity);
		
		// gay shit
		void RenderDrugs();
		void RenderPaper();
		void RenderLowResTextures();
		void ChatSpam();
    }

    bool CreateFonts();
    void DestroyFonts();
}