#include <skse64/NiNodes.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include <vector>

#include "Utils.h"
#include "Hooks.h"
#include "Camera.h"
#include "Menus.h"
#include "ScaleformUtil.h"
#include "Scaleform.h"
#include "Settings.h"

namespace Tralala
{

	/* Camera Hooking */
	/* ============== */

	const float cameraSpeed = 50.0;

	ThumbstickInfo leftThumbstick = { 0, 0 };
	ThumbstickInfo rightThumbstick = { 0, 0 };

	static void CalcAngle(NiPoint3* targetPos, AngleZX* angle)
	{
		PlayerCharacter* player = PlayerCharacter::GetSingleton();
		PlayerCamera* camera = PlayerCamera::GetSingleton();
		AngleZX baseAngle;
		NiPoint3 cameraPos;

		camera->GetCameraPos(&cameraPos);
		GetAngle(cameraPos, *targetPos, &baseAngle);

		double angleDiffZ = baseAngle.z - (double)player->rot.z;
		double angleDiffX = baseAngle.x - (double)player->rot.x;

		InputEventDispatcher * g_inputEventDispatcher = InputEventDispatcher::GetSingleton();

		if (g_inputEventDispatcher && g_inputEventDispatcher->IsGamepadEnabled())
		{
			float angle = atan2(128.0, baseAngle.distance);
			angleDiffZ += rightThumbstick.x * angle;

			if (*(bool*)g_bInvertYValuesAddr){
				angleDiffX += rightThumbstick.y * angle;
			}
			else{
				angleDiffX -= rightThumbstick.y * angle;
			}
		}

		angle->z = NormalRelativeAngle(angleDiffZ);
		angle->x = NormalRelativeAngle(angleDiffX);
		angle->distance = baseAngle.distance;
	}


	static void RotateCamera(AngleZX* angle)
	{
		if (TESCameraController::GetSingleton()->unk1C == 0)
			return;

		PlayerCamera* camera = PlayerCamera::GetSingleton();
		PlayerCharacter* player = PlayerCharacter::GetSingleton();

		double angleZ = NormalAbsoluteAngle((double)player->rot.z + angle->z / (cameraSpeed * 60 / 2000));
		double angleX = NormalRelativeAngle((double)player->rot.x + angle->x / (cameraSpeed * 60 / 2000));

		if (player->IsNotInFurniture())
		{
			if (camera->IsCameraThirdPerson())
			{
				ThirdPersonState* tps = camera->GetThirdPersonCamera();
				tps->diffRotZ = 0.0;
				tps->diffRotX = 0.0;
			}
			player->SetAngleZ(static_cast<float>(angleZ));
			player->SetAngleX(static_cast<float>(angleX));
		}
		else
		{	// Just ignore these, should be deleted
			if (camera->IsCameraFirstPerson())
			{
				FirstPersonState* fps = camera->GetFirstPersonCamera();
				angleZ -= player->GetActorRotationZ(0);

				fps->sittingRot = angleZ;
				player->SetAngleX(static_cast<float>(angleX));
			}
			else if (camera->IsCameraThirdPerson())
			{
				ThirdPersonState* tps = camera->GetThirdPersonCamera();
				angleZ -= camera->camRotZ;

				tps->diffRotZ = angleZ;
				tps->diffRotX = 0;
			}
			else
			{
				player->SetAngleZ(static_cast<float>(angleZ));
				player->SetAngleX(static_cast<float>(angleX));
			}
			//
		}
	}


