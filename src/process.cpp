#include "process.hpp"

#include <Windows.h>
#include <TlHelp32.h>
#include <winternl.h>
#include <psapi.h>
#include <cassert>
#include <filesystem>
#undef GetCommandLine

namespace LeagueGameReader
{
	namespace fs = std::filesystem;
	typedef NTSTATUS(NTAPI* _NtQueryInformationProcess)(HANDLE ProcessHandle, DWORD ProcessInformationClass, PVOID ProcessInformation, DWORD ProcessInformationLength, PDWORD ReturnLength);
	typedef struct _UNICODE_STRING
	{
		USHORT Length;
		USHORT MaximumLength;
		PWSTR Buffer;
	} UNICODE_STRING, * PUNICODE_STRING;
	typedef struct _PROCESS_BASIC_INFORMATION
	{
		LONG ExitStatus;
		PVOID PebBaseAddress;
		ULONG_PTR AffinityMask;
		LONG BasePriority;
		ULONG_PTR UniqueProcessId;
		ULONG_PTR ParentProcessId;
	} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;

	PVOID GetPebAddress(HANDLE ProcessHandle)
	{
		_NtQueryInformationProcess NtQueryInformationProcess = (_NtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
		PROCESS_BASIC_INFORMATION pbi;
		NtQueryInformationProcess(ProcessHandle, 0, &pbi, sizeof(pbi), NULL);
		return pbi.PebBaseAddress;
	}

	Process::Process(const char* name) :
		processName(name)
	{
	}

	Process::~Process()
	{
		Close();
	}

	void Process::Update()
	{
		alive = false; // Reset alive cache
		if (IsAlive() == false)
			Close();

		if (pid == 0)
			pid = GetPID(processName.c_str());

		if (handle == INVALID_HANDLE_VALUE && pid != 0)
			handle = InitHandle(processName.c_str());

		if (handle)
		{
			HMODULE modules[1024];
			DWORD bytesNeeded;
			if (EnumProcessModules(handle, modules, sizeof(modules), &bytesNeeded))
			{
				baseAddress = (u64)modules[0];
			}
		}
	}

	void Process::Close()
	{
		pid = 0;
		if (handle)
		{
			CloseHandle(handle);
			handle = INVALID_HANDLE_VALUE;
		}
		baseAddress = 0;
	}

	bool Process::IsAlive() const
	{
		if (alive)
			return alive;

		TCHAR filename[MAX_PATH];
		if (handle == INVALID_HANDLE_VALUE || pid == 0)
			return false;
		const_cast<bool&>(alive) = GetModuleFileNameEx(handle, NULL, filename, MAX_PATH) != 0;
		return alive;
	}

	ProcessQuery Process::IsModuleLoaded(std::string_view moduleName, u64* address) const
	{
		fs::path moduleToFind = fs::absolute(moduleName);
		HMODULE modules[1024];
		DWORD bytesNeeded;
		if (EnumProcessModules(handle, modules, sizeof(modules), &bytesNeeded) == false)
			return ProcessQuery::NotAvailable;

		unsigned int count = bytesNeeded / sizeof(HMODULE);
		for (unsigned int i = 0; i < count; i++)
		{
			TCHAR currentModule[MAX_PATH];
			if (GetModuleFileNameEx(handle, modules[i], currentModule, sizeof(currentModule) / sizeof(TCHAR)) == false)
				continue;

			fs::path modulePath(currentModule);
			if (moduleToFind == modulePath)
			{
				if (address)
					*address = (u64)modules[i];
				return ProcessQuery::Yes;
			}
		}
		
		return ProcessQuery::No;
	}

	u32 Process::GetCommandLine(std::wstring& commandLineContents) const
	{
		PVOID pebAddress;
		PVOID userProcParamsAddress;
		UNICODE_STRING commandLine;

		pebAddress = GetPebAddress(handle);

		/* get the address of ProcessParameters */
		if (!ReadProcessMemory(handle, (PCHAR)pebAddress + 0x10, &userProcParamsAddress, sizeof(PVOID), NULL))
		{
			printf("Could not read the address of ProcessParameters!\n");
			return GetLastError();
		}

		/* read the CommandLine UNICODE_STRING structure */
		if (!ReadProcessMemory(handle, (PCHAR)userProcParamsAddress + 0x40, &commandLine, sizeof(commandLine), NULL))
		{
			printf("Could not read CommandLine!\n");
			return GetLastError();
		}

		commandLineContents.resize(commandLine.Length);

		/* read the command line */
		if (!ReadProcessMemory(handle, commandLine.Buffer, commandLineContents.data(), commandLine.Length, NULL))
		{
			printf("Could not read the command line string!\n");
			return GetLastError();
		}

		return 0;
	}

	bool Process::ReadRel(u64 relativeAddress, void* buffer, size_t size) const
	{
		return ReadAbs(baseAddress + relativeAddress, buffer, size);
	}

	bool Process::ReadAbs(u64 absoluteAddress, void* buffer, size_t size) const
	{
		if (IsAlive() == false)
			return false;

		DWORD error = GetLastError();
		return !!ReadProcessMemory(handle, (LPCVOID)absoluteAddress, buffer, size, 0);
	}

	void* Process::InitHandle(const char* name)
	{
		DWORD allOperations = PROCESS_CREATE_THREAD |
			PROCESS_VM_OPERATION |
			PROCESS_VM_READ |
			PROCESS_VM_WRITE |
			PROCESS_QUERY_INFORMATION |
			PROCESS_SUSPEND_RESUME;
		DWORD pid = GetPID(name);
		return pid != 0 ? OpenProcess(allOperations, FALSE, pid) : INVALID_HANDLE_VALUE;
	}

	u32 Process::GetPID(const char* name)
	{
		DWORD pid = 0;
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		PROCESSENTRY32 process;
		ZeroMemory(&process, sizeof(process));
		process.dwSize = sizeof(process);

		// Walkthrough all processes.
		if (Process32First(snapshot, &process))
		{
			do
			{
				if (strcmp(process.szExeFile, name) == 0)
				{
					pid = process.th32ProcessID;
					break;
				}
			} while (Process32Next(snapshot, &process));
		}

		CloseHandle(snapshot);
		return pid;
	}

	bool Process::CheckAlive()
	{
		if (IsAlive())
			return true;

		handle = INVALID_HANDLE_VALUE;
		pid = 0;
		return false;
	}
}
