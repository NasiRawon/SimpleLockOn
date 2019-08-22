#pragma once

#include "skse64/GameEvents.h"
#include "Controls.h"
#include "Utils.h"
#include "ObjectRef.h"

namespace Tralala
{
	extern uintptr_t g_addEventAddr;
	extern uintptr_t g_removeEventAddr;

	void EventGetAddresses();

	template <typename T> class BSTEventSink;

	enum EventResult
	{
		kEvent_Continue = 0,
		kEvent_Abort
	};

	// 058 
	template <typename EventT, typename EventArgT = EventT>
	class EventDispatcher
	{
	protected:
		typedef BSTEventSink<EventT> SinkT;

		tArray<SinkT*>		eventSinks;			// 000
		tArray<SinkT*>		addBuffer;			// 018 - schedule for add
		tArray<SinkT*>		removeBuffer;		// 030 - schedule for remove
		SimpleLock			lock;				// 048
		bool				stateFlag;			// 050 - some internal state changed while sending
		char				pad[7];				// 051

	public:

		EventDispatcher() : stateFlag(false) {}

		void AddEventSinkEx(SinkT* eventSink)
		{
			typedef void(*AddEventSink_t)(EventDispatcher<EventT, EventArgT>*, SinkT*);
			AddEventSink_t AddEventSink = (AddEventSink_t)g_addEventAddr;

			AddEventSink(this, eventSink);
		}
		void RemoveEventSinkEx(SinkT* eventSink)
		{
			typedef void(*RemoveEventSink_t)(EventDispatcher<EventT, EventArgT>*, SinkT*);
			RemoveEventSink_t RemoveEventSink = (RemoveEventSink_t)g_removeEventAddr;

			RemoveEventSink(this, eventSink);
		}
	};
	STATIC_ASSERT(sizeof(EventDispatcher<void*>) == 0x58);

	// 10 
	struct MenuOpenCloseEvent
	{
		BSFixedString	menuName;	// 00
		bool			opening;	// 08
		char			pad[7];
	};

	struct TESDeathEvent
	{
		TESObjectREFR	* source;	// 00
		TESObjectREFR	* killer;	// 08
		UInt8			state;		// 10 -  0 - dying; 1 - death
	};

	// 08 
	template <typename T>
	class BSTEventSink
	{
	public:
		virtual ~BSTEventSink();
		virtual	EventResult	ReceiveEvent(T * evn, EventDispatcher<T> * dispatcher); // pure
		//	void	** _vtbl;	// 00
	};

	template <>
	class BSTEventSink <MenuOpenCloseEvent>
	{
	public:
		virtual ~BSTEventSink() {}	// todo?
		virtual	EventResult ReceiveEvent(MenuOpenCloseEvent * evn, EventDispatcher<MenuOpenCloseEvent> * dispatcher) = 0;
	};

	template <>
	class BSTEventSink <TESDeathEvent>
	{
	public:
		virtual ~BSTEventSink() {}; // todo?
		virtual	EventResult ReceiveEvent(TESDeathEvent * evn, EventDispatcher<TESDeathEvent> * dispatcher) = 0;
	};

	// E8 
	class InputEventDispatcher : public EventDispatcher<InputEvent, InputEvent*>
	{
	public:
		UInt32			unk058;			// 058
		UInt32			pad05C;			// 05C
		BSInputDevice	* keyboard;		// 060 
		BSInputDevice	* mouse;		// 068
		BSInputDevice	* gamepad;		// 070
		BSInputDevice	* vkeyboard;	// 078	- New in SE  .?AVBSWin32VirtualKeyboardDevice@@
		UInt8			unk080;			// 080
		UInt8			unk081;			// 081
		UInt8			pad082[6];		// 082
		BSTEventSource<void *>	unk088;	// 088	- TODO: template type
		UInt8			unk0E0;			// 0E0
		UInt8			pad0E1[7];		// 0E1

		bool	IsGamepadEnabled();

		static InputEventDispatcher* GetSingleton();
	};
	STATIC_ASSERT(offsetof(InputEventDispatcher, gamepad) == 0x70);
	STATIC_ASSERT(sizeof(InputEventDispatcher) == 0xE8);

	template <>
	class BSTEventSink <InputEvent>
	{
	public:
		virtual ~BSTEventSink() {}; // todo?
		virtual	EventResult ReceiveEvent(InputEvent ** evn, InputEventDispatcher * dispatcher) = 0;
	};

