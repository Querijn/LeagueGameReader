#pragma once

#include <league_gamereader/types.hpp>
#include <league_gamereader/game_object.hpp>

namespace LeagueGameReader
{
	struct Snapshot
	{
		f32 time;
		std::vector<GameObject> champions;
		std::vector<GameObject> minions;
		std::vector<GameObject> turrets;
	};
}