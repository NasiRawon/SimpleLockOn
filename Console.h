#pragma once

#include "skse64/ObScript.h"

namespace Tralala
{

	void ConsoleGetAddresses();

	class ConsoleManager
	{
	public:
		
		static ConsoleManager* GetSingleton();

		void VPrint(const char* fmt, va_list args);
	};

	void Console_Print(const char* fmt, ...);

	// 0C
	struct ObScriptParam
	{
		const char* typeStr;	// 00
		UInt32		typeID;		// 04
		UInt32		isOptional;	// 08
	};

	typedef bool (*ObScript_Execute)(void* paramInfo, void* scriptData, TESObjectREFR* thisObj, void* containingObj, void* scriptObj, void* locals, double* result, void* opcodeOffsetPtr);

	// 50
	struct ObScriptCommand
	{
		struct Chunk
		{
		};

		struct StringChunk : public Chunk
		{
			UInt16	length;		// 00
			char	str[1];		// 02

			std::string GetString() const
			{
				return std::string(str, length);
			}

			Chunk* GetNext()
			{
				UInt16 len = *(UInt16*)this;
				return (Chunk*)(str + length);
			}
		};

		struct IntegerChunk : public Chunk
		{
			char	unk00;		// 00
			int		value;		// 01

			int GetInteger() const
			{
				return *(int*)((char*)this + 1);
			}

			Chunk* GetNext()
			{
				UInt16 len = *(UInt16*)this;
				return (Chunk*)(this + 1);
			}
		};

		struct ScriptData
		{
			UInt16	opcode;			// 00
			UInt16	chunkSize;		// 02
			UInt16	numParams;		// 04

			Chunk* GetChunk()
			{
				return (Chunk*)((uintptr_t)this + 6);
			}

			UInt16	strLen;			// 06
			char	str[1];			// 08
		};

		const char* longName;		// 00
		const char* shortName;	// 08
		UInt32				opcode;			// 10
		UInt32				pad14;			// 14
		const char* helpText;		// 18
		UInt8				needsParent;	// 20
		UInt8				pad21;			// 21
		UInt16				numParams;		// 22
		UInt32				pad24;			// 24
		ObScriptParam* params;		// 28

		// handlers
		ObScript_Execute	execute;		// 30
		void* parse;		// 38
		void* eval;			// 40

		UInt32				flags;			// 48
		UInt32				pad4C;			// 4C
	};
}

namespace ConsoleCommand
{
	void GetAddress();
	void Register();
}
