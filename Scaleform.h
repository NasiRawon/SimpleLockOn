#pragma once

#include "ObjectRef.h"
#include "Menus.h"

namespace Tralala
{
	class LockOnMenu : 
		public IMenu
	{
		BSFixedString m_menuName;
		TESObjectREFR* m_refTarget;
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
		
		bool IsOpen() const
		{
			return m_isLockOn;
		}

		TESObjectREFR* GetTarget() const
		{
			return m_refTarget;
		}

		static IMenu* GetSingleton()
		{
			static LockOnMenu instance;
			return &instance;
		}
	};
}
