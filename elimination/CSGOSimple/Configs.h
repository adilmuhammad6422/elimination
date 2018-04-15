#pragma once
#include "JSON.h"
#include "valve_sdk\csgostructs.hpp"
#include "options.hpp"
#include <unordered_map>
#include <array>
#include <algorithm>
#include <fstream>

namespace Configs
{
	void DumpClassIDs(const char* fileName);

	void SaveCFG(std::string fileName);
	void LoadCFG(std::string fileName);
}