#pragma once

#include "skse64/GameMenus.h"
#include "Event.h"
#include "Utils.h"
#include "ScaleformUtil.h"

namespace Tralala
{

	void MenusGetAddresses();

	//10
	class IUIMessageData
	{
	public:
		virtual ~IUIMessageData();

		UInt8	unk08;		// 08
		UInt8	unk09;		// 09
		UInt8	pad0A[6];	// 0A
	};

	class BSUIScaleformData : public IUIMessageData
	{
	public:
		virtual ~BSUIScaleformData() {}

		void* event;		// 10
	};

	// 20
	// ### pool added in 1.3 (or maybe 1.2)
	class UIMessage
	{
	public:
		enum
		{
			kMessage_Refresh = 0,				// 0 used after ShowAllMapMarkers
			kMessage_Open,						// 1
			kMessage_PreviouslyKnownAsClose,	// 2
			kMessage_Close,						// 3
			kMessage_Unk04,
			kMessage_Unk05,
			kMessage_Scaleform,					// 6 BSUIScaleformData
			kMessage_Message					// 7 BSUIMessageData
		};

		BSFixedString		strData;	// 00
		UInt32				message;	// 08
		UInt32				pad0C;		// 0C
		void				* objData;	// 10 - something with a virtual destructor
		UInt8				isPooled;	// 18
		UInt8				pad19[7];	// 19
	};

	//// menu implementations

	// 30
	class IMenu : public FxDelegateHandler
	{
	public:
		IMenu();
		virtual ~IMenu();

		enum {
			kType_PauseGame = 1,
			kType_ShowCursor = 2
		};

		virtual void	Accept(void * processor) {};
		virtual void	Unk_02(void) {}
		virtual void	Unk_03(void) {}
		virtual UInt32	ProcessMessage(UIMessage* message);
		virtual void	Unk_05(void) {}
		virtual void	Render(void);
		virtual void	Unk_07(void) {}
		virtual void	Unk_08(void) {}

		GFxMovieView	* view;		// 10 - init'd to 0, a class, virtual fn 0x228 called in dtor
		UInt8			unk0C;		// 18 - init'd to 3
		UInt8			pad19[3];	// 19
		UInt32			flags;		// 1C - init'd to 0
		UInt32			unk14;		// 20 - init'd to 0x12
		UInt32			pad24;		// 24 
		GRefCountBase	* unk18;	// 28 - holds a reference
	};
	STATIC_ASSERT(offsetof(IMenu, view) == 0x10);


	// 018
	class MenuTableItem
	{
	public:
		BSFixedString	name;				// 000
		IMenu			* menuInstance;		// 008	0 if the menu is not currently open
		void			* menuConstructor;	// 010

		bool operator==(const MenuTableItem & rhs) const { return name == rhs.name; }
		bool operator==(const BSFixedString a_name) const { return name == a_name; }
		operator UInt64() const { return (UInt64)name.data; }

		static UInt32 GetHash(BSFixedString * key);
	};

	// 1C8
	class MenuManager
	{
		typedef tHashSet<MenuTableItem, BSFixedString> MenuTable;

		// 030-040
		struct Unknown3
		{
			UInt64		unk00;		// 000 - New in SE. Not init'd?

			UInt64		frequency;	// 008 

			UInt64		unk_010;	// 010 (= 0)
			UInt32		unk_018;	// 018 (= 0)
			UInt32		unk_01C;	// 01C (= 0)
			UInt32		unk_020;	// 020 (= 0)
			UInt32		unk_024;	// 024 (= 0)
			float		unk_028;	// 028 (= frequency related)

			UInt32		unk_02C;	// 02C
			UInt32		unk_030;	// 030

			UInt32		unk_034;	// 034 (= 0)
			UInt16		unk_038;	// 038
			UInt8		unk_03A;	// 03A (= 0)
			UInt8		pad[5];		// 03B
		};
		STATIC_ASSERT(sizeof(Unknown3) == 0x40);

	private:
		UInt64					unk_000;	// 000

		EventDispatcher<MenuOpenCloseEvent>		menuOpenCloseEventDispatcher;	// 008
		EventDispatcher<MenuModeChangeEvent>	menuModeChangeEventDispatcher;	// 060
		EventDispatcher<void*>					unk_064;						// 0B8 - New in 1.6.87.0 - Kinect related?

