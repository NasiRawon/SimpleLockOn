#include "Controls.h"
#include "PatternScanner.h"

namespace Tralala
{
	uintptr_t g_playerControlsAddr = 0;
	uintptr_t g_menuControlsAddr = 0;
	uintptr_t g_inputManagerAddr = 0;

	void ControlsGetAddresses()
	{
		const std::array<BYTE, 8> playerpattern = { 0x8B, 0x40, 0x10, 0xC1, 0xE8, 0x17, 0xA8, 0x01 };
		g_playerControlsAddr = (uintptr_t)scan_memory_data(playerpattern, 0x6E, true, 0x3, 0x7);

		const std::array<BYTE, 7> menupattern = { 0xC1, 0xE8, 0x04, 0xA8, 0x01, 0x74, 0x35 };
		g_menuControlsAddr = (uintptr_t)scan_memory_data(menupattern, 0x27, false, 0x3, 0x7);

		const std::array<BYTE, 8> inputpattern = { 0x48, 0x8B, 0x43, 0x60, 0xF6, 0x40, 0x12, 0x01 };
		g_inputManagerAddr = (uintptr_t)scan_memory_data(inputpattern, 0x33, false, 0x3, 0x7);
	}

	MenuControls * MenuControls::GetSingleton()
	{
		return *(MenuControls**)g_menuControlsAddr;
	}

	InputManager * InputManager::GetSingleton()
	{
		return *(InputManager**)g_inputManagerAddr;
	}

	PlayerControls * PlayerControls::GetSingleton()
	{
		return *(PlayerControls**)g_playerControlsAddr;
	}
}