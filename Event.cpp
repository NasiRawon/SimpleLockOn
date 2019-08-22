#include "Event.h"
#include "PatternScanner.h"

namespace Tralala
{
	uintptr_t g_addEventAddr = 0;
	uintptr_t g_removeEventAddr = 0;
	uintptr_t g_dispatcherAddr = 0;
	uintptr_t g_dispatcherListAddr = 0;

	void EventGetAddresses()
	{ 
		const std::array<BYTE, 6> addpattern = { 0x88, 0x46, 0x49, 0x48, 0x8B, 0xC6 };
		g_addEventAddr = (uintptr_t)scan_memory_data(addpattern, 0x40, false, 0x1, 0x5);

		const std::array<BYTE, 7> rempattern = { 0x4C, 0x8B, 0x86, 0x50, 0x05, 0x03, 0x00 };
		g_removeEventAddr = (uintptr_t)scan_memory_data(rempattern, 0x6D, false, 0x1, 0x5);

		const std::array<BYTE, 6> disppattern = { 0x88, 0x46, 0x49, 0x48, 0x8B, 0xC6 };
		g_dispatcherAddr = (uintptr_t)scan_memory_data(disppattern, 0x6F, false, 0x3, 0x7);
 
		const std::array<BYTE, 6> listpattern = { 0x88, 0x46, 0x49, 0x48, 0x8B, 0xC6 };
		g_dispatcherListAddr = (uintptr_t)scan_memory_data(listpattern, 0x3B, false, 0x1, 0x5);
	}

	InputEventDispatcher * InputEventDispatcher::GetSingleton()
	{
		return *(InputEventDispatcher**)g_dispatcherAddr;
	}

	bool InputEventDispatcher::IsGamepadEnabled()
	{
		return (gamepad) && gamepad->IsEnabled();
	}

	EventDispatcherList * EventDispatcherList::GetEventDispatcherList()
	{
		typedef EventDispatcherList*(*GetEventDispatcherList_t)();
		GetEventDispatcherList_t GetEventDispatcher = (GetEventDispatcherList_t)g_dispatcherListAddr;

		return GetEventDispatcher();
	}
}