	static void OnCameraMove()
	{
		BSFixedString dialogueMenu("Dialogue Menu");

		MenuManager* mm = MenuManager::GetSingleton();
		if (!mm || mm->IsMenuOpen(&dialogueMenu))
		{
			dialogueMenu.Release();
			return;
		}

		dialogueMenu.Release();

		LockOnMenu* lockOnMenu = (LockOnMenu*)LockOnMenu::GetSingleton();
		TESObjectREFR* refTarget = lockOnMenu->GetTarget();

		if (!refTarget)
			return;

		if (!refTarget->GetNiNode())
		{
			lockOnMenu->Close();

			return;
		}

		if (refTarget->IsDead(1))
		{
			lockOnMenu->Close();

			return;
		}

		if (PlayerCharacter::GetSingleton()->IsInKillMove())
		{
			lockOnMenu->Close();

			return;
		}

		NiPoint3 cameraPos, cameraAngle, targetPos;

		Actor* actor = (Actor*)refTarget;

		if (actor->GetTargetPos(&targetPos))
		{
			GFxMovieView* view = lockOnMenu->view;

			AngleZX targetAngle;

			PlayerCamera * camera = PlayerCamera::GetSingleton();
			double fov = 1 / tan(camera->worldFOV * M_PI / 360.0);

			camera->GetCameraPos(&cameraPos);
			camera->GetCameraAngle(&cameraAngle);

			GetAngle(cameraPos, targetPos, &targetAngle);


			targetAngle.z = NormalRelativeAngle(targetAngle.z - cameraAngle.z);
			targetAngle.x = NormalRelativeAngle(targetAngle.x - cameraAngle.x);

			double distance = targetAngle.distance * cos(targetAngle.z) * cos(targetAngle.x);
			double x = targetAngle.distance * sin(targetAngle.z);
			double y = targetAngle.distance * sin(targetAngle.x);

			if (abs(cameraAngle.y) > 0)
			{
				double x2 = x * cos(cameraAngle.y) - y * sin(cameraAngle.y);
				double y2 = y * cos(cameraAngle.y) + x * sin(cameraAngle.y);
				x = x2;
				y = y2;
			}

			static UInt32 screenWidth = 0;
			static UInt32 screenHeight = 0;
			if (screenHeight == 0)
			{
				screenWidth = *(UInt32*)g_iSizeWDisplayAddr;
				screenHeight = *(UInt32*)g_iSizeHDisplayAddr;
			}

			double g_targetPosX = x / distance * fov * 800.0 / screenWidth * screenHeight;
			double g_targetPosY = y / distance * fov * 480.0;
			double g_targetDist = targetAngle.distance;

			if (view)
			{
				GFxValue args[3];
				args[0].SetNumber(g_targetPosX);
				args[1].SetNumber(g_targetPosY);
				args[2].SetNumber(g_targetDist);
				lockOnMenu->SetTargetPosition(args);

				if (!view->GetVisible())
					view->SetVisible(true);
			}

			CalcAngle(&targetPos, &targetAngle);
			RotateCamera(&targetAngle);
		}
	}

	/* Events Hooking */
	/* ============== */

	EventResult LockOnInputEventHandler::ReceiveEvent(InputEvent** evns, InputEventDispatcher* dispatcher)
	{
		if (!*evns)
			return kEvent_Continue;

		for (InputEvent* e = *evns; e; e = e->next)
		{
			if (e->eventType == InputEvent::kEventType_Button)
			{
				ButtonEvent* t = (ButtonEvent*)e;

				UInt32	keyCode;
				UInt32	deviceType = t->deviceType;
				UInt32	keyMask = t->keyMask;

				// Mouse
				if (deviceType == kDeviceType_Mouse)
					keyCode = InputMap::kMacro_MouseButtonOffset + keyMask;
				// Gamepad
				else if (deviceType == kDeviceType_Gamepad)
					keyCode = InputMap::GamepadMaskToKeycode(keyMask);
				// Keyboard
				else
					keyCode = keyMask;

				// Valid scancode?
				if (keyCode >= InputMap::kMaxMacros)
					continue;

				float			timer = t->timer;

				bool isDown = t->flags != 0 && timer == 0.0;

				if ((keyCode == Settings::uHotkey) && isDown)
				{
					BSFixedString dialog("Dialogue Menu");

					MenuManager* mm = MenuManager::GetSingleton();
					InputManager* im = InputManager::GetSingleton();

					PlayerCamera* playerCamera = PlayerCamera::GetSingleton();
					if (playerCamera->IsCameraFree())
						continue;

					if (im->IsTextInputAllowed() || mm->IsInMenuMode() || mm->IsMenuOpen(&dialog))
						continue;

					dialog.Release();

					LockOnMenu* lockOnMenu = (LockOnMenu*)LockOnMenu::GetSingleton();

					if (!lockOnMenu->IsOpen())
					{
						Actor* akRef = nullptr;

						PlayerCharacter* player = PlayerCharacter::GetSingleton();

						if (player->IsOnMount() || player->IsInKillMove() || player->actorState.IsSittingOrSleeping())
							continue;

						float fovThreshold = playerCamera->worldFOV / 180.0 * M_PI / 2;

						UnkCellInfo** unkCellInfo = UnkCellInfo::GetSingleton();

						tArray<UInt32>* actorHandles = &(*unkCellInfo)->actorHandles;
						if (actorHandles->count == 0)
							continue;

						std::vector<std::pair<double, Actor*>> vec;
						vec.reserve(actorHandles->count);

						NiPoint3 camPos;
						playerCamera->GetCameraPos(&camPos);

						UInt32 handle;
						size_t i = 0;
						while (actorHandles->GetNthItem(i++, handle))
						{
							TESObjectREFR* ref = nullptr;
							if (handle != InvalidRefHandle())
								LookupRefByHandle(&handle, &ref);

							if (ref && ref->formType == kFormType_Character && ref->GetNiNode() && !ref->IsDead(1) && !ref->IsChild())
							{
								UInt8 result = 0;
								if (!(player->HasLOS(ref, &result)))
								{
									if (ref)
										ref->handleRefObject.DecRef();

									continue;
								}

								Actor* actor = (Actor*)ref;

								if (Settings::bEssentialActors && actor->IsEssential())
								{
									if (ref)
										ref->handleRefObject.DecRef();

									continue;
								}

								if (Settings::bHostileActors && !(player->IsHostileToActor(actor)))
								{
									if (ref)
										ref->handleRefObject.DecRef();

									continue;
								}

								NiPoint3 pos;
								actor->GetTargetPos(&pos);
								float dx = pos.x - camPos.x;
								float dy = pos.y - camPos.y;
								float dz = pos.z - camPos.z;
								float dd = sqrt(dx * dx + dy * dy + dz * dz);

								if (Settings::uMaxDistance < 2048)
									Settings::uMaxDistance = 2048;

								if (dd <= Settings::uMaxDistance)
								{
									float point;
									NiPoint3 cameraAngle;
									playerCamera->GetCameraAngle(&cameraAngle);
									float angleZ = NormalRelativeAngle(atan2(dx, dy) - cameraAngle.z);
									float angleX = NormalRelativeAngle(atan2(-dz, sqrt(dx * dx + dy * dy)) - cameraAngle.x);

									if (abs(angleZ) < fovThreshold)
									{
										point = sqrt(angleZ * angleZ + angleX * angleX); //crosshair

										if (point >= 0)
										{
											vec.push_back(std::make_pair(point, actor));
										}
									}
								}
							}

							if (ref)
								ref->handleRefObject.DecRef();
						}

						if (vec.size() == 0)
							continue;

						std::sort(vec.begin(), vec.end());

						akRef = vec.front().second;

						if (akRef)
							lockOnMenu->Open(akRef);
						else
							lockOnMenu->Close();
					}
					else
					{
						lockOnMenu->Close();
					}
				}
			}
		}

		return kEvent_Continue;
	};