		UnkArray			menuStack;					// 110
		MenuTable			menuTable;					// 128   (Entries ptr at 150)
		SimpleLock			menuTableLock;				// 158
		UInt32				numPauseGame;				// 160 (= 0) += 1 if (imenu->flags & 0x0001)
		UInt32				numItemMenu;				// 164 (= 0) += 1 if (imenu->flags & 0x2000)
		UInt32				numPreventGameLoad;			// 168 (= 0) += 1 if (imenu->flags & 0x0080)
		UInt32				numDoNotPreventSaveGame;	// 16C (= 0) += 1 if (imenu->flags & 0x0800)
		UInt32				numStopCrosshairUpdate;		// 170 (= 0) += 1 if (imenu->flags & 0x4000)
		UInt32				numFlag8000;				// 174 (= 0) += 1 if (imenu->flags & 0x8000)
		UInt32				numFlag20000;				// 178 (= 0)  = 1 if (imenu->flags & 0x20000)
		UInt8				numModal;					// 17C (= 0)  = 1 if (imenu->flags & 0x10)
		UInt8				pad_17D[3];					// 17D
		Unknown3			unk_180;					// 180
		bool				showMenus;					// 1C0 (= 0)
		bool				unk_1C1;					// 1C1 (= 0)
		char				pad[6];						// 1C2

	public:
		static MenuManager * GetSingleton();

		EventDispatcher<MenuOpenCloseEvent> * GetMenuOpenCloseEventDispatcher()
		{
			return &menuOpenCloseEventDispatcher;
		}

		bool				IsMenuOpen(BSFixedString * menuName);
		bool				IsInMenuMode();
		IMenu *				GetMenu(BSFixedString * menuName);
		GFxMovieView *		GetMovieView(BSFixedString * menuName);
		void				ShowMenus(bool show) { showMenus = show; }
		bool				IsShowingMenus() const { return showMenus; }

		typedef IMenu*	(*CreatorFunc)(void);
		void Register(const char* name, CreatorFunc creator);
		UInt32 GetNumPauseGame()
		{
			return numPauseGame;
		}
		UInt32 GetNumStopCrosshairUpdate()
		{
			return numStopCrosshairUpdate;
		}
	};
	STATIC_ASSERT(sizeof(MenuManager) == 0x1C8);

	// B80
	class UIManager
	{
	public:

		UInt8	unk00[0xB80];

		static UIManager *	GetSingleton();

		void AddMessage(StringCache::Ref * strData, UInt32 msgID, void * objData);
	};
	STATIC_ASSERT(sizeof(UIManager) == 0xB80);

