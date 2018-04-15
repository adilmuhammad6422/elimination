#pragma once
#include "valve_sdk\csgostructs.hpp"
#include "imgui\imgui.h"
#include "options.hpp"

namespace specList
{
	extern int specs;
	extern int modes;
	extern std::string spect;
	extern std::string mode;

	extern int lastspecs;
	extern int lastmodes;
	extern std::string lastspect;
	extern std::string lastmode;

	void doSpectatorList();
}