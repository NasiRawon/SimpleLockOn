#include "Console.h"
#include "Settings.h"
#include "PatternScanner.h"
#include "skse64_common/SafeWrite.h"


namespace Tralala
{
	uintptr_t g_consoleMgrAddr = 0;
	uintptr_t g_vPrintAddr = 0;

	void ConsoleGetAddresses()
	{
		const std::array<BYTE, 7> pattern = { 0x41, 0x0F, 0xB7, 0xD0, 0x48, 0x8B, 0xC8 };
		g_consoleMgrAddr = (uintptr_t)scan_memory_data(pattern, 0x46, true, 0x3, 0x7);

		const std::array<BYTE, 7> printpattern = { 0x8B, 0xD0, 0x85, 0xC0, 0x0F, 0x48, 0xD7 };
		g_vPrintAddr = (uintptr_t)scan_memory(printpattern, 0xE1, false);
	}

	ConsoleManager* ConsoleManager::GetSingleton()
	{
		return *(ConsoleManager * *)g_consoleMgrAddr;
	}

	void ConsoleManager::VPrint(const char* fmt, va_list args)
	{
		typedef void(*VPrint_t)(ConsoleManager*, const char*, va_list);
		VPrint_t VPrint = (VPrint_t)g_vPrintAddr;

		VPrint(this, fmt, args);
	}

	void Console_Print(const char* fmt, ...)
	{
		ConsoleManager* mgr = ConsoleManager::GetSingleton();
		if (mgr)
		{
			va_list args;
			va_start(args, fmt);

			mgr->VPrint(fmt, args);

			va_end(args);
		}
	}

	static ObScriptCommand* s_hijackedCommand = nullptr;

	static bool Cmd_SetLockOnVariable_Execute(void* paramInfo, void* scriptData, TESObjectREFR* thisObj, void* containingObj, void* scriptObj, void* locals, double* result, void* opcodeOffsetPtr)
	{
		ObScriptCommand::ScriptData* scriptDataEx = (ObScriptCommand::ScriptData*)scriptData;
		ObScriptCommand::StringChunk* strChunk = (ObScriptCommand::StringChunk*)scriptDataEx->GetChunk();
		std::string name = strChunk->GetString();

		ObScriptCommand::IntegerChunk* intChunk = (ObScriptCommand::IntegerChunk*)strChunk->GetNext();
		int val = intChunk->GetInteger();

		if (Settings::Set(name.c_str(), val))
			Console_Print("> set Settings::%s = %d", name.c_str(), val);
		else
			Console_Print("> (Error) Settings::%s is not found.", name.c_str());

		return true;
	}
}

namespace ConsoleCommand
{

	uintptr_t g_firstConsoleAddr = 0;

	void GetAddress()
	{
		const std::array<BYTE, 10> cmdpattern = { 0xC7, 0x85, 0x6C, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		g_firstConsoleAddr = (uintptr_t)scan_memory_data(cmdpattern, 0x49, true, 0x3, 0x7);
	}

	void Register()
	{
		_MESSAGE("ConsoleCommand::Register()");

		for (Tralala::ObScriptCommand * iter = (Tralala::ObScriptCommand*)g_firstConsoleAddr; iter->opcode < kObScript_NumConsoleCommands + kObScript_ConsoleOpBase; ++iter)
		{
			if (!strcmp(iter->longName, "ToggleESRAM")) // unused
			{
				Tralala::s_hijackedCommand = iter;
				break;
			}
		}

		Tralala::ObScriptCommand cmd = *Tralala::s_hijackedCommand;
		static Tralala::ObScriptParam params[] = {
			{ "String", 0, 0 },
			{ "Integer", 1, 0 }
		};
		cmd.longName = "SetLockOnVariable";
		cmd.shortName = "setlockonvar";
		cmd.helpText = "";
		cmd.needsParent = 0;
		cmd.numParams = 2;
		cmd.params = params;
		cmd.execute = &Tralala::Cmd_SetLockOnVariable_Execute;
		cmd.eval = nullptr;
		cmd.flags = 0;

		SafeWriteBuf((uintptr_t)Tralala::s_hijackedCommand, &cmd, sizeof(cmd));
	}
}
