#pragma once

#include "ObjectRef.h"
#include "Menus.h"

namespace Tralala
{
	class LockOnMenu : 
		public IMenu
	{
		static LockOnMenu* m_instance;

		GFxValue m_root;
		BSFixedString m_menuName;
		TESObjectREFR* m_refTarget;
		PlayerInputHandler* m_lookHandler;
		bool m_isLockOn;

		LockOnMenu();
		~LockOnMenu();
		void OnMenuOpen();
		void OnMenuClose();

		UInt32 ProcessMessage(UIMessage* message) override;
	public:
		LockOnMenu(const LockOnMenu&) = delete;
		LockOnMenu& operator=(const LockOnMenu&) = delete;
		
		void Open(TESObjectREFR * target);
		void Close();
		bool SetTargetPosition(GFxValue* args);
		
		bool IsOpen() const
		{
			return m_isLockOn;
		}

		TESObjectREFR* GetTarget() const
		{
			return m_refTarget;
		}

		// thread-unsafe but works fine
		static IMenu* GetSingleton()
		{
			if (!m_instance)
				m_instance = new LockOnMenu;

			return m_instance;
		}
	};
}