	class EventDispatcherList
	{
	public:
		EventDispatcher<void>								unk00;					//	00
		EventDispatcher<void>								activateDispatcher;		//  58  - sink offset 010
		EventDispatcher<TESActiveEffectApplyRemoveEvent>	unkB0;					//  B0  - sink offset 018
		EventDispatcher<void>								locationChangeDispatcher;	//  108 - sink offset 020
		EventDispatcher<void>								bookReadDispatcher;		//  160 - sink offset 028
		EventDispatcher<TESCellAttachDetachEvent>			unk1B8;					//  1B8 - sink offset 030
		EventDispatcher<void>								unk210;					//  210 - sink offset 038
		EventDispatcher<void>								unk2C0;					//  2C0 - sink offset 040
		EventDispatcher<TESCombatEvent>						combatDispatcher;		//  318 - sink offset 048
		EventDispatcher<TESContainerChangedEvent>			unk370;					//  370 - sink offset 050
		EventDispatcher<TESDeathEvent>						deathDispatcher;		//  3C8 - sink offset 058
		EventDispatcher<void>								unk420;					//  420 - sink offset 068
		EventDispatcher<void>								bleedoutDispatcher;		//  478 - sink offset 070
		EventDispatcher<void>								unk4D0;					//  4D0 - sink offset 078
		EventDispatcher<void>								unk528;					//  528 - sink offset 080
		EventDispatcher<void>								unk580;					//  580 - sink offset 088
		EventDispatcher<void>								unk5D8;					//  5D8 - sink offset 090
		EventDispatcher<void>								unk630;					//  630 - sink offset 098
		EventDispatcher<TESInitScriptEvent>					initScriptDispatcher;	//  688 - sink offset 0A0
		EventDispatcher<void>								unk6E0;					//  6E0 - sink offset 0A8
		EventDispatcher<void>								unk738;					//  738 - sink offset 0B0
		EventDispatcher<void>								unk790;					//  790 - sink offset 0B8
		EventDispatcher<void>								unk7E8;					//  7E8 - sink offset 0C0
		EventDispatcher<void>								unk840;					//  840 - sink offset 0C8
		EventDispatcher<TESObjectLoadedEvent>				objectLoadedDispatcher;	//  898 - sink offset 0D0
		EventDispatcher<void>								unk8F0;					//  8F0 - sink offset 0D8
		EventDispatcher<void>								unk948;					//  948 - sink offset 0E0
		EventDispatcher<void>								unk9A0;					//  9A0 - sink offset 0E8
		EventDispatcher<void>								unk9F8;					//  9F8 - sink offset 0F0
		EventDispatcher<void>								unkA50;					//  A50 - sink offset 0F8
		EventDispatcher<void>								unkAA8;					//  AA8 - sink offset 100
		EventDispatcher<void>								unkB00;					//  B00 - sink offset 108
		EventDispatcher<void>								unkB58;					//  B58 - sink offset 110
		EventDispatcher<void>								unkBB0;					//  BB0 - sink offset 118
		EventDispatcher<void>								unkC08;					//  C08 - sink offset 120
		EventDispatcher<void>								unkC60;					//  C60 - sink offset 128
		EventDispatcher<void>								unkCB8;					//  CB8 - sink offset 130
		EventDispatcher<void>								unkD10;					//  D10 - sink offset 138
		EventDispatcher<void>								unkD68;					//  D68 - sink offset 140
		EventDispatcher<void>								unkDC0;					//  DC0 - sink offset 148
		EventDispatcher<void>								unkE18;					//  E18 - sink offset 150
		EventDispatcher<void>								spellCastDispatcher;	//  E70 - sink offset 158
		EventDispatcher<void>								playerBowShotDispatcher;//  EC8 - sink offset 160
		EventDispatcher<void>								unkF20;					//  F20 - sink offset 168
		EventDispatcher<void>								unkF78;					//  F78 - sink offset 170
		EventDispatcher<void>								unkFD0;					//  FD0 - sink offset 178
		EventDispatcher<void>								unk1028;				// 1028 - sink offset 180
		EventDispatcher<void>								unk1080;				// 1080 - sink offset 188
		EventDispatcher<void>								unk10D8;				// 10D8 - sink offset 190
		EventDispatcher<void>								unk1130;				// 1130 - sink offset 198
		EventDispatcher<void>								unk1188;				// 1188 - sink offset 200
		EventDispatcher<void>								unk11E0;				// 11E0 - sink offset 208
		EventDispatcher<void>								unk1238;				// 1238 - sink offset 210
		EventDispatcher<TESUniqueIDChangeEvent>				uniqueIdChangeDispatcher;	// 1290 - sink offset 218

		static EventDispatcherList * GetEventDispatcherList();
	};
}