	// 260
	class UIStringHolder
	{
	public:
		void			* unk00;					// 000
		BSFixedString	faderData;					// 008 "FaderData"
		BSFixedString	hudData;					// 010 "HUDData"
		BSFixedString	hudCamData;					// 018 "HUDCamData"
		BSFixedString	floatingQuestMarkers;		// 020 "FloatingQuestMarkers"
		BSFixedString	consoleData;				// 028 "ConsoleData"
		BSFixedString	quantityData;				// 030 "QuantityData"
		BSFixedString	messageBoxData;				// 038 "MessageBoxData"
		BSFixedString	bsUIScaleformData;			// 040 "BSUIScaleformData"
		BSFixedString	bsUIMessageData;			// 048 "BSUIMessageData"
		BSFixedString	bsUIAnalogData;				// 050 "BSUIAnalogData"
		BSFixedString	inventoryUpdateData;		// 058 "InventoryUpdateData"
		BSFixedString	refHandleUIData;			// 060 "RefHandleUIData"
		BSFixedString	tesFormUIData;				// 068 "TESFormUIData"
		BSFixedString	loadingMenuData;			// 070 "LoadingMenuData"
		BSFixedString	kinectStateData;			// 078 "KinectStateChangeData"
		BSFixedString	kinectUserEventData;		// 080 "KinectUserEventData"
		BSFixedString	inventoryMenu;				// 088 "InventoryMenu"
		BSFixedString	console;					// 090 "Console"
		BSFixedString	dialogueMenu;				// 098 "Dialogue Menu"
		BSFixedString	hudMenu;					// 0A0 "HUD Menu"
		BSFixedString	mainMenu;					// 0A8 "Main Menu"
		BSFixedString	messageBoxMenu;				// 0B0 "MessageBoxMenu"
		BSFixedString	cursorMenu;					// 0B8 "Cursor Menu"
		BSFixedString	faderMenu;					// 0C0 "Fader Menu"
		BSFixedString	magicMenu;					// 0C8 "MagicMenu"
		BSFixedString	topMenu;					// 0D0 "Top Menu"
		BSFixedString	overlayMenu;				// 0D8 "Overlay Menu"
		BSFixedString	overlayInteractionMenu;		// 0E0 "Overlay Interaction Menu"
		BSFixedString	loadingMenu;				// 0E8 "Loading Menu"
		BSFixedString	tweenMenu;					// 0F0 "TweenMenu"
		BSFixedString	barterMenu;					// 0F8 "BarterMenu"
		BSFixedString	giftMenu;					// 100 "GiftMenu"
		BSFixedString	debugTextMenu;				// 108 "Debug Text Menu"
		BSFixedString	mapMenu;					// 110 "MapMenu"
		BSFixedString	lockpickingMenu;			// 118 "Lockpicking Menu"
		BSFixedString	quantityMenu;				// 120 "Quantity Menu"
		BSFixedString	statsMenu;					// 128 "StatsMenu"
		BSFixedString	containerMenu;				// 130 "ContainerMenu"
		BSFixedString	sleepWaitMenu;				// 138 "Sleep/Wait Menu"
		BSFixedString	levelUpMenu;				// 140 "LevelUp Menu"
		BSFixedString	journalMenu;				// 148 "Journal Menu"
		BSFixedString	bookMenu;					// 150 "Book Menu"
		BSFixedString	favoritesMenu;				// 158 "FavoritesMenu"
		BSFixedString	raceSexMenu;				// 160 "RaceSex Menu"
		BSFixedString	craftingMenu;				// 168 "Crafting Menu"
		BSFixedString	trainingMenu;				// 170 "Training Menu"
		BSFixedString	mistMenu;					// 178 "Mist Menu"
		BSFixedString	tutorialMenu;				// 180 "Tutorial Menu"
		BSFixedString	creditsMenu;				// 188 "Credits Menu"
		BSFixedString	modManagerMenu;				// 190 "Mod Manager Menu"		- NEW IN SE
		BSFixedString	creationClubMenu;			// 198 "Creation Club Menu"		- NEW IN SE
		BSFixedString	titleSequenceMenu;			// 1A0 "TitleSequence Menu"
		BSFixedString	consoleNativeUIMenu;		// 1A8 "Console Native UI Menu"
		BSFixedString	kinectMenu;					// 1B0 "Kinect Menu"
		BSFixedString	loadWaitSpinner;			// 1B8 "LoadWaitSpinner"		- NEW IN SE
		BSFixedString	streamingInstallMenu;		// 1C0	"StreamingInstallMenu"  - NEW IN SE
		BSFixedString	textWidget;					// 1C8 "TextWidget"
		BSFixedString	buttonBarWidget;			// 1D0 "ButtonBarWidget"
		BSFixedString	graphWidget;				// 1D8 "GraphWidget"
		BSFixedString	textureWidget;				// 1E0 "TextWidget"
		BSFixedString	uiMenuOK;					// 1E8 "UIMenuOK"
		BSFixedString	uiMenuCancel;				// 1F0 "UIMenuCancel"
		BSFixedString	showText;					// 1F8 "Show Text"
		BSFixedString	hideText;					// 200 "Hide Text"
		BSFixedString	showList;					// 208 "Show List"
		BSFixedString	voiceReady;					// 210 "Voice Ready"
		BSFixedString	dmfoStr;					// 218 "DMFOStr"
		BSFixedString	showJournal;				// 220 "Show Journal"
		BSFixedString	journalSettingsSaved;		// 228 "Journal Settings Saved"
		BSFixedString	closeMenu;					// 230 "CloseMenu"
		BSFixedString	closingAllMenus;			// 238 "Closing All Menus"
		BSFixedString	refreshMenu;				// 240 "RefreshMenu"
		BSFixedString	cancelLoading;				// 248 "CancelLoading"		- NEW IN SE
		BSFixedString	menuTextureDegradeEvent;	// 250 "Menu Texture Degrade Event"
		BSFixedString	diamondMarker;				// 258 "<img src='DiamondMarker' width='10' height='15' align='baseline' vspace='5'>"

		static UIStringHolder *	GetSingleton(void);
	};
}