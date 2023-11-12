#include <Windows.h>
#include <codecvt>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#include <memory>
#include <psapi.h>
#include <stdexcept>
#include <thread>
#include <vector>
#include <mutex>

#include "game_reader.hpp"
#include "process.hpp"
#include "settings.hpp"

#include <league_gamereader/library.hpp>
#include <league_gamereader/types.hpp>

#ifndef NUMCHARS
	#define NUMCHARS(x) (sizeof(x) / sizeof(TCHAR))
#endif

#if LGR_REMOTE_SETTINGS
	#include <wininet.h>
	#pragma comment(lib, "wininet.lib")
#endif

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "version.lib")

namespace LeagueGameReader
{
	namespace fs					 = std::filesystem;
	static const char* g_processName = "League of Legends.exe";
	static std::string g_repoOwner	 = "Querijn";
	static std::string g_repoName	 = "LeagueGameReader";

	static bool							g_isRunning		   = false;
	static std::unique_ptr<std::thread> g_connectionThread = nullptr;
	static Settings						g_settings;
	static Snapshot						g_readingSnapshot;
	static Snapshot						g_sharingSnapshot;
	static u64							g_snapshotInterval = 300;
	static std::mutex					g_copySnapshotMutex;
	Process								g_process(g_processName);

	template<typename... Args>
	static std::string string_format(const std::string& format, Args... args)
	{
		int stringSize = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
		if (stringSize <= 0)
			throw std::runtime_error("Error during formatting.");

		size_t					bufferSize = static_cast<size_t>(stringSize);
		std::unique_ptr<char[]> buf(new char[bufferSize]);
		std::snprintf(buf.get(), bufferSize, format.c_str(), args...);

		return std::string(buf.get(), buf.get() + bufferSize - 1); // We don't want the '\0' inside
	}

	std::string WideStringToString(const wchar_t* wstr)
	{
		using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converterX;

		return converterX.to_bytes(wstr);
	}

	static HRESULT GetProcessFileName(wchar_t* processName, int size)
	{
		if (GetProcessImageFileNameW(g_process.handle, processName, size) == 0)
		{
			auto error = GetLastError();
			return E_FAIL;
		}

		size_t	 stringLength = lstrlenW(processName);
		wchar_t* slashPointer = wcschr(&processName[1], '\\');
		if (slashPointer)
			slashPointer = wcschr(slashPointer + 1, '\\');
		if (!slashPointer)
			return E_FAIL;
		*slashPointer = 0;

		wchar_t targetPath[MAX_PATH];
		wchar_t driveFolder[MAX_PATH] = L"A:";
		for (wchar_t d = 'A'; d < 'Z'; ++d)
		{
			driveFolder[0] = d;
			targetPath[0]  = 0;
			if (QueryDosDeviceW(driveFolder, targetPath, NUMCHARS(targetPath)) == 0 || _wcsicmp(targetPath, processName) != 0)
				continue;

			std::wstring folder = std::wstring(driveFolder) + L"\\" + (slashPointer + 1);
			wcscpy_s(processName, size, folder.c_str());
			return S_OK;
		}

		return E_FAIL;
	}

	bool GetExecutableVersion(const char* filePath, std::string& result)
	{
		DWORD  verHandle = 0;
		UINT   size		 = 0;
		LPBYTE buffer	 = nullptr;
		DWORD  verSize	 = GetFileVersionInfoSize(filePath, &verHandle);

		if (verSize == 0)
			return false;

		std::vector<u8> data(verSize, 0);
		if (GetFileVersionInfo(filePath, verHandle, verSize, data.data()) == false)
			return false;

		if (VerQueryValue(data.data(), "\\", (VOID FAR * FAR*)&buffer, &size) == false || size == 0)
			return false;

		VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)buffer;
		if (verInfo->dwSignature != 0xfeef04bd)
			return false;