	EventResult LockOnDeathEventHandler::ReceiveEvent(TESDeathEvent* evn, EventDispatcher<TESDeathEvent>* dispatcher)
	{
		if (evn && evn->source == PlayerCharacter::GetSingleton())
			((LockOnMenu*)LockOnMenu::GetSingleton())->Close();

		return kEvent_Continue;
	};

	EventResult LockOnMenuEventHandler::ReceiveEvent(MenuOpenCloseEvent* evn, EventDispatcher<MenuOpenCloseEvent>* dispatcher)
	{
		LockOnMenu* lockOnMenu = (LockOnMenu*)LockOnMenu::GetSingleton();

		if (lockOnMenu->IsOpen())
		{
			UIStringHolder* holder = UIStringHolder::GetSingleton();
			MenuManager* mm = MenuManager::GetSingleton();
			IMenu* menu = mm->GetMenu(&evn->menuName);
			GFxMovieView* view = lockOnMenu->view;

			if (evn->opening)
			{
				if (menu)
				{
					if ((menu->flags & 0x4001) != 0)
					{
						view->SetVisible(false);
					}
				}
			}
			else if (!evn->opening)
			{
				MenuTopicManager* mtm = MenuTopicManager::GetSingleton();

				if (!mtm->isInDialogueState && mm->GetNumPauseGame() == 0 && !view->GetVisible())
				{
					view->SetVisible(true);
				}

				if (evn->menuName == holder->loadingMenu && !mm->IsMenuOpen(&holder->mainMenu))
				{
					lockOnMenu->Close();
				}
			}
		}

		return kEvent_Continue;
	};

	LockOnInputEventHandler		g_inputEventHandler;
	LockOnDeathEventHandler		g_deathEventHandler;
	LockOnMenuEventHandler		g_menuEventHandler;

	static void InstallEventsHooks(PlayerControls* playerControls)
	{
		*(PlayerControls * *)g_playerControlsAddr = playerControls;

		InputEventDispatcher* inputEventDispatcher = InputEventDispatcher::GetSingleton();
		if (inputEventDispatcher)
			inputEventDispatcher->AddEventSinkEx(&g_inputEventHandler);

		EventDispatcherList::GetEventDispatcherList()->deathDispatcher.AddEventSinkEx(&g_deathEventHandler);

		MenuManager* mm = MenuManager::GetSingleton();
		if (mm)
		{
			mm->Register("LockOnMenu", LockOnMenu::GetSingleton);
			mm->GetMenuOpenCloseEventDispatcher()->AddEventSinkEx(&g_menuEventHandler);
		}
	}

	static void RemoveEventsHooks()
	{
		InputEventDispatcher* inputEventDispatcher = InputEventDispatcher::GetSingleton();
		if (inputEventDispatcher)
			inputEventDispatcher->RemoveEventSinkEx(&g_inputEventHandler);

		EventDispatcherList::GetEventDispatcherList()->deathDispatcher.RemoveEventSinkEx(&g_deathEventHandler);

		MenuManager* mm = MenuManager::GetSingleton();
		if (mm)
			mm->GetMenuOpenCloseEventDispatcher()->RemoveEventSinkEx(&g_menuEventHandler);
	}
}


