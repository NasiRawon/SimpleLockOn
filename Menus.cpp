#include "Menus.h"
#include "PatternScanner.h"

namespace Tralala
{
	uintptr_t g_menuManagerAddr = 0;
	uintptr_t g_uiManagerAddr = 0;
	uintptr_t g_addMessageAddr = 0;
	uintptr_t g_isMenuOpenAddr = 0;
	uintptr_t g_registerAddr = 0;
	uintptr_t g_isInMenuMode1Addr = 0;
	uintptr_t g_isInMenuMode2Addr = 0;
	uintptr_t g_crc64Addr = 0;
	uintptr_t g_menuDtorAddr = 0;
	uintptr_t g_stringHoldAddr = 0;

	void MenusGetAddresses(void)
	{
		const std::array<BYTE, 9> menuMgpattern = { 0x48, 0x8B, 0xD1, 0x41, 0xB0, 0x01, 0x49, 0x8B, 0xCA };
		g_menuManagerAddr = (uintptr_t)scan_memory_data(menuMgpattern, 0x34, false, 0x3, 0x7);

		const std::array<BYTE, 6> menuOppattern = { 0x33, 0xD2, 0x41, 0x0F, 0xB6, 0xCE };
		g_isMenuOpenAddr = (uintptr_t)scan_memory_data(menuOppattern, 0x4D, false, 0x1, 0x5);

		const std::array<BYTE, 10> regpattern = { 0x48, 0x8B, 0xC4, 0x53, 0x56, 0x57, 0x48, 0x83, 0xEC, 0x50 };
		g_registerAddr = (uintptr_t)scan_memory(regpattern, 0, true);

		const std::array<BYTE, 5> menu1pattern = { 0x0F, 0x57, 0xF6, 0xA8, 0x20 };
		g_isInMenuMode1Addr = (uintptr_t)scan_memory_data(menu1pattern, 0x7, true, 0x2, 0x6);

		const std::array<BYTE, 5> menu2pattern = { 0x0F, 0x57, 0xF6, 0xA8, 0x20 };
		g_isInMenuMode2Addr = (uintptr_t)scan_memory_data(menu2pattern, 0xF, true, 0x2, 0x6);

		const std::array<BYTE, 8> uipattern = { 0xC6, 0x43, 0x50, 0x00, 0xC6, 0x43, 0x3D, 0x01 };
		g_uiManagerAddr = (uintptr_t)scan_memory_data(uipattern, 0xAB, true, 0x3, 0x7);

		const std::array<BYTE, 8> addpattern = { 0xC6, 0x43, 0x50, 0x00, 0xC6, 0x43, 0x3D, 0x01 };
		g_addMessageAddr = (uintptr_t)scan_memory_data(addpattern, 0xB2, true, 0x1, 0x5);

		const std::array<BYTE, 8> crcpattern = { 0x49, 0xC1, 0xE8, 0x08, 0x41, 0x0F, 0xB6, 0xC2 };
		g_crc64Addr = (uintptr_t)scan_memory(crcpattern, 0xAF, false);

		const std::array<BYTE, 7> dtorpattern = { 0x4C, 0x8B, 0x86, 0x50, 0x05, 0x03, 0x00 };
		g_menuDtorAddr = (uintptr_t)scan_memory_data(dtorpattern, 0xE1, true, 0x1, 0x5);

		const std::array<BYTE, 7> strpattern = { 0x48, 0x8B, 0xF8, 0xC6, 0x47, 0x20, 0x01 };
		g_stringHoldAddr = (uintptr_t)scan_memory_data(strpattern, 0x7, true, 0x3, 0x7);
	}

	IMenu::IMenu() :
		view(nullptr),
		unk0C(3),
		flags(0),
		unk14(0x12),
		unk18(nullptr)
	{
	}

	IMenu::~IMenu()
	{
		typedef void(*IMenu_dtor_t)(IMenu*);
		IMenu_dtor_t IMenu_dtor = (IMenu_dtor_t)g_menuDtorAddr;

		IMenu_dtor(this);
	}

	UInt32 IMenu::ProcessMessage(UIMessage* message)
	{
		if (message->message == 6){
			if (view && message->objData){
				BSUIScaleformData *scaleformData = static_cast<BSUIScaleformData*>(message->objData);

				view->HandleEvent(&scaleformData->event);
				return 0;
			}
		}
		return 2;
	}

	void IMenu::Render()
	{
		if (view)
			view->Render();
	}

	UInt32 MenuTableItem::GetHash(BSFixedString * key)
	{
		typedef void(*CalculateCRC32_64_t)(UInt32 * out, UInt64 data);
		CalculateCRC32_64_t CalculateCRC32_64 = (CalculateCRC32_64_t)g_crc64Addr;

		UInt32 hash;
		CalculateCRC32_64(&hash, (UInt64)key->data);
		return hash;
	}

	MenuManager * MenuManager::GetSingleton()
	{
		return *(MenuManager**)g_menuManagerAddr;
	}

	bool MenuManager::IsMenuOpen(BSFixedString * menuName)
	{
		typedef bool(*IsMenuOpen_t)(MenuManager *, BSFixedString *);
		IsMenuOpen_t IsMenuOpen = (IsMenuOpen_t)g_isMenuOpenAddr;

		return IsMenuOpen(this, menuName);
	}

	bool MenuManager::IsInMenuMode()
	{
		return (*(bool*)g_isInMenuMode1Addr || *(bool*)g_isInMenuMode2Addr);
	}

	IMenu * MenuManager::GetMenu(BSFixedString * menuName)
	{
		if (!menuName->data)
			return nullptr;

		MenuTableItem * item = menuTable.Find(menuName);

		if (!item)
			return nullptr;

		IMenu * menu = item->menuInstance;
		if (!menu)
			return nullptr;

		return menu;
	}

	GFxMovieView * MenuManager::GetMovieView(BSFixedString * menuName)
	{
		IMenu * menu = GetMenu(menuName);
		if (!menu)
			return nullptr;

		return menu->view;
	}

	void MenuManager::Register(const char* name, CreatorFunc creator)
	{
		typedef void(*Register_t)(MenuManager*, const char*, CreatorFunc);
		Register_t Register = (Register_t)g_registerAddr;

		Register(this, name, creator);
	}

	UIManager * UIManager::GetSingleton()
	{
		return *(UIManager**)g_uiManagerAddr;
	}

	void UIManager::AddMessage(StringCache::Ref * strData, UInt32 msgID, void * objData)
	{
		typedef void(*AddMessage_t)(UIManager*, StringCache::Ref *, UInt32, void*);
		AddMessage_t AddMessage = (AddMessage_t)g_addMessageAddr;

		AddMessage(this, strData, msgID, objData);
	}

	UIStringHolder * UIStringHolder::GetSingleton()
	{
		return *(UIStringHolder**)g_stringHoldAddr;
	}
}