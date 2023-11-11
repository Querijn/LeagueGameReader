#pragma once

#include <league_gamereader/types.hpp>

namespace LeagueGameReader
{
	#define LIST_SETTINGS(Func)		\
		Func(gameTime)				\
		Func(objIndex)				\
		Func(objTeam)				\
		Func(objNetworkID)			\
		Func(objPosition)			\
		Func(objHealth)				\
		Func(objMaxHealth)			\
		Func(objMana)				\
		Func(objMaxMana)			\
		Func(objName)				\
		Func(objNameLength)			\
		Func(objLvl)				\
		Func(objExperience)			\
		Func(objCurrentGold)		\
		Func(objTotalGold)			\
		Func(objDisplayName)		\
		Func(objDisplayNameLength)	\
		Func(objectManager)			\
		Func(objectMapCount)		\
		Func(objectMapRoot)			\
		Func(objectMapNodeNetId)	\
		Func(objectMapNodeObject)	\
		Func(heroList)				\
		Func(minionList)			\
		Func(turretList)			\
		Func(inhibitorList)			\

	class Settings
	{
	public:
#define LIST_SETTINGS_DEFINE(Name) u64 Name = 0;
		LIST_SETTINGS(LIST_SETTINGS_DEFINE)
#undef LIST_SETTINGS_DEFINE

		void Load(const char* jsonFileData);
		bool IsValid() const;
		void Invalidate();
	};
}