#include <skse64_common/BranchTrampoline.h>
#include <skse64_common/SafeWrite.h>
#include <xbyak/xbyak.h>

namespace MainHooks
{
	uintptr_t g_cameraHookAddr = 0;
	uintptr_t g_eventHookAddr = 0;
	uintptr_t g_removeEventAddr = 0;

	void GetAddresses()
	{
		const std::array<BYTE, 11> campattern = { 0x4C, 0x8B, 0xC3, 0x48, 0x8B, 0xD7, 0xB9, 0x14, 0x00, 0x00, 0x00 };
		g_cameraHookAddr = (uintptr_t)scan_memory(campattern, 0x64, false);

		const std::array<BYTE, 7> eventpattern = { 0x43, 0x8B, 0x1C, 0x34, 0x89, 0x5D, 0xCF };
		g_eventHookAddr = (uintptr_t)scan_memory(eventpattern, 0xC6, true);

		const std::array<BYTE, 8> removepattern = { 0x44, 0x89, 0x75, 0x10, 0x4C, 0x39, 0x76, 0x70 };
		g_removeEventAddr = (uintptr_t)scan_memory(removepattern, 0x52, false);
	}

	void Install()
	{
		{
			struct InstallHookOnCameraMove_Code : Xbyak::CodeGenerator
			{
				InstallHookOnCameraMove_Code(void* buf, UInt64 funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retn1Label;
					Xbyak::Label retn2Label;
					Xbyak::Label funcLabel;
					Xbyak::Label jumpLabel;

					push(rax);
					sub(rsp, 0x28);

					call(ptr[rip + funcLabel]);

					add(rsp, 0x28);
					pop(rax);

					mov(rbx, rax);
					test(rax, rax);
					je(jumpLabel);
					jmp(ptr[rip + retn1Label]);

					L(funcLabel);
					dq(funcAddr);

					L(jumpLabel);
					jmp(ptr[rip + retn2Label]);

					L(retn1Label);
					dq(g_cameraHookAddr + 0x6);

					L(retn2Label);
					dq(g_cameraHookAddr + 0x91);
				}
			};

			void* codeBuf = g_localTrampoline.StartAlloc();
			InstallHookOnCameraMove_Code code(codeBuf, GetFnAddr(Tralala::OnCameraMove));
			g_localTrampoline.EndAlloc(code.getCurr());

			g_branchTrampoline.Write6Branch(g_cameraHookAddr, uintptr_t(code.getCode()));
		}
		
		{
			struct InstallEventsHook_Code : Xbyak::CodeGenerator 
			{
				InstallEventsHook_Code(void* buf, UInt64 funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;
					Xbyak::Label funcLabel;

					push(rcx);
					push(rax);
					sub(rsp, 0x20);

					mov(rcx, rax);
					call(ptr[rip + funcLabel]);

					add(rsp, 0x20);
					pop(rax);
					pop(rcx);

					jmp(ptr[rip + retnLabel]);

					L(funcLabel);
					dq(funcAddr);

					L(retnLabel);
					dq(g_eventHookAddr + 0x6);
				}
			};

			void* codeBuf = g_localTrampoline.StartAlloc();
			InstallEventsHook_Code code(codeBuf, GetFnAddr(Tralala::InstallEventsHooks));
			g_localTrampoline.EndAlloc(code.getCurr());

			g_branchTrampoline.Write6Branch(g_eventHookAddr, uintptr_t(code.getCode()));

			SafeWrite8(g_eventHookAddr + 0x6, 0x90);
		}

		{
			struct UninstallEventsHook_Code : Xbyak::CodeGenerator
			{
				UninstallEventsHook_Code(void* buf, UInt64 funcAddr) : Xbyak::CodeGenerator(4096, buf)
				{
					Xbyak::Label retnLabel;
					Xbyak::Label funcLabel;

					push(rcx);
					sub(rsp, 0x28);

					call(ptr[rip + funcLabel]);

					add(rsp, 0x28);
					pop(rcx);

					mov(qword[rsp + 0x70], rsi);

					jmp(ptr[rip + retnLabel]);

					L(funcLabel);
					dq(funcAddr);

					L(retnLabel);
					dq(g_removeEventAddr + 0x5);
				}
			};

			void* codeBuf = g_localTrampoline.StartAlloc();
			UninstallEventsHook_Code code(codeBuf, GetFnAddr(Tralala::RemoveEventsHooks));
			g_localTrampoline.EndAlloc(code.getCurr());

			g_branchTrampoline.Write5Branch(g_removeEventAddr, uintptr_t(code.getCode()));
		}
	}
}