		result = string_format("%d.%d.%d.%d",
							   (verInfo->dwFileVersionMS >> 16) & 0xffff,
							   (verInfo->dwFileVersionMS >> 0) & 0xffff,
							   (verInfo->dwFileVersionLS >> 16) & 0xffff,
							   (verInfo->dwFileVersionLS >> 0) & 0xffff);
		return true;
	}

	bool LoadLocalSettings()
	{
		wchar_t processName[MAX_PATH];
		if (GetProcessFileName(processName, MAX_PATH) != S_OK)
			return false;

		std::string version;
		if (GetExecutableVersion(WideStringToString(processName).c_str(), version) == false)
			return false;

		fs::path offsetPath = "offsets/" + version + ".json";
		if (fs::exists(offsetPath) == false)
			return false;

		std::ifstream file(offsetPath);
		std::string	  content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		g_settings.Load(content.c_str());
		return g_settings.IsValid();
	}

	bool LoadRemoteSettings()
	{
#if LGR_REMOTE_SETTINGS
		wchar_t processName[MAX_PATH];
		if (GetProcessFileName(processName, MAX_PATH) != S_OK)
			return false;

		std::string version;
		if (GetExecutableVersion(WideStringToString(processName).c_str(), version) == false)
			return false;

		HINTERNET client = InternetOpenA("LeagueGameReader", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
		if (client == nullptr)
		{
			std::cerr << "InternetOpen failed: " << GetLastError() << std::endl;
			return false;
		}

		std::string url		= "https://raw.githubusercontent.com/" + g_repoOwner + "/" + g_repoName + "/main/offsets/" + version + ".json";
		HINTERNET	request = InternetOpenUrlA(client, url.c_str(), nullptr, 0, INTERNET_FLAG_RELOAD, 0);
		if (request == nullptr)
		{
			std::cerr << "InternetOpenUrl failed: " << GetLastError() << std::endl;
			InternetCloseHandle(client);
			return false;
		}

		std::string responseBody;
		char		buffer[1024];
		DWORD		bytesRead;
		while (InternetReadFile(request, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
			responseBody.append(buffer, bytesRead);

		InternetCloseHandle(request);
		InternetCloseHandle(client);
		g_settings.Load(responseBody.c_str());
#endif

		return g_settings.IsValid();
	}

	const Snapshot& GetCurrentSnapshot()
	{
		std::lock_guard l(g_copySnapshotMutex);
		return g_sharingSnapshot;
	}

	static bool OnBecameAlive()
	{
		bool settingsLoaded = LoadLocalSettings();
		if (settingsLoaded == false)
			settingsLoaded = LoadRemoteSettings();

		return settingsLoaded ? InitGameReader(g_settings) : false;
	}

	static void ConnectionThreadFunc()
	{
		bool wasAlive = false;
		while (g_isRunning)
		{
			g_process.Update();

			if (g_process.IsAlive() == false)
			{
				wasAlive = false;
				continue;
			}

			if (!wasAlive)
				wasAlive = OnBecameAlive();

			if (g_settings.IsValid())
			{
				ReadToSnapshot(g_process, g_settings, g_readingSnapshot);
				std::lock_guard l(g_copySnapshotMutex); // Lock the mutex while we copy the snapshot
				g_sharingSnapshot = g_readingSnapshot;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(g_process.IsAlive() ? g_snapshotInterval : 1000));
		}
	}

	ConnectStatus ConnectToGame()
	{
		if (g_connectionThread == nullptr)
		{
			g_isRunning		   = true;
			g_connectionThread = std::make_unique<std::thread>(ConnectionThreadFunc);
			g_connectionThread->detach();

			// Give the thread a bit of time to do its thing.
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		if (g_process.IsAlive() == false)
			return ConnectStatus::GameNotFound;

		if (g_process.handle == nullptr || g_process.handle == (void*)~0)
			return ConnectStatus::CannotConnect;

		if (g_process.baseAddress == 0)
			return ConnectStatus::CannotFindBase;

		if (g_settings.IsValid() == false)
			return ConnectStatus::SettingsMissing;

		return ConnectStatus::Connected;
	}

	void SetSnapshotInterval(int milliseconds)
	{
		g_snapshotInterval = milliseconds;
	}

	void Destroy()
	{
		g_isRunning		   = false;
		g_connectionThread = nullptr;
		g_process.Close();
	}
}
