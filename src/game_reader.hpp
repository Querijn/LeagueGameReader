#pragma once

#include <league_gamereader/general_items.hpp>

#include "settings.hpp"
#include "process.hpp"

namespace LeagueGameReader
{
	bool InitGameReader(const Settings& settings);
	bool ReadToSnapshot(const Process& process, const Settings& settings, Snapshot& snapshot);
}