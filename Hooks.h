#pragma once

#include "ObjectRef.h"
#include "Event.h"
#include "Controls.h"

namespace Tralala
{
	struct ThumbstickInfo
	{
		float x;
		float y;
	};

	class LockOnInputEventHandler :
		public BSTEventSink <InputEvent>
	{
	public:
		EventResult ReceiveEvent(InputEvent** evns, InputEventDispatcher* dispatcher) override;
	};

	class LockOnDeathEventHandler :
		public BSTEventSink <TESDeathEvent>
	{
	public:
		EventResult ReceiveEvent(TESDeathEvent* evn, EventDispatcher<TESDeathEvent>* dispatcher) override;
	};

	class LockOnMenuEventHandler :
		public BSTEventSink <MenuOpenCloseEvent>
	{
	public:
		EventResult ReceiveEvent(MenuOpenCloseEvent* evn, EventDispatcher<MenuOpenCloseEvent>* dispatcher) override;
	};
}

namespace MainHooks
{
	void GetAddresses();
	void Install();
}