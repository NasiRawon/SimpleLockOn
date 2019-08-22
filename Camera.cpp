#include "Camera.h"
#include "skse64/NiNodes.h"

namespace Tralala
{
	uintptr_t g_playerCameraAddr = 0;

	void PlayerCameraGetAddress()
	{
		const std::array<BYTE, 9> pattern = { 0x8B, 0x4D, 0xC7, 0x0F, 0xB6, 0xC9, 0x83, 0xF8, 0x04 };
		g_playerCameraAddr = (uintptr_t)scan_memory_data(pattern, 0x21, true, 0x3, 0x7);
	}

	float ThirdPersonState::GetDistanceWithinTargetHead(Tralala::Actor * target)
	{
		if (!target || !target->processManager || !target->processManager->middleProcess)
			return FLT_MAX;

		NiNode* headNode = (NiNode*)target->processManager->middleProcess->unk158;
		if (!headNode)
			return FLT_MAX;

		NiPoint3 dist;
		dist.x = camPos.x - headNode->m_worldTransform.pos.x;
		dist.y = camPos.y - headNode->m_worldTransform.pos.y;
		dist.z = camPos.z - headNode->m_worldTransform.pos.z;

		return (dist.x*dist.x + dist.y*dist.y + dist.z*dist.z);
	}

	void ThirdPersonState::SetFirstPersonSmooth(float minZoom, bool overShoulder)
	{
		if (unkDD[0] == 0)
		{
			savedZoom = curPosY;
			dstPosY = minZoom;
			unkDD[0] = 1;
		}
		else
		{
			curPosY = dstPosY;
		}

		if (!overShoulder)
			fOverShoulderPosX = fOverShoulderCombatAddY = fOverShoulderPosZ = 0;
	}

	PlayerCamera * PlayerCamera::GetSingleton()
	{
		return *(PlayerCamera**)g_playerCameraAddr;
	}

	bool PlayerCamera::IsCameraFirstPerson()
	{
		return cameraState == cameraStates[kCameraState_FirstPerson];
	}

	bool PlayerCamera::IsCameraThirdPerson()
	{
		return cameraState == cameraStates[kCameraState_ThirdPerson2];
	}

	bool PlayerCamera::IsCameraFree()
	{
		return cameraState == cameraStates[kCameraState_Free];
	}

	void PlayerCamera::GetFirstCamPos(NiPoint3* pos)
	{
		Tralala::PlayerCharacter * player = Tralala::PlayerCharacter::GetSingleton();

		pos->x = player->pos.x;
		pos->y = player->pos.y;

		FirstPersonState * fps = GetFirstPersonCamera();
		if (fps->cameraNode)
		{
			pos->z = player->pos.z + fps->cameraNode->m_localTransform.pos.z;
		}
		else
		{
			player->GetMarkerPosition(pos);
		}
	}

	void PlayerCamera::GetThirdCamPos(NiPoint3* pos)
	{
		ThirdPersonState * tps = GetThirdPersonCamera();
		tps->GetCameraPosition(pos);
	}

	void PlayerCamera::GetCameraPos(NiPoint3* pos)
	{
		PlayerCharacter* player = PlayerCharacter::GetSingleton();
		float x, y, z;

		if (IsCameraFirstPerson())
		{
			NiNode* node = cameraNode;
			if (node)
			{
				x = node->m_worldTransform.pos.x;
				y = node->m_worldTransform.pos.y;
				z = node->m_worldTransform.pos.z;
			}
			else
			{
				return GetFirstCamPos(pos);
			}
		}
		else if (IsCameraThirdPerson())
		{
			return GetThirdCamPos(pos);
		}
		else
		{
			NiPoint3 playerPos;

			player->GetMarkerPosition(&playerPos);
			z = playerPos.z;
			x = player->pos.x;
			y = player->pos.y;
		}

		pos->x = x;
		pos->y = y;
		pos->z = z;
	}

	void PlayerCamera::GetCameraAngle(NiPoint3* pos)
	{
		PlayerCharacter* player = PlayerCharacter::GetSingleton();

		if (IsCameraFirstPerson())
		{
			FirstPersonState* fps = GetFirstPersonCamera();
			NiPoint3 angle;
			ComputeAnglesFromMatrix(&fps->cameraNode->m_worldTransform.rot, &angle);
			pos->z = NormalAbsoluteAngle(-angle.z);
			pos->x = player->rot.x - angle.x;
			pos->y = angle.y;
		}
		else if (IsCameraThirdPerson())
		{
			ThirdPersonState* tps = GetThirdPersonCamera();
			NiPoint3 angle;
			pos->z = player->rot.z + tps->diffRotZ;
			pos->x = player->rot.x + tps->diffRotX;
			pos->y = 0;
		}
		else
		{
			pos->x = player->rot.x;
			pos->y = player->rot.y;
			pos->z = player->rot.z;
		}
	}
}