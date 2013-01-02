#include "options.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGED, MirEnumerateMirrorPaths)
#endif


NTSTATUS 
MirEnumerateMirrorPaths(
	_In_ PREGISTRY_KEYVALUE_CALLBACK Callback, 
	_In_ PVOID Data
)
{
	UNREFERENCED_PARAMETER(Callback);
	UNREFERENCED_PARAMETER(Data);

	return STATUS_SUCCESS;
}