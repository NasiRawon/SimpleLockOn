#pragma once

struct Settings
{
	static UInt32	uHotkey;
	static bool		bHostileActors;
	static bool		bEssentialActors;
	static UInt32	uMaxDistance;

	static bool SetBool(const char *name, bool val);
	static bool SetInteger(const char *name, int val);
	static bool Set(const char *name, int val);

	static void Load();
};