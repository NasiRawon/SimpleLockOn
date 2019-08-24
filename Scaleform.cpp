#include "Scaleform.h"
#include "ScaleformUtil.h"
#include "Utils.h"
#include "Controls.h"

namespace Tralala
{
	LockOnMenu* LockOnMenu::m_instance = nullptr;

	LockOnMenu::LockOnMenu() : 
		m_menuName("LockOnMenu"),
		m_refTarget(nullptr), 
		m_isLockOn(false)
	{
		_MESSAGE("LockOnMenu ctor is called");

		if (GFxLoader::GetSingleton()->LoadMovie(this, &view, "SimpleLockOn", 0, 0.0))
		{
			flags = 0x18902;
			unk0C = 2;
			unk14 = 0x12;
		}
	}

	LockOnMenu::~LockOnMenu()
	{
		_MESSAGE("LockOnMenu dtor is called");

		m_isLockOn = false;
		m_refTarget = nullptr;
		m_menuName.Release();
	}

	void LockOnMenu::Open(TESObjectREFR* target)
	{
		PlayerControls* pControls = PlayerControls::GetSingleton();
		if (pControls) 
		{
			PlayerInputHandler* lookHandler = pControls->lookHandler;
			lookHandler->enabled = 0;
		}

		m_refTarget = target;
		m_isLockOn = true;
		
		UIManager::GetSingleton()->AddMessage(&m_menuName, UIMessage::kMessage_Open, nullptr);
	}

	void LockOnMenu::Close()
	{
		PlayerControls* pControls = PlayerControls::GetSingleton();
		if (pControls) 
		{
			PlayerInputHandler* lookHandler = pControls->lookHandler;
			lookHandler->enabled = 1;
		}

		m_refTarget = nullptr;
		m_isLockOn = false;

		UIManager::GetSingleton()->AddMessage(&m_menuName, UIMessage::kMessage_Close, nullptr);
	}

	void LockOnMenu::OnMenuOpen()
	{
		if (view)
		{
			const char * targetName = m_refTarget->GetReferenceName();
			GFxValue args[1];
			args[0].SetString(targetName);

			view->Invoke("_root.widgetHolder.setTargetName", nullptr, args, 1);
		}
	}

	void LockOnMenu::OnMenuClose()
	{
		if (view)
		{
			GFxValue args[1];
			args[0].SetString("");

			view->Invoke("_root.widgetHolder.setTargetName", nullptr, args, 1);
		}
	}

	UInt32 LockOnMenu::ProcessMessage(UIMessage * message)
	{
		UInt32 result = 2;

		if (view)
		{
			switch (message->message)
			{
			case UIMessage::kMessage_Open:
				OnMenuOpen();
				break;
			case UIMessage::kMessage_Close:
				OnMenuClose();
				break;
			}
		}

		return result;
	}
}