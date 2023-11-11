#pragma once

#include <league_gamereader/types.hpp>

#include <string>
#include <string_view>

#undef GetCommandLine

namespace LeagueGameReader
{
	enum ProcessQuery
	{
		No,
		NotAvailable,
		Yes
	};

	struct Process
	{
		std::string processName;
		void*		handle		= (void*)~0;
		u32			pid			= 0;
		u64			baseAddress = 0;
		bool		alive		= false;

		Process(const char* name);
		~Process();

		void Update();
		void Close();

		bool		 IsAlive() const;
		ProcessQuery IsModuleLoaded(std::string_view moduleName, u64* address) const;
		u32			 GetCommandLine(std::wstring& commandLine) const;
		bool		 ReadRel(u64 relativeAddress, void* buffer, size_t size) const;
		template<typename T>
		bool ReadRel(u64 relativeAddress, T& value) const;

		bool ReadAbs(u64 absoluteAddress, void* buffer, size_t size) const;
		template<typename T>
		bool ReadAbs(u64 absoluteAddress, T& value) const;

	  private:
		static void* InitHandle(const char* name);
		static u32	 GetPID(const char* name);
		bool		 CheckAlive();
	};

	template<typename T>
	inline bool Process::ReadRel(u64 relativeAddress, T& value) const
	{
		return ReadRel(relativeAddress, &value, sizeof(T));
	}

	template<typename T>
	inline bool Process::ReadAbs(u64 absoluteAddress, T& value) const
	{
		return ReadAbs(absoluteAddress, &value, sizeof(T));
	}
}