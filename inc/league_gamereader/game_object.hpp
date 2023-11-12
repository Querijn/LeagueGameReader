#pragma once

#include <league_gamereader/types.hpp>

namespace LeagueGameReader
{
	struct Float3
	{
		float x, y, z;
	};

	struct GameObject
	{
		std::string name;
		std::string displayName;

		u16	   team;
		u32	   networkID;
		Float3 position;
		f32	   health;
		f32	   maxHealth;
		f32	   mana;
		f32	   maxMana;
		u32	   lvl;
		f32	   experience;
		f32	   currentGold;
		f32	   totalGold;
	};
}
