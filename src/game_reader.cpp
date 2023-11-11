#include <string>
#include <vector>

#include "game_reader.hpp"

#include <cassert>
#include <fstream>

#include "process.hpp"
#include "settings.hpp"

#include "windows.h"

#define STR_MERGE_IMPL(x, y) x##y
#define STR_MERGE(x, y) STR_MERGE_IMPL(x, y)
#define MAKE_PAD(size) unsigned char STR_MERGE(pad_, __COUNTER__)[size]
#define DEFINE_MEMBER(x, offset) \
	struct                       \
	{                            \
		MAKE_PAD(offset);        \
		x;                       \
	}

namespace LeagueGameReader
{
	extern Process g_process;

	struct RiotString
	{
		static const u64 kMaxLength = 15;
		union Data
		{
			struct
			{
				u64 pointer;
			};
			struct
			{
				char content[kMaxLength + 1];
			};
		};

		Data data;
		u64	 length;
		u64	 capacity;

		bool IsSelfContaining() const { return capacity <= kMaxLength; }

		std::string ToString() const
		{
			if (IsSelfContaining())
				return data.content;

			assert(length < 63);
			char buffer[64];
			if (g_process.ReadAbs(data.pointer, buffer, length < 63 ? length : 62) == false)
				return "";

			buffer[length-1] = 0; // Very likely already zero-terminated, but whatever
			return buffer;
		}
	};

	template<typename T>
	struct RiotVector
	{
		Address begin;
		i32		count;
		i32		capacity;
	};

	struct ObjectList
	{
		Address			vtable;
		RiotVector<u64> items;
	};

	size_t g_gameObjectSize;

	bool InitGameReader(const Settings& settings)
	{
		assert(settings.IsValid());
		if (settings.IsValid() == false)
			return false;

		g_gameObjectSize = 0x2000; // TODO: Determine gameobject size
		return true;
	}

	bool ReadGameObject(u64 gameObjectAddressAbs, const Settings& settings, GameObject& object)
	{
		static u8 buffer[0x6000];
		if (!g_process.ReadAbs(gameObjectAddressAbs, buffer))
			return false;

		RiotString& name		= *(RiotString*)(buffer + settings.objName);
		RiotString& displayName = *(RiotString*)(buffer + settings.objDisplayName);

		if (name.length > 100)
			return false;
		if (displayName.length > 100)
			return false;

		object.name		   = name.ToString();
		object.displayName = displayName.ToString();
		object.team		   = *(u16*)(buffer + settings.objTeam);
		object.networkID   = *(u32*)(buffer + settings.objNetworkID);
		object.position	   = *(Float3*)(buffer + settings.objPosition);
		object.health	   = *(f32*)(buffer + settings.objHealth);
		object.maxHealth   = *(f32*)(buffer + settings.objMaxHealth);
		object.mana		   = *(f32*)(buffer + settings.objMana);
		object.maxMana	   = *(f32*)(buffer + settings.objMaxMana);
		object.lvl		   = *(u32*)(buffer + settings.objLvl);
		object.experience  = *(f32*)(buffer + settings.objExperience);
		object.currentGold = *(f32*)(buffer + settings.objCurrentGold);
		object.totalGold   = *(f32*)(buffer + settings.objTotalGold);
		return true;
	}

	bool ReadGameObjectList(u64 listAddressRel, const Settings& settings, std::vector<GameObject>& list)
	{
		u64 absoluteAddress;
		if (!g_process.ReadRel(listAddressRel, absoluteAddress))
			return false;

		ObjectList objectlist;
		if (!g_process.ReadAbs(absoluteAddress, objectlist))
			return false;

		u64 gameObjectAddresses[100];
		int listIndex = 0;
		while (true)
		{
			int readCount = (objectlist.items.count - listIndex < 100) ? (objectlist.items.count - listIndex) : 100;
			if (readCount == 0)
				break;

			if (!g_process.ReadAbs(objectlist.items.begin + listIndex, gameObjectAddresses, sizeof(u64) * readCount))
				return false;

			for (int i = 0; i < readCount; i++)
			{
				GameObject obj;
				if (ReadGameObject(gameObjectAddresses[i], settings, obj))
					list.push_back(obj);
			}

			listIndex += readCount;
		}

		return true;
	}

	bool ReadToSnapshot(const Process& process, const Settings& settings, Snapshot& snapshot)
	{
		snapshot = Snapshot(); // Reset
		if (settings.IsValid() == false)
			return false;

		if (!g_process.ReadRel(settings.gameTime, snapshot.time))
			return false;

		// Rest of the game isn't ready before the game started.
		if (snapshot.time < 1.0f)
			return true;

		if (ReadGameObjectList(settings.heroList, settings, snapshot.champions) == false)
			return false;

		if (ReadGameObjectList(settings.minionList, settings, snapshot.minions) == false)
			return false;

		if (ReadGameObjectList(settings.turretList, settings, snapshot.turrets) == false)
			return false;

		return true;
	}
}