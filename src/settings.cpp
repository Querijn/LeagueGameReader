#include "settings.hpp"
#include <sstream>

#include "json.hpp"

namespace LeagueGameReader
{
	u64 HexToU64(const std::string& input)
	{
		std::istringstream iss(input);
		u64 result;
		iss >> std::hex >> result;
		return result;
	}

	void Settings::Load(const char* jsonFileData)
	{
		try
		{
			nlohmann::json json = nlohmann::json::parse(jsonFileData);

#define LIST_SETTINGS_LOAD(Name) Name = HexToU64(json[#Name]);
			LIST_SETTINGS(LIST_SETTINGS_LOAD)
#undef LIST_SETTINGS_LOAD
		}
		catch (...)
		{
		}
	}

	bool Settings::IsValid() const
	{
#define LIST_SETTINGS_IS_VALID(Name) if (Name == 0) return false;
		LIST_SETTINGS(LIST_SETTINGS_IS_VALID)
#undef LIST_SETTINGS_IS_VALID
		return true;
	}

	void Settings::Invalidate()
	{
#define LIST_SETTINGS_INVALIDATE(Name) Name = 0;
		LIST_SETTINGS(LIST_SETTINGS_INVALIDATE)
#undef LIST_SETTINGS_INVALIDATE
	}
}
