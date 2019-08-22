#include "ScaleformUtil.h"
#include "PatternScanner.h"

namespace Tralala
{
	uintptr_t g_scaleFormHeapAddr = 0;
	uintptr_t g_gfxLoaderAddr = 0;
	uintptr_t g_loadMovieAddr = 0;
	uintptr_t g_addManagedAddr = 0;
	uintptr_t g_relManagedAddr = 0;
	uintptr_t g_invokeAddr = 0;

	void ScaleformUtilGetAddresses()
	{
		const std::array<BYTE, 7> heappattern = { 0x89, 0x45, 0xEF, 0xC6, 0x45, 0x67, 0x00 };
		g_scaleFormHeapAddr = (uintptr_t)scan_memory_data(heappattern, 0x2C, false, 0x3, 0x7);

		const std::array<BYTE, 7> gfxpattern = { 0x80, 0xB8, 0x8C, 0x0B, 0x00, 0x00, 0x00 };
		g_gfxLoaderAddr = (uintptr_t)scan_memory_data(gfxpattern, 0x11, false, 0x3, 0x7);

		const std::array<BYTE, 7> movpattern = { 0x80, 0xB8, 0x8C, 0x0B, 0x00, 0x00, 0x00 };
		g_loadMovieAddr = (uintptr_t)scan_memory_data(movpattern, 0xA, false, 0x1, 0x5);

		const std::array<BYTE, 7> addpattern = { 0x41, 0xFF, 0x00, 0x48, 0x83, 0xC4, 0x28 };
		g_addManagedAddr = (uintptr_t)scan_memory(addpattern, 0x22, false);

		const std::array<BYTE, 6> relpattern = { 0x8B, 0x42, 0x08, 0x49, 0x8B, 0xD8 };
		g_relManagedAddr = (uintptr_t)scan_memory(relpattern, 0x6, false);

		const std::array<BYTE, 11> invokepattern = { 0x48, 0x89, 0x78, 0x08, 0xC7, 0x40, 0x10, 0x04, 0x00, 0x00, 0x00 };
		g_invokeAddr = (uintptr_t)scan_memory_data(invokepattern, 0xE4, true, 0x1, 0x5);
	}


	ScaleformHeap * ScaleformHeap::GetSingleton()
	{
		return *(ScaleformHeap**)g_scaleFormHeapAddr;
	}

	void * ScaleformHeap_Allocate(UInt32 size)
	{
		return ScaleformHeap::GetSingleton()->Allocate(size);
	}

	void ScaleformHeap_Free(void * ptr)
	{
		return ScaleformHeap::GetSingleton()->Free(ptr);
	}

	GFxLoader * GFxLoader::GetSingleton()
	{
		return *(GFxLoader**)g_gfxLoaderAddr;
	}

	bool GFxLoader::LoadMovie(void * menu, GFxMovieView ** viewOut, const char* name, int arg4, float arg5)
	{
		typedef bool(*LoadMovie_t)(GFxLoader *, void*, GFxMovieView**, const char*, int, float);
		LoadMovie_t LoadMovie = (LoadMovie_t)g_loadMovieAddr;

		return LoadMovie(this, menu, viewOut, name, arg4, arg5);
	}

	GFxValue::~GFxValue()
	{
		CleanManaged();
	}

	void GFxValue::AddManaged()
	{
		typedef void(*AddManaged_Internal_t)(ObjectInterface *, GFxValue *, void*);
		AddManaged_Internal_t AddManaged_Internal = (AddManaged_Internal_t)g_addManagedAddr;

		if (IsManaged())
			AddManaged_Internal(objectInterface, this, data.obj);
	}

	void GFxValue::AddManaged(const GFxValue& src)
	{
		if (IsManaged())
		{
			typedef void(*AddManaged_Internal_t)(ObjectInterface *, GFxValue *, void*);
			AddManaged_Internal_t AddManaged_Internal = (AddManaged_Internal_t)g_addManagedAddr;

			objectInterface = src.objectInterface;
			AddManaged_Internal(objectInterface, this, data.obj);
		}
	}

	void GFxValue::CleanManaged()
	{
		if (IsManaged())
		{
			typedef void(*ReleaseManaged_Internal_t)(ObjectInterface *, GFxValue *, void*);
			ReleaseManaged_Internal_t ReleaseManaged_Internal = (ReleaseManaged_Internal_t)g_relManagedAddr;

			ReleaseManaged_Internal(objectInterface, this, data.obj);

			objectInterface = nullptr;
			type = kType_Undefined;
		}
	}

	void GFxValue::SetUndefined()
	{
		CleanManaged();

		type = kType_Undefined;
	}

	void GFxValue::SetNull()
	{
		CleanManaged();

		type = kType_Null;
	}

	void GFxValue::SetBool(bool value)
	{
		CleanManaged();

		type = kType_Bool;
		data.boolean = value;
	}

	void GFxValue::SetNumber(double value)
	{
		CleanManaged();

		type = kType_Number;
		data.number = value;
	}

	void GFxValue::SetString(const char * value)
	{
		CleanManaged();

		type = kType_String;
		data.string = value;
	}

	void GFxValue::SetWideString(const wchar_t * value)
	{
		CleanManaged();

		type = kType_WideString;
		data.wideString = value;
	}

	bool GFxValue::Invoke(const char * name, GFxValue * result, GFxValue * args, UInt32 numArgs)
	{
		typedef bool(*Invoke_t)(ObjectInterface*, void * obj, GFxValue * result, const char * name, GFxValue * args, UInt32 numArgs, bool isDisplayObj);
		Invoke_t Invoke = (Invoke_t)g_invokeAddr;

		return Invoke(objectInterface, data.obj, result, name, args, numArgs, IsDisplayObject());
	}
}