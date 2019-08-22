#include "Utils.h"
#include "PatternScanner.h"

#define _USE_MATH_DEFINES
#include <math.h>

namespace Tralala
{
	uintptr_t g_TESCameraControllerAddr = 0;
	uintptr_t g_bsfixedstringCtorAddr = 0;
	uintptr_t g_bsfixedstringDtorAddr = 0;
	uintptr_t g_unkcellinfoAddr = 0;
	uintptr_t g_bInvertYValuesAddr = 0;
	uintptr_t g_iSizeWDisplayAddr = 0;
	uintptr_t g_iSizeHDisplayAddr = 0;
	uintptr_t g_MenuTopicManagerAddr = 0;

	void UtilsGetAddresses(void)
	{
		const std::array<BYTE, 10> Controllerpattern = { 0xF3, 0x0F, 0x5C, 0xF0, 0x48, 0x8B, 0xCF, 0x0F, 0x28, 0xCE };
		g_TESCameraControllerAddr = (uintptr_t)scan_memory_data(Controllerpattern, 0x99, false, 0x3, 0x7);

		const std::array<BYTE, 6> ctorpattern = { 0x45, 0x33, 0xDB, 0x48, 0x8B, 0xD9 };
		g_bsfixedstringCtorAddr = (uintptr_t)scan_memory(ctorpattern, 0x6, false);

		const std::array<BYTE, 7> dtorpattern = { 0x44, 0x8D, 0x42, 0x38, 0x48, 0x8B, 0xCB };
		g_bsfixedstringDtorAddr = (uintptr_t)scan_memory_data(dtorpattern, 0x18, false, 0x3, 0x7);

		const std::array<BYTE, 6> pattern = { 0x0F, 0x94, 0xC0, 0x88, 0x41, 0x0A };
		g_unkcellinfoAddr = (uintptr_t)scan_memory_data(pattern, 0x10, false, 0x3, 0x7);

		const std::array<BYTE, 12> invertpattern = { 0xF3, 0x0F, 0x10, 0x0B, 0xF3, 0x0F, 0x59, 0xC9, 0xF3, 0x0F, 0x59, 0xC0 };
		g_bInvertYValuesAddr = (uintptr_t)scan_memory_data(invertpattern, 0x67, false, 0x3, 0x7);

		const std::array<BYTE, 6> wdispattern = { 0x89, 0x44, 0x24, 0x5C, 0x33, 0xC0 };
		g_iSizeWDisplayAddr = (uintptr_t)scan_memory_data(wdispattern, 0x36, false, 0x2, 0x6);

		const std::array<BYTE, 6> hdispattern = { 0x89, 0x44, 0x24, 0x5C, 0x33, 0xC0 };
		g_iSizeHDisplayAddr = (uintptr_t)scan_memory_data(hdispattern, 0x2C, false, 0x2, 0x6);

		const std::array<BYTE, 13> Topicpattern = { 0x48, 0x85, 0xC0, 0x41, 0xB1, 0x01, 0x48, 0x8B, 0xD7, 0x4C, 0x0F, 0x45, 0xC0 };
		g_MenuTopicManagerAddr = (uintptr_t)scan_memory_data(Topicpattern, 0x55, true, 0x3, 0x7);
	}

	UnkCellInfo** UnkCellInfo::GetSingleton()
	{
		return (UnkCellInfo**)g_unkcellinfoAddr;
	}

	TESCameraController* TESCameraController::GetSingleton()
	{
		return (TESCameraController*)g_TESCameraControllerAddr;
	}

	MenuTopicManager* MenuTopicManager::GetSingleton()
	{
		return *(MenuTopicManager * *)g_MenuTopicManagerAddr;
	}

	StringCache::Ref::Ref()
	{
		typedef StringCache::Ref*(*ctor_t)(StringCache::Ref *, const char *);
		ctor_t ctor = (ctor_t)g_bsfixedstringCtorAddr;

		ctor(this, "");
	}

	StringCache::Ref::Ref(const char * buf)
	{
		typedef StringCache::Ref*(*ctor_t)(StringCache::Ref *, const char *);
		ctor_t ctor = (ctor_t)g_bsfixedstringCtorAddr;

		ctor(this, buf);
	}

	void StringCache::Ref::Release()
	{
		typedef void(*dtor_t)(StringCache::Ref*);
		dtor_t dtor = (dtor_t)g_bsfixedstringDtorAddr;

		dtor(this);
	}


	double NormalAbsoluteAngle(double angle)
	{
		while (angle < 0)
			angle += 2 * M_PI;
		while (angle > 2 * M_PI)
			angle -= 2 * M_PI;
		return angle;
	}

	double NormalRelativeAngle(double angle)
	{
		while (angle > M_PI)
			angle -= 2 * M_PI;
		while (angle < -M_PI)
			angle += 2 * M_PI;
		return angle;
	}

	void ComputeAnglesFromMatrix(NiMatrix33* mat, NiPoint3* angle)
	{
		const double threshold = 0.001;
		if (abs(mat->data[2][1] - 1.0) < threshold)
		{
			angle->x = M_PI / 2;
			angle->y = 0;
			angle->z = atan2(mat->data[1][0], mat->data[0][0]);
		}
		else if (abs(mat->data[2][1] + 1.0) < threshold)
		{
			angle->x = -M_PI / 2;
			angle->y = 0;
			angle->z = atan2(mat->data[1][0], mat->data[0][0]);
		}
		else
		{
			angle->x = asin(mat->data[2][1]);
			angle->y = atan2(-mat->data[2][0], mat->data[2][2]);
			angle->z = atan2(-mat->data[0][1], mat->data[1][1]);
		}
	}

	void GetAngle(const NiPoint3 &from, const NiPoint3 &to, AngleZX* angle)
	{
		float x = to.x - from.x;
		float y = to.y - from.y;
		float z = to.z - from.z;
		float xy = sqrt(x*x + y*y);

		angle->z = atan2(x, y);
		angle->x = atan2(-z, xy);
		angle->distance = sqrt(xy*xy + z*z);
	}
}