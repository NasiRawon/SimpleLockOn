#include "ObjectRef.h"
#include "Utils.h"
#include "Camera.h"

namespace Tralala
{
	uintptr_t g_playerCharacterAddr = 0;
	uintptr_t g_invalidRefHandleAddr = 0;
	uintptr_t g_lookupAddr = 0;
	uintptr_t g_isonmountAddr = 0;
	uintptr_t g_setAngleZAddr = 0;
	uintptr_t g_setAngleXAddr = 0;
	uintptr_t g_refNameAddr = 0;
	uintptr_t g_hasLOSAddr = 0;
	uintptr_t g_hostileAddr = 0;

	void ObjectRefGetAddresses()
	{
		const std::array<BYTE, 8> pattern = { 0x8B, 0x40, 0x10, 0xC1, 0xE8, 0x17, 0xA8, 0x01 };
		g_playerCharacterAddr = (uintptr_t)scan_memory_data(pattern, 0x23, true, 0x3, 0x7);

		const std::array<BYTE, 10> invpattern = { 0x8B, 0x0C, 0x03, 0x89, 0x4C, 0x24, 0x40, 0x48, 0x8B, 0xD6 };
		g_invalidRefHandleAddr = (uintptr_t)scan_memory_data(invpattern, 0x47, true, 0x2, 0x6);

		const std::array<BYTE, 7> lookpattern = { 0x48, 0x6B, 0xD8, 0x78, 0x48, 0x8B, 0xCE };
		g_lookupAddr = (uintptr_t)scan_memory_data(lookpattern, 0x32, true, 0x1, 0x5);

		const std::array<BYTE, 7> mountpattern = { 0x48, 0x83, 0xC2, 0x54, 0x0F, 0x28, 0xDE };
		g_isonmountAddr = (uintptr_t)scan_memory_data(mountpattern, 0x7A, false, 0x1, 0x5);

		const std::array<BYTE, 8> angleZpattern = { 0x8B, 0xC2, 0xC1, 0xE8, 0x15, 0x83, 0xE0, 0x0F };
		g_setAngleZAddr = (uintptr_t)scan_memory(angleZpattern, 0x6, false);

		const std::array<BYTE, 12> angleXpattern = { 0xC1, 0xE9, 0x05, 0x83, 0xE1, 0x07, 0x83, 0xE9, 0x03, 0x83, 0xF9, 0x02 };
		g_setAngleXAddr = (uintptr_t)scan_memory(angleXpattern, 0x6B, false);

		const std::array<BYTE, 9> refNamepattern = { 0x48, 0x8B, 0xF0, 0x45, 0x33, 0xE4, 0x48, 0x85, 0xC0 };
		g_refNameAddr = (uintptr_t)scan_memory(refNamepattern, 0x30, false);

		const std::array<BYTE, 8> lospattern = { 0xC6, 0x44, 0x24, 0x48, 0x00, 0x49, 0x8B, 0xD1 };
		g_hasLOSAddr = (uintptr_t)scan_memory_data(lospattern, 0xB, true, 0x1, 0x5);

		const std::array<BYTE, 11> hostpattern = { 0x41, 0x0F, 0x28, 0xD8, 0x41, 0x0F, 0x28, 0xD5, 0x49, 0x8B, 0xD6 };
		g_hostileAddr = (uintptr_t)scan_memory_data(hostpattern, 0x64, true, 0x1, 0x5);
	}

	UInt32 InvalidRefHandle()
	{
		return *(UInt32*)g_invalidRefHandleAddr;
	}

	bool LookupRefByHandle(UInt32 * refHandle, TESObjectREFR ** refrOut)
	{
		typedef bool(*LookupREFRByHandle_t)(UInt32 * refHandle, TESObjectREFR ** refrOut);
		LookupREFRByHandle_t LookupREFRByHandle = (LookupREFRByHandle_t)g_lookupAddr;

		return LookupREFRByHandle(refHandle, refrOut);
	}

