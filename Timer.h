#pragma once
#include <windows.h>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")
#include <iostream>
#include <TlHelp32.h>
#include <regex>
void Dependencies()
{
	const char* url = "https://github.com/valleyofdoom/TimerResolution/releases/download/SetTimerResolution-v1.0.0/SetTimerResolution.exe";
	const char* url2 = "https://github.com/valleyofdoom/TimerResolution/releases/download/MeasureSleep-v1.0.0/MeasureSleep.exe";
	const char* url3 = "https://raw.githubusercontent.com/valleyofdoom/TimerResolution/refs/heads/main/micro-adjust-benchmark.ps1";
	const char* oPath = "SetTimerResolution.exe";
	const char* outputPath = "MeasureSleep.exe";
	const char* oPath3 = "TimerResolution-Benchmark.ps1";

	HRESULT result = URLDownloadToFileA(NULL, url, oPath, 0, NULL);
	HRESULT result2 = URLDownloadToFileA(NULL, url2, outputPath, 0, NULL);
	HRESULT result3 = URLDownloadToFileA(NULL, url3, oPath3, 0, NULL);

	if (result == S_OK)
	{
		MessageBoxA(0, "SetTimerResolution Descargado Exitosamente!", "Exito", MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBoxA(0, "Error al Descargar SetTimerResolution", "Error", MB_OK | MB_ICONINFORMATION);
	}
	if (result2 == S_OK)
	{
		MessageBoxA(0, "MeasureSleep Descargado Exitosamente!", "Exito", MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBoxA(0, "Error al Descargar MeasureSleep", "Error", MB_OK | MB_ICONINFORMATION);
	}
	if (result3 == S_OK)
	{
		MessageBoxA(0, "Benchmark de Timer Resolution Descargado Exitosamente!", "Exito", MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		MessageBoxA(0, "Error al Descargar Benchmark de Timer Resolution", "Error", MB_OK | MB_ICONINFORMATION);
	}
}
bool SetGlobalTimerResolution()
{
	HKEY hKey;
	LPCSTR subKey = R"(SYSTEM\CurrentControlSet\Control\Session Manager\kernel)";
	DWORD value = 1;
	LONG result = RegCreateKeyExA(
		HKEY_LOCAL_MACHINE,
		subKey,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_SET_VALUE,
		NULL,
		&hKey,
		NULL
	);
	if (result != ERROR_SUCCESS) {
		std::cerr << "Error al abrir/crear la clave: " << result << std::endl;
		return false;
	}

	result = RegSetValueExA(
		hKey,
		"GlobalTimerResolutionRequests",
		0,
		REG_DWORD,
		reinterpret_cast<const BYTE*>(&value),
		sizeof(value)
	);

	RegCloseKey(hKey);

	if (result != ERROR_SUCCESS) {
		std::cerr << "Error al establecer el valor: " << result << std::endl;
		return false;
	}

	std::cout << "Clave de registro escrita correctamente." << std::endl;
	return true;
}
bool IsProcessRunning(const std::string& processName)
{
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	if (Process32First(snapshot, &entry))
	{
		do
		{
			if (_stricmp(entry.szExeFile, processName.c_str()) == 0)
			{
				CloseHandle(snapshot);
				return true;
			}
		} while (Process32Next(snapshot, &entry));
	}
	CloseHandle(snapshot);
	return false;
}

void RunPowerShellCommand(const std::string& scriptURL)
{
	const std::string command = "powershell -ExecutionPolicy Bypass -WindowStyle Hidden -Command \"iwr -UseBasicParsing -Uri '" + scriptURL + "' | iex\"";

	system(command.c_str());
}
void DisableIdleState()
{
	system("powershell -Command \"powercfg -setacvalueindex SCHEME_CURRENT SUB_PROCESSOR IDLEDISABLE 1; powercfg -setactive SCHEME_CURRENT\"");
}

bool CheckBcdeditValue(const std::string& key, const std::string& expected)
{
	std::string command = "bcdedit";
	FILE* pipe = _popen(command.c_str(), "r");
	if (!pipe) return false;

	char buffer[256];
	std::string output;

	while (fgets(buffer, sizeof(buffer), pipe) != NULL)
	{
		output += buffer;
	}
	_pclose(pipe);

	std::regex pattern(key + R"(.*\s+(\S+))", std::regex::icase);
	std::smatch match;

	if (std::regex_search(output, match, pattern))
	{
		std::string value = match[1];
		std::transform(value.begin(), value.end(), value.begin(), ::tolower);
		std::string expectedLower = expected;
		std::transform(expectedLower.begin(), expectedLower.end(), expectedLower.begin(), ::tolower);
		return value == expectedLower;
	}

	return false;
}

bool CheckSystemTweaks()
{
	bool ok = true;
	if (!CheckBcdeditValue("nx", "AlwaysOff"))
	{
		std::cerr << "[ERROR] nx no esta en AlwaysOff\n";
		ok = false;
	}
	if (!CheckBcdeditValue("disabledynamictick", "Yes"))
	{
		std::cerr << "[ERROR] disabledynamictick no esta en Yes\n";
		ok = false;
	}
	if (!CheckBcdeditValue("useplatformclock", "No"))
	{
		std::cerr << "[ERROR] useplatformclock no esta en No\n";
		ok = false;
	}
	if (!CheckBcdeditValue("useplatformtick", "No"))
	{
		std::cerr << "[ERROR] useplatformtick no esta en No\n";
		ok = false;
	}

	return ok;
}
void FixSystemTweaks()
{
	std::cout << "\nCorrigiendo ajustes de bcdedit...\n";
	system("bcdedit /set nx AlwaysOff");
	system("bcdedit /set disabledynamictick yes");
	system("bcdedit /set useplatformclock no");
	system("bcdedit /set useplatformtick no");
	std::cout << "Ajustes corregidos. Es necesario reiniciar para aplicar completamente los cambios.\n";
}

bool IsRunningAsAdmin() {
	BOOL fIsRunAsAdmin = FALSE;
	PSID pAdministratorsGroup = NULL;
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

	if (AllocateAndInitializeSid(
		&NtAuthority, 2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&pAdministratorsGroup))
	{
		CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin);
		FreeSid(pAdministratorsGroup);
	}
	return fIsRunAsAdmin;
}