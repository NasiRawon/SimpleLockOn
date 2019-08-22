#pragma once

#include "skse64/GameInput.h"

namespace Tralala
{
	extern uintptr_t g_playerControlsAddr;

	void ControlsGetAddresses();

	class PlayerInputHandler;

	// 90
	class MenuControls
	{
	public:
		virtual			~MenuControls();
		virtual UInt32	Unk_01();

		//	void			** _vtbl;		// 00
		BSTEventSink<MenuModeChangeEvent> menuModeEventSink; // 08
		UInt64			unk10;			// 10
		tArray<void*>	arr18;			// 18
		UInt64			unk30[3];		// 30

		MenuEventHandler* clickHandler;	// 48
		MenuEventHandler* directionHandler;	// 50
		MenuEventHandler* consoleOpenHandler;	// 58
		MenuEventHandler* quickSaveLoadHandler;	// 60
		MenuEventHandler* menuOpenHandler;	// 68
		MenuEventHandler* favoritesHandler;	// 70
		MenuEventHandler* screenshotHandler;	// 78

		UInt8			unk80;			// 80
		bool			beastForm;			// 81
		bool			remapMode;		// 82
		UInt8			unk83;			// 83
		UInt8			pad84[0x90 - 0x84];	// 84

		static MenuControls *	GetSingleton();
	};
	STATIC_ASSERT(offsetof(MenuControls, remapMode) == 0x082);

	// 128 
	class InputManager
	{
	public:
		enum
		{
			kContext_Gameplay = 0,
			kContext_MenuMode,
			kContext_Console,
			kContext_ItemMenu,
			kContext_Inventory,
			kContext_DebugText,
			kContext_Favorites,
			kContext_Map,
			kContext_Stats,
			kContext_Cursor,
			kContext_Book,
			kContext_DebugOverlay,
			kContext_Journal,
			kContext_TFCMode,
			kContext_MapDebug,
			kContext_Lockpicking,
			kContext_Favor,
			kContextCount = 17
		};

		struct InputContext
		{
			// 18
			struct Mapping
			{
				BSFixedString	name;		// 00
				UInt32			buttonID;	// 08
				UInt32			sortIndex;	// 0C
				UInt32			unk10;		// 10
				UInt32			pad14;		// 14
			};

			tArray<Mapping>	keyboardMap;
			tArray<Mapping>	mouseMap;
			tArray<Mapping>	gamepadMap;
		};

		inline bool IsMovementControlsEnabled() const	{ return (unk118 & 0x401) == 0x401; }

		void*			unkPtr000;					// 000
		BSTEventSource<void *>	unk008;				// 008 - TODO: template type
		InputContext	* context[kContextCount];	// 060
		tArray<void*>	unk0E8;						// 0E8
		tArray<void*>	unk100;						// 100
		UInt32			unk118;						// 118 - init'd to 0xFFFFFFFF, probably flag
		UInt32			unk11C;						// 11C - init'd to 0x80000000
		bool			allowTextInput;				// 120
		UInt8			unk121;						// 121
		UInt8			unk122;						// 122
		UInt8			pad[5];						// 123

		static InputManager *	GetSingleton();

		inline bool IsTextInputAllowed()
		{
			return allowTextInput;
		}

	};
	STATIC_ASSERT(sizeof(InputManager) == 0x128);

	// 1E0
	class PlayerControls
	{
	public:
		virtual			~PlayerControls();
		virtual UInt32	Unk_01();

		struct Data024
		{
			float			unk024;			// 024
			float			unk028;			// 028
			float			unk02C;			// 02C
			float			unk030;			// 030
			float			unk034;			// 034
			float			unk038;			// 038
			float			unk03C;			// 03C
			float			unk040;			// 040
			UInt32			unk044;			// 044
			UInt8			autoRun;		// 048
			UInt8			runMode;		// 049
			UInt8			unk04A;			// 04A - related to looking handler, if you move your mouse this will be 1, as long as you're in gameplay context not menu
			bool			FOVslideMode;	// 04B
			bool			POVscriptMode;	// 04C - 0 = enable, 1 = disable
			bool			POVbeastMode;	// 04D - 0 = enable, 1 = disable
			UInt8			unk04E;			// 04E
			UInt8			unk04F;			// 04F
			bool			remapMode;		// 050 - might be named differently
			UInt8			pad51[7];		// 051
		};

		//	void			** _vtbl;		// 000
		BSTEventSink<void*> menuOpenCloseEvent;	// 008
		BSTEventSink<void*> menuModeChangeEvent;	// 010
		BSTEventSink<void*> furnitureEvent;	// 018
		UInt32			unk020;			// 020
		Data024			unk024;			// 024
		tArray<void*>	unk058;			// 058
		tArray<void*>	unk070;			// 070
		tArray<void *>	unk088;			// 088
		UInt8			unkA0[8];		// 0A0
		UInt64			unkA8;			// 0A8
		float			unkB0[8];		// B0
		UInt32			unkD0[10];		// D0
		UInt8			unkF8[8];		// F8
		float			unk100[20];		// 100
		tArray<void*>	unk150;			// 150
		UInt64			unk168;			// 168

		PlayerInputHandler*		movementHandler;	// 170
		PlayerInputHandler*		lookHandler;		// 178
		PlayerInputHandler*		sprintHandler;		// 180
		PlayerInputHandler*		readyWeaponHandler; // 188
		PlayerInputHandler*		autoMoveHandler;	// 190
		PlayerInputHandler*		toggleRunHandler;	// 198
		PlayerInputHandler*		activateHandler;	// 1A0
		PlayerInputHandler*		jumpHandler;		// 1A8
		PlayerInputHandler*		shoutHandler;		// 1B0
		PlayerInputHandler*		attackBlockHandler; // 1B8
		PlayerInputHandler*		runHandler;			// 1C0
		PlayerInputHandler*		sneakHandler;		// 1C8
		PlayerInputHandler*		togglePOVHandler;	// 1D0

		UInt8			unk1D8;			// 1D8 - 0: disable look, 1: enable look
		UInt8			pad1D9[3];		// 1D9
		UInt32			unk1DC;			// 1DC

		static PlayerControls *	GetSingleton();
	};
	STATIC_ASSERT(sizeof(PlayerControls) == 0x1E0);


	// 10
	class PlayerInputHandler
	{
	public:
		PlayerInputHandler();
		virtual ~PlayerInputHandler();

		virtual	bool	CanProcess(InputEvent* evn);
		virtual	void	ProcessThumbstick(ThumbstickEvent* evn, PlayerControls::Data024* unk24);
		virtual	void	ProcessMouseMove(MouseMoveEvent* evn, PlayerControls::Data024* unk24);
		virtual	void	ProcessButton(ButtonEvent* evn, PlayerControls::Data024* unk24);

		bool	enabled;				// 08
	};
	STATIC_ASSERT(sizeof(PlayerInputHandler) == 0x10);
}