	float TESObjectREFR::GetDistance(TESObjectREFR * target)
	{
		float x, y, z;

		x = pos.x - target->pos.x;
		y = pos.y - target->pos.y;
		z = pos.z - target->pos.z;

		return sqrt(x*x + y*y + z*z);
	}

	float TESObjectREFR::GetTargetHeight()
	{
		NiPoint3 p1, p2;
		return GetBoundRightBackTop(&p1)->z - GetBoundLeftFrontBottom(&p2)->z;
	}

	const char * TESObjectREFR::GetReferenceName()
	{
		typedef const char*(*GetReferenceName_t)(TESObjectREFR*);
		GetReferenceName_t GetReferenceName = (GetReferenceName_t)g_refNameAddr;

		return GetReferenceName(this);
	}

	bool Actor::IsOnMount()
	{
		typedef bool(*IsOnMount_t)(Actor*);
		IsOnMount_t IsOnMount = (IsOnMount_t)g_isonmountAddr;

		return IsOnMount(this);
	}

	bool Actor::IsTalking()
	{
		if ((flags1 & 0x180) != 0x180)
			return true;

		if (unk108 > 0)
			return true;

		return false;
	}

	void Actor::SetAngleX(float angle)
	{
		typedef void(*SetAngleX_t)(Actor * actor, float angle);
		SetAngleX_t SetAngleX = (SetAngleX_t)g_setAngleXAddr;

		SetAngleX(this, angle);
	}
	
	void Actor::SetAngleZ(float angle)
	{
		typedef void(*SetAngleZ_t)(Actor * actor, float angle);
		SetAngleZ_t SetAngleZ = (SetAngleZ_t)g_setAngleZAddr;

		SetAngleZ(this, angle);
	}

	bool Actor::IsFlyingActor()
	{
		return ((race->data.raceFlags & TESRace::kRace_Flies) == TESRace::kRace_Flies);
	}

	bool Actor::IsNotInFurniture()
	{
		return ((actorState.flags08 & 0x0003C000) == 0);
	}

	bool Actor::GetNodePosition(const char* nodeName, NiPoint3* point)
	{
		bool bResult = false;

		if (nodeName[0])
		{
			NiAVObject* object = (NiAVObject*)GetNiNode();
			if (object)
			{
				object = object->GetObjectByName(&nodeName);
				if (object)
				{
					point->x = object->m_worldTransform.pos.x;
					point->y = object->m_worldTransform.pos.y;
					point->z = object->m_worldTransform.pos.z;
					bResult = true;
				}
			}
		}

		return bResult;
	}


	bool Actor::GetTorsoPos(NiPoint3* point)
	{
		BGSBodyPartData* bodyPart = race->bodyPartData;
		BGSBodyPartData::Data* data;

		data = bodyPart->part[0];
		if (data)
		{
			return GetNodePosition(data->unk08.data, point);
		}

		return false;
	}

	bool Actor::GetTargetPos(NiPoint3* pos)
	{
		if (!GetNiNode())
			return false;

		if (formType != kFormType_Character)
			return false;

		if (!GetTorsoPos(pos))
				GetMarkerPosition(pos);

		return true;
	}

	bool Actor::HasLOS(TESObjectREFR* target, UInt8 * arg1)
	{
		typedef bool(*HasLOS_t)(Actor* source, TESObjectREFR* target, UInt8* arg1);
		HasLOS_t HasLOS = (HasLOS_t)g_hasLOSAddr;

		return HasLOS(this, target, arg1);
	}

	bool Actor::IsHostileToActor(Actor * target)
	{
		typedef bool(*IsHostileToActor_t)(Actor* source, Actor* target);
		IsHostileToActor_t IsHostileToActor = (IsHostileToActor_t)g_hostileAddr;

		return IsHostileToActor(this, target);
	}

	PlayerCharacter * PlayerCharacter::GetSingleton()
	{
		return *(PlayerCharacter**)g_playerCharacterAddr;
	}
}
