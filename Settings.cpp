#include "Settings.h"
#include <Shlwapi.h>
#include <string>

UInt32	Settings::uHotkey = 42;
UInt32	Settings::uMaxDistance = 2048;
bool	Settings::bHostileActors = true;
bool	Settings::bEssentialActors = true;

std::wstring thisPath = L"";

bool Settings::SetBool(const char *name, bool val)
{
	if (_stricmp(name, "bOnlyHostile") == 0)
	{
		bHostileActors = val;
		WritePrivateProfileString(L"Settings", L"bOnlyHostile", val ? L"1" : L"0", thisPath.c_str());
	}
	else if (_stricmp(name, "bEssentialActor") == 0)
	{
		bEssentialActors = val;
		WritePrivateProfileString(L"Settings", L"bEssentialActor", val ? L"1" : L"0", thisPath.c_str());
	}
	else
	{
		return false;
	}

	_MESSAGE("  %s = %d", name, val);

	return true;
}


bool Settings::SetInteger(const char *name, int val)
{
	if (_stricmp(name, "uHotKey") == 0)
	{
		uHotkey = val;
		WritePrivateProfileString(L"Settings", L"uHotKey", std::to_wstring(val).c_str(), thisPath.c_str());
	}
	else if (_stricmp(name, "uMaxDistance") == 0)
	{
		if (val < 2048)
			val = 2048;

		uMaxDistance = val;
		WritePrivateProfileString(L"Settings", L"uMaxDistance", std::to_wstring(val).c_str(), thisPath.c_str());
	}
	else
	{
		return false;
	}

	_MESSAGE("  %s = %d", name, (int)val);

	return true;
}


bool Settings::Set(const char *name, int val)
{
	if (!name || !name[0])
		return false;

	if (name[0] == 'b')
		return SetBool(name, (val != 0));
	if (name[0] == 'u')
		return SetInteger(name, val);

	return false;
}

void Settings::Load()
{
	TCHAR path[MAX_PATH];
	HMODULE hm;
	if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		L"SimpleLockOn.dll", &hm))
	{
		GetModuleFileName(hm, path, MAX_PATH);
		PathRemoveFileSpec(path);
		thisPath = std::wstring(path);
		if (!thisPath.empty() && thisPath.at(thisPath.length() - 1) != '\\'){
			thisPath += L"\\SimpleLockOn.ini";
		}
	}

	Settings::uHotkey = GetPrivateProfileInt(L"Settings", L"uHotKey", 42, thisPath.c_str());
	Settings::bHostileActors = GetPrivateProfileInt(L"Settings", L"bOnlyHostile", 1, thisPath.c_str());
	Settings::bEssentialActors = GetPrivateProfileInt(L"Settings", L"bEssentialActor", 1, thisPath.c_str());
	Settings::uMaxDistance = GetPrivateProfileInt(L"Settings", L"uMaxDistance", 2048, thisPath.c_str());
}